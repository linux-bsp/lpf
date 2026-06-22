// SPDX-License-Identifier: GPL-2.0

#include "pdm/runtime/pdm_runtime.h"

#include "pdm/core/pdm_core.h"
#include "pdm_runtime_internal.h"

static const pdm_runtime_config_driver_t *
pdm_runtime_config_driver_first(void)
{
	return &pdm_runtime_config_driver_start + 1;
}

static const pdm_runtime_config_driver_t *
pdm_runtime_config_driver_last(void)
{
	return &pdm_runtime_config_driver_end;
}

static const pdm_runtime_config_driver_t *
pdm_runtime_config_find_driver(const pdm_config_device_node_t *node)
{
	const pdm_runtime_config_driver_t *driver;
	const pdm_runtime_config_driver_t *type_match = NULL;

	if (!node)
		return NULL;

	for (driver = pdm_runtime_config_driver_first();
	     driver < pdm_runtime_config_driver_last(); driver++) {
		if (!driver->probe || driver->type != node->device_type)
			continue;

		if (!driver->compatible) {
			if (!type_match)
				type_match = driver;
			continue;
		}

		if (node->compatible &&
		    0 == osal_strcmp(driver->compatible, node->compatible))
			return driver;
	}

	return type_match;
}

int32_t pdm_runtime_probe_devices(void)
{
	const pdm_config_device_node_t *nodes;
	uint32_t node_count;
	const pdm_runtime_config_driver_t *driver;
	uint32_t i;
	int32_t ret;

	ret = pdm_config_load();
	if (ret == OSAL_ERR_NOT_SUPPORTED)
		return OSAL_SUCCESS;
	if (ret != OSAL_SUCCESS)
		return ret;

	nodes = pdm_config_get_device_nodes(&node_count);
	if (!nodes && node_count > 0)
		return OSAL_ENODEV;

	for (i = 0; i < node_count; i++) {
		const pdm_config_device_node_t *node;

		node = &nodes[i];
		if (node->device_type == PDM_CONFIG_DEVICE_TYPE_INVALID)
			continue;
		if (node->status != PDM_CONFIG_NODE_STATUS_OKAY)
			continue;

		driver = pdm_runtime_config_find_driver(node);
		if (!driver) {
			LOG_WARN("PDM-RUNTIME",
				 "no config driver for node %s type=%u compatible=%s",
				 node->name ? node->name : "unknown",
				 node->device_type,
				 node->compatible ? node->compatible : "none");
			continue;
		}

		ret = driver->probe(node);
		if (ret != OSAL_SUCCESS)
			goto out_error;
	}

	return OSAL_SUCCESS;

out_error:
	/* 注意: 在新总线架构中，设备由 Device Tree 创建和管理 */
	/* pdm_device_unregister_all(); - 旧伪总线 API 已删除 */
	return ret;
}
