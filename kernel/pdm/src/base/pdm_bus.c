// SPDX-License-Identifier: GPL-2.0

#include <linux/list.h>

#include "pdm_bus.h"
#include "pdm_status.h"

typedef struct {
	struct list_head node;
	const pdm_driver_t *driver;
} pdm_bus_driver_node_t;

typedef struct {
	struct list_head node;
	const pconfig_device_config_t *device;
	const pdm_driver_t *driver;
} pdm_bus_device_node_t;

static LIST_HEAD(g_pdm_bus_drivers);
static LIST_HEAD(g_pdm_bus_devices);
static osal_mutex_t g_pdm_bus_lock;
static bool g_pdm_bus_ready;

static pdm_bus_driver_node_t *
pdm_bus_find_driver_node_locked(pconfig_device_type_t type)
{
	pdm_bus_driver_node_t *driver_node;

	list_for_each_entry(driver_node, &g_pdm_bus_drivers, node) {
		if (driver_node->driver->type == type)
			return driver_node;
	}

	return NULL;
}

static pdm_bus_device_node_t *
pdm_bus_find_device_node_locked(const pconfig_device_config_t *device)
{
	pdm_bus_device_node_t *device_node;

	list_for_each_entry(device_node, &g_pdm_bus_devices, node) {
		if (device_node->device->device_type == device->device_type &&
		    device_node->device->index == device->index)
			return device_node;
	}

	return NULL;
}

int32_t pdm_bus_init(void)
{
	if (g_pdm_bus_ready)
		return OSAL_SUCCESS;

	if (osal_mutex_init(&g_pdm_bus_lock, NULL) != OSAL_SUCCESS)
		return OSAL_ERR_GENERIC;

	INIT_LIST_HEAD(&g_pdm_bus_drivers);
	INIT_LIST_HEAD(&g_pdm_bus_devices);
	g_pdm_bus_ready = true;
	return OSAL_SUCCESS;
}

void pdm_bus_deinit(void)
{
	pdm_bus_driver_node_t *driver_node;
	pdm_bus_driver_node_t *driver_tmp;

	if (!g_pdm_bus_ready)
		return;

	pdm_bus_remove_devices();

	osal_mutex_lock(&g_pdm_bus_lock);
	list_for_each_entry_safe(driver_node, driver_tmp, &g_pdm_bus_drivers,
				 node) {
		list_del(&driver_node->node);
		osal_free(driver_node);
	}
	osal_mutex_unlock(&g_pdm_bus_lock);

	osal_mutex_destroy(&g_pdm_bus_lock);
	g_pdm_bus_ready = false;
}

int32_t pdm_bus_register_driver(const pdm_driver_t *driver)
{
	pdm_bus_driver_node_t *driver_node;

	if (!driver || driver->type == PCONFIG_DEVICE_TYPE_INVALID ||
	    !driver->probe)
		return OSAL_ERR_INVALID_PARAM;

	if (pdm_bus_init() != OSAL_SUCCESS)
		return OSAL_ERR_GENERIC;

	driver_node = osal_zalloc(sizeof(*driver_node));
	if (!driver_node)
		return OSAL_ERR_NO_MEMORY;

	driver_node->driver = driver;

	osal_mutex_lock(&g_pdm_bus_lock);
	if (pdm_bus_find_driver_node_locked(driver->type)) {
		osal_mutex_unlock(&g_pdm_bus_lock);
		osal_free(driver_node);
		return OSAL_ERR_ALREADY_EXISTS;
	}
	list_add_tail(&driver_node->node, &g_pdm_bus_drivers);
	osal_mutex_unlock(&g_pdm_bus_lock);

	LOG_INFO("PDM", "Registered driver %s type=%u",
		 driver->name ? driver->name : "unknown", driver->type);
	return OSAL_SUCCESS;
}

void pdm_bus_unregister_driver(const pdm_driver_t *driver)
{
	pdm_bus_driver_node_t *driver_node;

	if (!g_pdm_bus_ready || !driver)
		return;

	osal_mutex_lock(&g_pdm_bus_lock);
	list_for_each_entry(driver_node, &g_pdm_bus_drivers, node) {
		if (driver_node->driver != driver)
			continue;

		list_del(&driver_node->node);
		osal_mutex_unlock(&g_pdm_bus_lock);
		osal_free(driver_node);
		return;
	}
	osal_mutex_unlock(&g_pdm_bus_lock);
}

int32_t pdm_bus_register_device(const pconfig_device_config_t *device)
{
	pdm_bus_device_node_t *device_node;
	pdm_bus_driver_node_t *driver_node;
	const pdm_driver_t *driver;
	int32_t ret;

	if (!device || device->device_type == PCONFIG_DEVICE_TYPE_INVALID)
		return OSAL_ERR_INVALID_PARAM;

	if (pdm_bus_init() != OSAL_SUCCESS)
		return OSAL_ERR_GENERIC;

	osal_mutex_lock(&g_pdm_bus_lock);
	if (pdm_bus_find_device_node_locked(device)) {
		osal_mutex_unlock(&g_pdm_bus_lock);
		return OSAL_ERR_ALREADY_EXISTS;
	}
	driver_node = pdm_bus_find_driver_node_locked(device->device_type);
	driver = driver_node ? driver_node->driver : NULL;
	osal_mutex_unlock(&g_pdm_bus_lock);

	if (!driver) {
		LOG_ERROR("PDM", "No driver for device type=%u",
			  device->device_type);
		return OSAL_ERR_NOT_SUPPORTED;
	}

	device_node = osal_zalloc(sizeof(*device_node));
	if (!device_node)
		return OSAL_ERR_NO_MEMORY;

	device_node->device = device;
	device_node->driver = driver;

	ret = driver->probe(device);
	if (ret != OSAL_SUCCESS) {
		osal_free(device_node);
		return ret;
	}

	osal_mutex_lock(&g_pdm_bus_lock);
	if (pdm_bus_find_device_node_locked(device)) {
		osal_mutex_unlock(&g_pdm_bus_lock);
		if (driver->remove)
			driver->remove(device);
		osal_free(device_node);
		return OSAL_ERR_ALREADY_EXISTS;
	}
	list_add_tail(&device_node->node, &g_pdm_bus_devices);
	osal_mutex_unlock(&g_pdm_bus_lock);

	return OSAL_SUCCESS;
}

void pdm_bus_remove_devices(void)
{
	pdm_bus_device_node_t *device_node;

	if (!g_pdm_bus_ready)
		return;

	for (;;) {
		osal_mutex_lock(&g_pdm_bus_lock);
		if (list_empty(&g_pdm_bus_devices)) {
			osal_mutex_unlock(&g_pdm_bus_lock);
			break;
		}

		device_node = list_last_entry(&g_pdm_bus_devices,
					      pdm_bus_device_node_t, node);
		list_del_init(&device_node->node);
		osal_mutex_unlock(&g_pdm_bus_lock);

		if (device_node->driver && device_node->driver->remove)
			device_node->driver->remove(device_node->device);
		osal_free(device_node);
	}
}
