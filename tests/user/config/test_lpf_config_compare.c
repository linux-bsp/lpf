// SPDX-License-Identifier: MIT

#include "test_lpf_config_compare.h"
#include "lpf_config_static.h"
#include "lpf_config_normalizer.h"

#include <string.h>

extern const lpf_config_platform_config_t *const lpf_config_static_start;
extern const lpf_config_platform_config_t *const lpf_config_static_end;

const lpf_config_platform_config_t *test_lpf_config_mock_static_config(void)
{
	const lpf_config_platform_config_t *const *config;

	for (config = &lpf_config_static_start + 1;
	     config < &lpf_config_static_end; config++) {
		if (*config && (*config)->project_name &&
		    strcmp((*config)->project_name, "x86_mock_modules") == 0)
			return *config;
	}

	return NULL;
}

int test_lpf_config_string_equal(const char *left, const char *right)
{
	if (!left || !right)
		return left == right ? 0 : -1;

	return strcmp(left, right);
}

static int test_lpf_config_compare_mcu_entries(
	const lpf_config_mcu_entry_t *left,
	const lpf_config_mcu_entry_t *right)
{
	const lpf_config_mcu_config_t *a = &left->config;
	const lpf_config_mcu_config_t *b = &right->config;

	if (left->enabled != right->enabled)
		return -1;
	if (test_lpf_config_string_equal(a->name, b->name))
		return -1;
	if (a->interface != b->interface)
		return -1;
	if (a->cmd_timeout_ms != b->cmd_timeout_ms ||
	    a->retry_count != b->retry_count)
		return -1;

	switch (a->interface) {
	case LPF_CONFIG_MCU_INTERFACE_CAN:
		if (test_lpf_config_string_equal(a->hw.can.device,
						 b->hw.can.device))
			return -1;
		return a->hw.can.bitrate == b->hw.can.bitrate &&
			       a->hw.can.rx_timeout == b->hw.can.rx_timeout &&
			       a->hw.can.tx_timeout == b->hw.can.tx_timeout &&
			       a->hw.can.tx_id == b->hw.can.tx_id &&
			       a->hw.can.rx_id == b->hw.can.rx_id ?
			       0 :
			       -1;
	case LPF_CONFIG_MCU_INTERFACE_SERIAL:
		if (test_lpf_config_string_equal(a->hw.serial.device,
						 b->hw.serial.device))
			return -1;
		return a->hw.serial.baudrate == b->hw.serial.baudrate &&
			       a->hw.serial.data_bits == b->hw.serial.data_bits &&
			       a->hw.serial.stop_bits == b->hw.serial.stop_bits &&
			       a->hw.serial.parity == b->hw.serial.parity &&
			       a->hw.serial.flow_control ==
				       b->hw.serial.flow_control ?
			       0 :
			       -1;
	default:
		return -1;
	}
}

static int test_lpf_config_compare_led_entries(
	const lpf_config_led_entry_t *left,
	const lpf_config_led_entry_t *right)
{
	const lpf_config_led_config_t *a = &left->config;
	const lpf_config_led_config_t *b = &right->config;

	if (left->enabled != right->enabled)
		return -1;
	if (test_lpf_config_string_equal(a->name, b->name))
		return -1;
	if (a->control != b->control ||
	    a->max_brightness != b->max_brightness ||
	    a->default_brightness != b->default_brightness)
		return -1;

	switch (a->control) {
	case LPF_CONFIG_LED_CONTROL_GPIO:
		return a->hw.gpio.gpio_num == b->hw.gpio.gpio_num &&
			       a->hw.gpio.pin_mux == b->hw.gpio.pin_mux &&
			       a->hw.gpio.active_low == b->hw.gpio.active_low &&
			       a->hw.gpio.pull_up == b->hw.gpio.pull_up &&
			       a->hw.gpio.pull_down == b->hw.gpio.pull_down ?
			       0 :
			       -1;
	case LPF_CONFIG_LED_CONTROL_PWM:
		if (test_lpf_config_string_equal(a->hw.pwm.consumer,
						 b->hw.pwm.consumer))
			return -1;
		return a->hw.pwm.period_ns == b->hw.pwm.period_ns &&
			       a->hw.pwm.polarity_inversed ==
				       b->hw.pwm.polarity_inversed ?
			       0 :
			       -1;
	default:
		return -1;
	}
}

int test_lpf_config_compare_devices(
	const lpf_config_device_config_t *left,
	const lpf_config_device_config_t *right)
{
	if (left->device_type != right->device_type || left->index != right->index)
		return -1;

	switch (left->device_type) {
	case LPF_CONFIG_DEVICE_TYPE_MCU:
		return test_lpf_config_compare_mcu_entries(left->entry,
							   right->entry);
	case LPF_CONFIG_DEVICE_TYPE_LED:
		return test_lpf_config_compare_led_entries(left->entry,
							   right->entry);
	default:
		return -1;
	}
}

int test_lpf_config_normalize_platform(
	const lpf_config_platform_config_t *platform,
	lpf_config_device_config_t *devices, uint32_t *count)
{
	*count = TEST_LPF_CONFIG_DEVICE_CAPACITY;
	return lpf_config_normalize_devices(platform, devices, count);
}
