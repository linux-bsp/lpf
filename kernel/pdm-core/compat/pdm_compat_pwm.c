// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>
#include <linux/pwm.h>

#include "pdm/compat/pdm_compat_errno.h"
#include "pdm/compat/pdm_compat_pwm.h"

static void pdm_compat_pwm_fill_state(const struct pwm_state *src,
				      pdm_pwm_state_t *dst)
{
	dst->period_ns = (uint32_t)src->period;
	dst->duty_ns = (uint32_t)src->duty_cycle;
	dst->enabled = src->enabled;
	dst->polarity_inversed = src->polarity == PWM_POLARITY_INVERSED;
}

static void pdm_compat_pwm_fill_kernel_state(const pdm_pwm_state_t *src,
					     struct pwm_state *dst)
{
	dst->period = src->period_ns;
	dst->duty_cycle = src->duty_ns;
	dst->enabled = src->enabled;
	dst->polarity = src->polarity_inversed ? PWM_POLARITY_INVERSED :
						 PWM_POLARITY_NORMAL;
}

int32_t pdm_compat_pwm_get(const char *consumer, pdm_pwm_handle_t *handle)
{
	struct pwm_device *pwm;
	int ret;

	if (!consumer || !handle)
		return OSAL_ERR_INVALID_PARAM;

	pwm = pwm_get(NULL, consumer);
	if (IS_ERR(pwm)) {
		ret = PTR_ERR(pwm);
		return pdm_compat_errno_to_status(ret);
	}

	*handle = pwm;
	return OSAL_SUCCESS;
}

void pdm_compat_pwm_put(pdm_pwm_handle_t handle)
{
	if (!handle)
		return;

	pwm_put((struct pwm_device *)handle);
}

int32_t pdm_compat_pwm_apply(pdm_pwm_handle_t handle,
			     const pdm_pwm_state_t *state)
{
	struct pwm_state pwm_state;
	struct pwm_device *pwm = handle;
	int ret;

	if (!pwm || !state || state->period_ns == 0 ||
	    state->duty_ns > state->period_ns)
		return OSAL_ERR_INVALID_PARAM;

	pwm_get_state(pwm, &pwm_state);
	pdm_compat_pwm_fill_kernel_state(state, &pwm_state);
	ret = pwm_apply_might_sleep(pwm, &pwm_state);
	return pdm_compat_errno_to_status(ret);
}

int32_t pdm_compat_pwm_get_state(pdm_pwm_handle_t handle,
				 pdm_pwm_state_t *state)
{
	struct pwm_state pwm_state;
	struct pwm_device *pwm = handle;

	if (!pwm || !state)
		return OSAL_ERR_INVALID_PARAM;

	pwm_get_state(pwm, &pwm_state);
	pdm_compat_pwm_fill_state(&pwm_state, state);
	return OSAL_SUCCESS;
}

int32_t pdm_compat_pwm_enable(pdm_pwm_handle_t handle)
{
	struct pwm_device *pwm = handle;
	int ret;

	if (!pwm)
		return OSAL_ERR_INVALID_PARAM;

	ret = pwm_enable(pwm);
	return pdm_compat_errno_to_status(ret);
}

int32_t pdm_compat_pwm_disable(pdm_pwm_handle_t handle)
{
	struct pwm_device *pwm = handle;

	if (!pwm)
		return OSAL_ERR_INVALID_PARAM;

	pwm_disable(pwm);
	return OSAL_SUCCESS;
}
