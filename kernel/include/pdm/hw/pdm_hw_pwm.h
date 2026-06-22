// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_HW_PWM_H
#define PDM_HW_PWM_H

#include "pdm/types/pdm_pwm_types.h"

int32_t pdm_hw_pwm_init(const pdm_pwm_config_t *config,
			pdm_hw_pwm_handle_t *handle);
int32_t pdm_hw_pwm_deinit(pdm_hw_pwm_handle_t handle);
int32_t pdm_hw_pwm_apply(pdm_hw_pwm_handle_t handle,
			 const pdm_pwm_state_t *state);
int32_t pdm_hw_pwm_get_state(pdm_hw_pwm_handle_t handle,
			     pdm_pwm_state_t *state);
int32_t pdm_hw_pwm_enable(pdm_hw_pwm_handle_t handle);
int32_t pdm_hw_pwm_disable(pdm_hw_pwm_handle_t handle);

#endif /* PDM_HW_PWM_H */
