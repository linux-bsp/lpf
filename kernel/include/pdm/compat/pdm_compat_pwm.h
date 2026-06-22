// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_COMPAT_PWM_H
#define PDM_COMPAT_PWM_H

#include "pdm/types/pdm_pwm_types.h"

int32_t pdm_compat_pwm_get(const char *consumer, pdm_pwm_handle_t *handle);
void pdm_compat_pwm_put(pdm_pwm_handle_t handle);
int32_t pdm_compat_pwm_apply(pdm_pwm_handle_t handle,
			     const pdm_pwm_state_t *state);
int32_t pdm_compat_pwm_get_state(pdm_pwm_handle_t handle,
				 pdm_pwm_state_t *state);
int32_t pdm_compat_pwm_enable(pdm_pwm_handle_t handle);
int32_t pdm_compat_pwm_disable(pdm_pwm_handle_t handle);

#endif /* PDM_COMPAT_PWM_H */
