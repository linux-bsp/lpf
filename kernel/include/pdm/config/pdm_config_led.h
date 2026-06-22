/************************************************************************
 * PDM_CONFIG LED configuration types
 ************************************************************************/

#ifndef PDM_CONFIG_LED_H
#define PDM_CONFIG_LED_H

#include "pdm/config/pdm_config_common.h"

typedef enum {
	PDM_CONFIG_LED_CONTROL_GPIO = 0x00,
	PDM_CONFIG_LED_CONTROL_PWM = 0x01,
} pdm_config_led_control_t;

typedef struct {
	const char *consumer;
	uint32_t period_ns;
	bool polarity_inversed;
} pdm_config_pwm_config_t;

typedef struct {
	char name[64];
	pdm_config_led_control_t control;
	uint32_t max_brightness;
	uint32_t default_brightness;
	union {
		pdm_config_gpio_config_t gpio;
		pdm_config_pwm_config_t pwm;
	} hw;
} pdm_config_led_config_t;

typedef struct {
	const char *description;
	bool enabled;
	pdm_config_led_config_t config;
} pdm_config_led_entry_t;

#endif /* PDM_CONFIG_LED_H */
