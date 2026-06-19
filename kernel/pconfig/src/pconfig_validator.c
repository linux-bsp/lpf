// SPDX-License-Identifier: GPL-2.0

#include "osal.h"
#include "pconfig_validator.h"

static uint32_t pconfig_mcu_count(const pconfig_platform_config_t *platform)
{
	return platform->mcu_count;
}

static const void *pconfig_mcu_entry(const pconfig_platform_config_t *platform,
				     uint32_t index)
{
	return pconfig_hw_get_mcu(platform, index);
}

static bool pconfig_mcu_enabled(const void *entry)
{
	const pconfig_mcu_entry_t *mcu = entry;

	return mcu && mcu->enabled;
}

static bool pconfig_mcu_parity_valid(pconfig_mcu_parity_t parity)
{
	return parity == PCONFIG_MCU_PARITY_NONE ||
	       parity == PCONFIG_MCU_PARITY_ODD ||
	       parity == PCONFIG_MCU_PARITY_EVEN;
}

static bool
pconfig_mcu_flow_control_valid(pconfig_mcu_flow_control_t flow_control)
{
	return flow_control == PCONFIG_MCU_FLOW_NONE ||
	       flow_control == PCONFIG_MCU_FLOW_HW ||
	       flow_control == PCONFIG_MCU_FLOW_SW;
}

static int32_t pconfig_validate_mcu_can(uint32_t index,
					const pconfig_mcu_config_t *config)
{
	if (NULL == config->hw.can.device || 0 == config->hw.can.bitrate) {
		LOG_ERROR("PCONFIG", "MCU[%u] invalid CAN config", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	if (0 == config->hw.can.rx_timeout || 0 == config->hw.can.tx_timeout) {
		LOG_ERROR("PCONFIG", "MCU[%u] invalid CAN timeout", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	return OSAL_SUCCESS;
}

static int32_t pconfig_validate_mcu_serial(uint32_t index,
					   const pconfig_mcu_config_t *config)
{
	if (NULL == config->hw.serial.device ||
	    0 == config->hw.serial.baudrate) {
		LOG_ERROR("PCONFIG", "MCU[%u] invalid serial config", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	if (config->hw.serial.data_bits < 5 ||
	    config->hw.serial.data_bits > 8 ||
	    config->hw.serial.stop_bits < 1 ||
	    config->hw.serial.stop_bits > 2) {
		LOG_ERROR("PCONFIG", "MCU[%u] invalid serial frame config",
			  index);
		return OSAL_ERR_INVALID_PARAM;
	}

	if (!pconfig_mcu_parity_valid(config->hw.serial.parity) ||
	    !pconfig_mcu_flow_control_valid(config->hw.serial.flow_control)) {
		LOG_ERROR("PCONFIG", "MCU[%u] invalid serial mode", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	return OSAL_SUCCESS;
}

static int32_t pconfig_validate_mcu_entry(uint32_t index, const void *entry)
{
	const pconfig_mcu_entry_t *mcu = entry;
	const pconfig_mcu_config_t *config;

	if (NULL == mcu)
		return OSAL_ERR_INVALID_PARAM;

	if (!mcu->enabled)
		return OSAL_SUCCESS;

	config = &mcu->config;
	if ('\0' == config->name[0]) {
		LOG_ERROR("PCONFIG", "MCU[%u] missing name", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	if (0 == config->cmd_timeout_ms) {
		LOG_ERROR("PCONFIG", "MCU[%u] invalid command timeout", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	switch (config->interface) {
	case PCONFIG_MCU_INTERFACE_CAN:
		return pconfig_validate_mcu_can(index, config);
	case PCONFIG_MCU_INTERFACE_SERIAL:
		return pconfig_validate_mcu_serial(index, config);
	default:
		LOG_ERROR("PCONFIG", "MCU[%u] unsupported interface", index);
		return OSAL_ERR_NOT_SUPPORTED;
	}
}

static void pconfig_print_mcu_entry(uint32_t index, const void *entry)
{
	const pconfig_mcu_entry_t *mcu = entry;

	LOG_INFO("PCONFIG", "  MCU[%u]: %s", index,
		 mcu && mcu->description ? mcu->description : "N/A");
}

static uint32_t pconfig_led_count(const pconfig_platform_config_t *platform)
{
	return platform->led_count;
}

static const void *pconfig_led_entry(const pconfig_platform_config_t *platform,
				     uint32_t index)
{
	return pconfig_hw_get_led(platform, index);
}

static bool pconfig_led_enabled(const void *entry)
{
	const pconfig_led_entry_t *led = entry;

	return led && led->enabled;
}

static int32_t pconfig_validate_led_entry(uint32_t index,
					  const void *entry)
{
	const pconfig_led_entry_t *led = entry;
	const pconfig_led_config_t *config;

	if (NULL == led)
		return OSAL_ERR_INVALID_PARAM;

	if (!led->enabled)
		return OSAL_SUCCESS;

	config = &led->config;
	if ('\0' == config->name[0]) {
		LOG_ERROR("PCONFIG", "LED[%u] missing name", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	if (0 == config->max_brightness) {
		LOG_ERROR("PCONFIG", "LED[%u] max brightness is zero", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	if (config->default_brightness > config->max_brightness) {
		LOG_ERROR("PCONFIG", "LED[%u] default brightness exceeds max",
			  index);
		return OSAL_ERR_INVALID_PARAM;
	}

	switch (config->control) {
	case PCONFIG_LED_CONTROL_GPIO:
		break;
	case PCONFIG_LED_CONTROL_PWM:
		if (NULL == config->hw.pwm.consumer ||
		    0 == config->hw.pwm.period_ns) {
			LOG_ERROR("PCONFIG", "LED[%u] invalid PWM config",
				  index);
			return OSAL_ERR_INVALID_PARAM;
		}
		break;
	default:
		LOG_ERROR("PCONFIG", "LED[%u] invalid control type", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	return OSAL_SUCCESS;
}

static void pconfig_print_led_entry(uint32_t index, const void *entry)
{
	const pconfig_led_entry_t *led = entry;

	LOG_INFO("PCONFIG", "  LED[%u]: %s", index,
		 led && led->description ? led->description : "N/A");
}

static const pconfig_device_descriptor_t g_pconfig_device_descriptors[] = {
	{
		.name = "MCU",
		.type = PCONFIG_DEVICE_TYPE_MCU,
		.count = pconfig_mcu_count,
		.entry = pconfig_mcu_entry,
		.enabled = pconfig_mcu_enabled,
		.validate = pconfig_validate_mcu_entry,
		.print = pconfig_print_mcu_entry,
	},
	{
		.name = "LED",
		.type = PCONFIG_DEVICE_TYPE_LED,
		.count = pconfig_led_count,
		.entry = pconfig_led_entry,
		.enabled = pconfig_led_enabled,
		.validate = pconfig_validate_led_entry,
		.print = pconfig_print_led_entry,
	},
};

const pconfig_device_descriptor_t *pconfig_device_descriptors(uint32_t *count)
{
	if (count)
		*count = OSAL_ARRAY_SIZE(g_pconfig_device_descriptors);

	return g_pconfig_device_descriptors;
}
