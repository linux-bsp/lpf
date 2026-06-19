/************************************************************************
 * PCONFIG LED configuration types
 ************************************************************************/

#ifndef PCONFIG_LED_H
#define PCONFIG_LED_H

#include "pconfig_common.h"

typedef enum {
	PCONFIG_LED_CONTROL_GPIO = 0x00,
	PCONFIG_LED_CONTROL_PWM = 0x01,
} pconfig_led_control_t;

typedef struct {
	const char *consumer;
	uint32_t period_ns;
	bool polarity_inversed;
} pconfig_pwm_config_t;

typedef struct {
	char name[64];
	pconfig_led_control_t control;
	uint32_t max_brightness;
	uint32_t default_brightness;
	union {
		pconfig_gpio_config_t gpio;
		pconfig_pwm_config_t pwm;
	} hw;
} pconfig_led_config_t;

typedef struct {
	const char *description;
	bool enabled;
	pconfig_led_config_t config;
} pconfig_led_entry_t;

#endif /* PCONFIG_LED_H */
