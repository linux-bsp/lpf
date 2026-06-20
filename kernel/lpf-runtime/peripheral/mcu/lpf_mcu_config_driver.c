// SPDX-License-Identifier: GPL-2.0

#include "lpf_runtime_internal.h"

static int32_t lpf_mcu_make_device_config(
	const lpf_config_mcu_entry_t *entry, uint32_t index,
	lpf_device_config_t *config)
{
	lpf_capability_t capabilities;

	if (!entry || !config)
		return OSAL_ERR_INVALID_PARAM;

	capabilities = LPF_DEVICE_CAP_USER_IOCTL | LPF_DEVICE_CAP_DEBUGFS;
	switch (entry->config.interface) {
	case LPF_CONFIG_MCU_INTERFACE_CAN:
		capabilities |= LPF_DEVICE_CAP_TRANSPORT_CAN;
		break;
	case LPF_CONFIG_MCU_INTERFACE_SERIAL:
		capabilities |= LPF_DEVICE_CAP_TRANSPORT_UART;
		break;
	default:
		return OSAL_ERR_NOT_SUPPORTED;
	}

	config->type = LPF_DEVICE_TYPE_MCU;
	config->index = index;
	config->entry = entry;
	config->name = entry->config.name;
	config->capabilities = capabilities;
	return OSAL_SUCCESS;
}

static int32_t lpf_mcu_probe_config(const lpf_config_platform_config_t *platform)
{
	uint32_t i;

	if (!platform)
		return OSAL_ERR_INVALID_PARAM;

	for (i = 0; i < platform->mcu_count; i++) {
		const lpf_config_mcu_entry_t *entry;
		lpf_device_config_t config;
		int32_t ret;

		entry = lpf_config_hw_get_mcu(platform, i);
		if (!entry || !entry->enabled)
			continue;

		osal_memset(&config, 0, sizeof(config));
		ret = lpf_mcu_make_device_config(entry, i, &config);
		if (ret != OSAL_SUCCESS)
			return ret;

		ret = lpf_device_register(&config);
		if (ret != OSAL_SUCCESS)
			return ret;
	}

	return OSAL_SUCCESS;
}

lpf_runtime_config_driver_register(mcu, LPF_CONFIG_DEVICE_TYPE_MCU,
				   lpf_mcu_probe_config);
