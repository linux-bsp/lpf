// SPDX-License-Identifier: GPL-2.0

#include "pdm_runtime_internal.h"

static int32_t pdm_mcu_make_device_config(
	const pdm_config_mcu_entry_t *entry, uint32_t index,
	pdm_device_config_t *config)
{
	pdm_capability_t capabilities;

	if (!entry || !config)
		return OSAL_ERR_INVALID_PARAM;

	capabilities = PDM_DEVICE_CAP_USER_IOCTL | PDM_DEVICE_CAP_DEBUGFS;
	switch (entry->config.interface) {
	case PDM_CONFIG_MCU_INTERFACE_CAN:
		capabilities |= PDM_DEVICE_CAP_TRANSPORT_CAN;
		break;
	case PDM_CONFIG_MCU_INTERFACE_SERIAL:
		capabilities |= PDM_DEVICE_CAP_TRANSPORT_UART;
		break;
	default:
		return OSAL_ERR_NOT_SUPPORTED;
	}

	config->type = PDM_DEVICE_TYPE_MCU;
	config->index = index;
	config->entry = entry;
	config->name = entry->config.name;
	config->capabilities = capabilities;
	return OSAL_SUCCESS;
}

static int32_t pdm_mcu_probe_config(const pdm_config_device_node_t *node)
{
	const pdm_config_mcu_entry_t *entry;
	pdm_device_config_t config;
	int32_t ret;

	if (!node || node->device_type != PDM_CONFIG_DEVICE_TYPE_MCU)
		return OSAL_ERR_INVALID_PARAM;

	entry = (const pdm_config_mcu_entry_t *)node->payload;
	if (!entry || !entry->enabled)
		return OSAL_SUCCESS;

	osal_memset(&config, 0, sizeof(config));
	ret = pdm_mcu_make_device_config(entry, node->index, &config);
	if (ret != OSAL_SUCCESS)
		return ret;

	return pdm_device_register(&config);
}

pdm_runtime_config_driver_register(mcu, PDM_CONFIG_DEVICE_TYPE_MCU,
				   pdm_mcu_probe_config);
