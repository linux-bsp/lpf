// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_PWM_TYPES_H
#define PDM_PWM_TYPES_H

#include "osal.h"

typedef void *pdm_hw_pwm_handle_t;
typedef void *pdm_pwm_handle_t;

typedef struct {
	const char *consumer;
	uint32_t period_ns;
	uint32_t duty_ns;
	bool enabled;
	bool polarity_inversed;
} pdm_pwm_config_t;

typedef struct {
	uint32_t period_ns;
	uint32_t duty_ns;
	bool enabled;
	bool polarity_inversed;
} pdm_pwm_state_t;

#endif /* PDM_PWM_TYPES_H */
