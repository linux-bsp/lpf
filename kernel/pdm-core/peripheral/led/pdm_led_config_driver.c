// SPDX-License-Identifier: GPL-2.0

#include "pdm_runtime_internal.h"

static int32_t pdm_led_make_device_config(
	const pdm_config_led_entry_t *entry, uint32_t index,
	pdm_device_config_t *config)
{
	pdm_capability_t capabilities;

	if (!entry || !config)
		return OSAL_ERR_INVALID_PARAM;

	capabilities = PDM_DEVICE_CAP_USER_IOCTL | PDM_DEVICE_CAP_DEBUGFS;
	switch (entry->config.control) {
	case PDM_CONFIG_LED_CONTROL_GPIO:
		capabilities |= PDM_DEVICE_CAP_CONTROL_GPIO;
		break;
	case PDM_CONFIG_LED_CONTROL_PWM:
		capabilities |= PDM_DEVICE_CAP_CONTROL_PWM;
		break;
	default:
		return OSAL_ERR_NOT_SUPPORTED;
	}

	config->type = PDM_DEVICE_TYPE_LED;
	config->index = index;
	config->entry = entry;
	config->name = entry->config.name;
	config->capabilities = capabilities;
	return OSAL_SUCCESS;
}

static int32_t pdm_led_probe_config(const pdm_config_device_node_t *node)
{
	const pdm_config_led_entry_t *entry;
	pdm_device_config_t config;
	int32_t ret;

	if (!node || node->device_type != PDM_CONFIG_DEVICE_TYPE_LED)
		return OSAL_ERR_INVALID_PARAM;

	entry = (const pdm_config_led_entry_t *)node->payload;
	if (!entry || !entry->enabled)
		return OSAL_SUCCESS;

	osal_memset(&config, 0, sizeof(config));
	ret = pdm_led_make_device_config(entry, node->index, &config);
	if (ret != OSAL_SUCCESS)
		return ret;

	return pdm_device_register(&config);
}

pdm_runtime_config_driver_register(led, PDM_CONFIG_DEVICE_TYPE_LED,
				   pdm_led_probe_config);
