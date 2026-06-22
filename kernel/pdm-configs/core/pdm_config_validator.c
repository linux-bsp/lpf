// SPDX-License-Identifier: GPL-2.0

#include "osal.h"
#include "pdm_config_validator.h"

static uint32_t pdm_config_node_count_by_type(
	const pdm_config_platform_config_t *platform,
	pdm_config_device_type_t type)
{
	uint32_t count = 0;
	uint32_t i;

	if (!platform || !platform->device_nodes)
		return 0;

	for (i = 0; i < platform->device_node_count; i++) {
		if (platform->device_nodes[i].device_type == type)
			count++;
	}

	return count;
}

static const pdm_config_device_node_t *pdm_config_node_by_type(
	const pdm_config_platform_config_t *platform,
	pdm_config_device_type_t type, uint32_t ordinal)
{
	uint32_t current_ordinal = 0;
	uint32_t i;

	if (!platform || !platform->device_nodes)
		return NULL;

	for (i = 0; i < platform->device_node_count; i++) {
		const pdm_config_device_node_t *node = &platform->device_nodes[i];

		if (node->device_type != type)
			continue;
		if (current_ordinal == ordinal)
			return node;
		current_ordinal++;
	}

	return NULL;
}

static uint32_t pdm_config_mcu_count(const pdm_config_platform_config_t *platform)
{
	uint32_t count;

	count = pdm_config_node_count_by_type(platform,
					      PDM_CONFIG_DEVICE_TYPE_MCU);
	if (count > 0)
		return count;

	return platform->mcu_count;
}

static const void *pdm_config_mcu_entry(const pdm_config_platform_config_t *platform,
				     uint32_t index)
{
	const pdm_config_device_node_t *node;

	node = pdm_config_node_by_type(platform, PDM_CONFIG_DEVICE_TYPE_MCU,
				       index);
	if (node)
		return node->payload;

	return pdm_config_hw_get_mcu(platform, index);
}

static bool pdm_config_mcu_enabled(const void *entry)
{
	const pdm_config_mcu_entry_t *mcu = entry;

	return mcu && mcu->enabled;
}

static const char *pdm_config_mcu_node_name(const void *entry)
{
	const pdm_config_mcu_entry_t *mcu = entry;

	return mcu ? mcu->config.name : NULL;
}

static bool pdm_config_mcu_parity_valid(pdm_config_mcu_parity_t parity)
{
	return parity == PDM_CONFIG_MCU_PARITY_NONE ||
	       parity == PDM_CONFIG_MCU_PARITY_ODD ||
	       parity == PDM_CONFIG_MCU_PARITY_EVEN;
}

static bool
pdm_config_mcu_flow_control_valid(pdm_config_mcu_flow_control_t flow_control)
{
	return flow_control == PDM_CONFIG_MCU_FLOW_NONE ||
	       flow_control == PDM_CONFIG_MCU_FLOW_HW ||
	       flow_control == PDM_CONFIG_MCU_FLOW_SW;
}

static int32_t pdm_config_validate_mcu_can(uint32_t index,
					const pdm_config_mcu_config_t *config)
{
	if (NULL == config->hw.can.device || 0 == config->hw.can.bitrate) {
		LOG_ERROR("PDM_CONFIG", "MCU[%u] invalid CAN config", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	if (0 == config->hw.can.rx_timeout || 0 == config->hw.can.tx_timeout) {
		LOG_ERROR("PDM_CONFIG", "MCU[%u] invalid CAN timeout", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	return OSAL_SUCCESS;
}

static int32_t pdm_config_validate_mcu_serial(uint32_t index,
					   const pdm_config_mcu_config_t *config)
{
	if (NULL == config->hw.serial.device ||
	    0 == config->hw.serial.baudrate) {
		LOG_ERROR("PDM_CONFIG", "MCU[%u] invalid serial config", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	if (config->hw.serial.data_bits < 5 ||
	    config->hw.serial.data_bits > 8 ||
	    config->hw.serial.stop_bits < 1 ||
	    config->hw.serial.stop_bits > 2) {
		LOG_ERROR("PDM_CONFIG", "MCU[%u] invalid serial frame config",
			  index);
		return OSAL_ERR_INVALID_PARAM;
	}

	if (!pdm_config_mcu_parity_valid(config->hw.serial.parity) ||
	    !pdm_config_mcu_flow_control_valid(config->hw.serial.flow_control)) {
		LOG_ERROR("PDM_CONFIG", "MCU[%u] invalid serial mode", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	return OSAL_SUCCESS;
}

static int32_t pdm_config_validate_mcu_entry(uint32_t index, const void *entry)
{
	const pdm_config_mcu_entry_t *mcu = entry;
	const pdm_config_mcu_config_t *config;

	if (NULL == mcu)
		return OSAL_ERR_INVALID_PARAM;

	if (!mcu->enabled)
		return OSAL_SUCCESS;

	config = &mcu->config;
	if ('\0' == config->name[0]) {
		LOG_ERROR("PDM_CONFIG", "MCU[%u] missing name", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	if (0 == config->cmd_timeout_ms) {
		LOG_ERROR("PDM_CONFIG", "MCU[%u] invalid command timeout", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	switch (config->interface) {
	case PDM_CONFIG_MCU_INTERFACE_CAN:
		return pdm_config_validate_mcu_can(index, config);
	case PDM_CONFIG_MCU_INTERFACE_SERIAL:
		return pdm_config_validate_mcu_serial(index, config);
	default:
		LOG_ERROR("PDM_CONFIG", "MCU[%u] unsupported interface", index);
		return OSAL_ERR_NOT_SUPPORTED;
	}
}

static void pdm_config_print_mcu_entry(uint32_t index, const void *entry)
{
	const pdm_config_mcu_entry_t *mcu = entry;

	LOG_INFO("PDM_CONFIG", "  MCU[%u]: %s", index,
		 mcu && mcu->description ? mcu->description : "N/A");
}

static uint32_t pdm_config_led_count(const pdm_config_platform_config_t *platform)
{
	uint32_t count;

	count = pdm_config_node_count_by_type(platform,
					      PDM_CONFIG_DEVICE_TYPE_LED);
	if (count > 0)
		return count;

	return platform->led_count;
}

static const void *pdm_config_led_entry(const pdm_config_platform_config_t *platform,
				     uint32_t index)
{
	const pdm_config_device_node_t *node;

	node = pdm_config_node_by_type(platform, PDM_CONFIG_DEVICE_TYPE_LED,
				       index);
	if (node)
		return node->payload;

	return pdm_config_hw_get_led(platform, index);
}

static bool pdm_config_led_enabled(const void *entry)
{
	const pdm_config_led_entry_t *led = entry;

	return led && led->enabled;
}

static const char *pdm_config_led_node_name(const void *entry)
{
	const pdm_config_led_entry_t *led = entry;

	return led ? led->config.name : NULL;
}

static int32_t pdm_config_validate_led_entry(uint32_t index,
					  const void *entry)
{
	const pdm_config_led_entry_t *led = entry;
	const pdm_config_led_config_t *config;

	if (NULL == led)
		return OSAL_ERR_INVALID_PARAM;

	if (!led->enabled)
		return OSAL_SUCCESS;

	config = &led->config;
	if ('\0' == config->name[0]) {
		LOG_ERROR("PDM_CONFIG", "LED[%u] missing name", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	if (0 == config->max_brightness) {
		LOG_ERROR("PDM_CONFIG", "LED[%u] max brightness is zero", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	if (config->default_brightness > config->max_brightness) {
		LOG_ERROR("PDM_CONFIG", "LED[%u] default brightness exceeds max",
			  index);
		return OSAL_ERR_INVALID_PARAM;
	}

	switch (config->control) {
	case PDM_CONFIG_LED_CONTROL_GPIO:
		break;
	case PDM_CONFIG_LED_CONTROL_PWM:
		if (NULL == config->hw.pwm.consumer ||
		    0 == config->hw.pwm.period_ns) {
			LOG_ERROR("PDM_CONFIG", "LED[%u] invalid PWM config",
				  index);
			return OSAL_ERR_INVALID_PARAM;
		}
		break;
	default:
		LOG_ERROR("PDM_CONFIG", "LED[%u] invalid control type", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	return OSAL_SUCCESS;
}

static void pdm_config_print_led_entry(uint32_t index, const void *entry)
{
	const pdm_config_led_entry_t *led = entry;

	LOG_INFO("PDM_CONFIG", "  LED[%u]: %s", index,
		 led && led->description ? led->description : "N/A");
}

static const pdm_config_device_descriptor_t g_lpf_config_device_descriptors[] = {
	{
		.name = "MCU",
		.compatible = "lpf,mcu",
		.type = PDM_CONFIG_DEVICE_TYPE_MCU,
		.payload_size = sizeof(pdm_config_mcu_entry_t),
		.count = pdm_config_mcu_count,
		.entry = pdm_config_mcu_entry,
		.enabled = pdm_config_mcu_enabled,
		.node_name = pdm_config_mcu_node_name,
		.validate = pdm_config_validate_mcu_entry,
		.print = pdm_config_print_mcu_entry,
	},
	{
		.name = "LED",
		.compatible = "lpf,led",
		.type = PDM_CONFIG_DEVICE_TYPE_LED,
		.payload_size = sizeof(pdm_config_led_entry_t),
		.count = pdm_config_led_count,
		.entry = pdm_config_led_entry,
		.enabled = pdm_config_led_enabled,
		.node_name = pdm_config_led_node_name,
		.validate = pdm_config_validate_led_entry,
		.print = pdm_config_print_led_entry,
	},
};

const pdm_config_device_descriptor_t *pdm_config_device_descriptors(uint32_t *count)
{
	if (count)
		*count = OSAL_ARRAY_SIZE(g_lpf_config_device_descriptors);

	return g_lpf_config_device_descriptors;
}
