// SPDX-License-Identifier: GPL-2.0

#include "lpf/runtime/lpf_runtime.h"

#include "lpf/core/lpf_core.h"
#include "lpf_runtime_internal.h"

static const lpf_runtime_config_driver_t *
lpf_runtime_config_driver_first(void)
{
	return &lpf_runtime_config_driver_start + 1;
}

static const lpf_runtime_config_driver_t *
lpf_runtime_config_driver_last(void)
{
	return &lpf_runtime_config_driver_end;
}

static const lpf_runtime_config_driver_t *
lpf_runtime_config_find_driver(lpf_config_device_type_t type)
{
	const lpf_runtime_config_driver_t *driver;

	for (driver = lpf_runtime_config_driver_first();
	     driver < lpf_runtime_config_driver_last(); driver++) {
		if (driver->type == type && driver->probe)
			return driver;
	}

	return NULL;
}

int32_t lpf_runtime_probe_devices(void)
{
	const lpf_config_device_node_t *nodes;
	uint32_t node_count;
	const lpf_runtime_config_driver_t *driver;
	uint32_t i;
	int32_t ret;

	ret = lpf_config_load();
	if (ret != OSAL_SUCCESS)
		return ret;

	nodes = lpf_config_get_device_nodes(&node_count);
	if (!nodes && node_count > 0)
		return OSAL_ENODEV;

	for (i = 0; i < node_count; i++) {
		const lpf_config_device_node_t *node;

		node = &nodes[i];
		if (node->device_type == LPF_CONFIG_DEVICE_TYPE_INVALID)
			continue;
		if (node->status != LPF_CONFIG_NODE_STATUS_OKAY)
			continue;

		driver = lpf_runtime_config_find_driver(node->device_type);
		if (!driver) {
			LOG_WARN("LPF-RUNTIME",
				 "no config driver for node %s type=%u",
				 node->name ? node->name : "unknown",
				 node->device_type);
			continue;
		}

		ret = driver->probe(node);
		if (ret != OSAL_SUCCESS)
			goto out_error;
	}

	return OSAL_SUCCESS;

out_error:
	lpf_device_unregister_all();
	return ret;
}
