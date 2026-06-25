// SPDX-License-Identifier: GPL-2.0
/**
 * @file pdm_device.c
 * @brief PDM device management for bus integration
 */

#include <linux/idr.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/slab.h>

#include "pdm/bus/pdm_bus.h"
#include "pdm/bus/pdm_device.h"
#include "pdm/pdm_manager.h"
#include "osal.h"

/* ========================================================================
 * Unified ID Management
 * ======================================================================== */

struct pdm_id_allocator {
	struct ida ida;
	const char *name;
	atomic_t count;
};

/* ID allocators indexed by device type */
static struct pdm_id_allocator pdm_id_allocators[] = {
	[PDM_MANAGER_DEVICE_TYPE_MCU] = {
		.name = "mcu",
		.count = ATOMIC_INIT(0),
	},
	[PDM_MANAGER_DEVICE_TYPE_LED] = {
		.name = "led",
		.count = ATOMIC_INIT(0),
	},
};

#define PDM_ID_ALLOCATORS_SIZE ARRAY_SIZE(pdm_id_allocators)

static struct pdm_id_allocator *pdm_id_get_allocator(u32 device_type)
{
	if (device_type >= PDM_ID_ALLOCATORS_SIZE)
		return NULL;

	if (!pdm_id_allocators[device_type].name)
		return NULL;

	return &pdm_id_allocators[device_type];
}

static int pdm_id_alloc(u32 device_type, int requested_id)
{
	struct pdm_id_allocator *allocator;
	int id;

	allocator = pdm_id_get_allocator(device_type);
	if (!allocator) {
		LOG_ERROR("Invalid device type: 0x%x", device_type);
		return -EINVAL;
	}

	if (requested_id >= 0) {
		id = ida_alloc_range(&allocator->ida, requested_id,
				     requested_id, GFP_KERNEL);
	} else {
		id = ida_alloc(&allocator->ida, GFP_KERNEL);
	}

	if (id >= 0) {
		atomic_inc(&allocator->count);
		LOG_DEBUG("Allocated %s ID %d", allocator->name, id);
	} else {
		LOG_ERROR("Failed to allocate %s ID: %d",
			  allocator->name, id);
	}

	return id;
}

static void pdm_id_free(u32 device_type, int id)
{
	struct pdm_id_allocator *allocator;

	if (id < 0)
		return;

	allocator = pdm_id_get_allocator(device_type);
	if (!allocator)
		return;

	ida_free(&allocator->ida, id);
	atomic_dec_if_positive(&allocator->count);
	LOG_DEBUG("Freed %s ID %d", allocator->name, id);
}

static int pdm_id_init(void)
{
	size_t i;

	for (i = 0; i < PDM_ID_ALLOCATORS_SIZE; i++) {
		if (pdm_id_allocators[i].name)
			ida_init(&pdm_id_allocators[i].ida);
	}

	LOG_INFO("PDM ID management initialized");
	return 0;
}

static void pdm_id_destroy(void)
{
	size_t i;

	for (i = 0; i < PDM_ID_ALLOCATORS_SIZE; i++) {
		if (pdm_id_allocators[i].name)
			ida_destroy(&pdm_id_allocators[i].ida);
	}

	LOG_INFO("PDM ID management destroyed");
}

/* ========================================================================
 * PDM Device Management
 * ======================================================================== */

static void pdm_device_release(struct device *dev)
{
	struct pdm_device *pdm_dev = dev_to_pdm_device(dev);

	LOG_DEBUG("Releasing device [%s]", dev_name(dev));
	pdm_device_unbind(pdm_dev);
	of_node_put(dev->of_node);
	kfree(pdm_dev);
}

struct pdm_device *pdm_device_alloc(unsigned int size)
{
	struct pdm_device *pdm_dev;

	pdm_dev = kzalloc(sizeof(*pdm_dev) + size, GFP_KERNEL);
	if (!pdm_dev) {
		return NULL;
	}

	device_initialize(&pdm_dev->dev);
	pdm_dev->dev.bus = &pdm_bus_type;
	pdm_dev->dev.release = pdm_device_release;
	pdm_dev->type = PDM_MANAGER_DEVICE_TYPE_INVALID;
	pdm_dev->state = PDM_MANAGER_DEVICE_STATE_REGISTERED;
	pdm_dev->id = -1;
	pdm_dev->requested_id = -1;

	return pdm_dev;
}
EXPORT_SYMBOL_GPL(pdm_device_alloc);

int pdm_device_register(struct pdm_device *pdm_dev, const char *name)
{
	int ret;

	if (!pdm_dev || !name) {
		return -EINVAL;
	}

	ret = dev_set_name(&pdm_dev->dev, "%s", name);
	if (ret) {
		LOG_ERROR("Failed to name device [%s], error %d",
			  name, ret);
		put_device(&pdm_dev->dev);
		return ret;
	}

	ret = device_add(&pdm_dev->dev);
	if (ret) {
		LOG_ERROR("Failed to register device [%s], error %d",
			  name, ret);
		put_device(&pdm_dev->dev);
		return ret;
	}

	LOG_DEBUG("Device [%s] registered", name);
	return 0;
}
EXPORT_SYMBOL_GPL(pdm_device_register);

void pdm_device_unregister(struct pdm_device *pdm_dev)
{
	if (!pdm_dev) {
		return;
	}

	LOG_DEBUG("Unregistering device [%s]",
		  dev_name(&pdm_dev->dev));
	device_unregister(&pdm_dev->dev);
}
EXPORT_SYMBOL_GPL(pdm_device_unregister);

void pdm_device_set_requested_id(struct pdm_device *pdm_dev, int id)
{
	if (!pdm_dev) {
		return;
	}

	pdm_dev->requested_id = id;
	if (!pdm_dev->id_allocated) {
		pdm_dev->id = id;
	}
}
EXPORT_SYMBOL_GPL(pdm_device_set_requested_id);

int pdm_device_bind(struct pdm_device *pdm_dev, u32 type, u64 capabilities)
{
	int id;

	if (!pdm_dev)
		return -EINVAL;

	if (pdm_dev->id_allocated) {
		if (pdm_dev->type != type)
			return -EBUSY;
		pdm_dev->capabilities |= capabilities;
		return 0;
	}

	id = pdm_id_alloc(type, pdm_dev->requested_id);
	if (id < 0)
		return id;

	pdm_dev->id = id;
	pdm_dev->id_allocated = true;
	pdm_dev->type = type;
	pdm_dev->capabilities |= capabilities;
	pdm_device_set_state(pdm_dev, PDM_MANAGER_DEVICE_STATE_REGISTERED);
	return 0;
}
EXPORT_SYMBOL_GPL(pdm_device_bind);

void pdm_device_unbind(struct pdm_device *pdm_dev)
{
	if (!pdm_dev || !pdm_dev->id_allocated)
		return;

	pdm_id_free(pdm_dev->type, pdm_dev->id);

	pdm_dev->id_allocated = false;
	pdm_dev->id = pdm_dev->requested_id;
	pdm_dev->type = PDM_MANAGER_DEVICE_TYPE_INVALID;
	pdm_dev->capabilities = PDM_MANAGER_DEVICE_CAP_NONE;
	pdm_device_set_state(pdm_dev, PDM_MANAGER_DEVICE_STATE_REGISTERED);
}
EXPORT_SYMBOL_GPL(pdm_device_unbind);

void pdm_device_ids_destroy(void)
{
	pdm_id_destroy();
}

int pdm_device_ids_init(void)
{
	return pdm_id_init();
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("PDM Device Management");

