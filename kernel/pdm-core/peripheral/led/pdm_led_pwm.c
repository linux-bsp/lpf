// SPDX-License-Identifier: GPL-2.0

#include <linux/math64.h>

#include "pdm/hw/pdm_hw_pwm.h"
#include "pdm_led_internal.h"

static int32_t pdm_led_pwm_init(pdm_led_context_t *ctx)
{
	pdm_pwm_config_t pwm_config;
	pdm_hw_pwm_handle_t pwm;
	int32_t ret;

	if (!ctx->config->hw.pwm.consumer ||
	    ctx->config->hw.pwm.period_ns == 0)
		return OSAL_ERR_INVALID_PARAM;

	pwm_config.consumer = ctx->config->hw.pwm.consumer;
	pwm_config.period_ns = ctx->config->hw.pwm.period_ns;
	pwm_config.duty_ns = 0;
	pwm_config.enabled = false;
	pwm_config.polarity_inversed = ctx->config->hw.pwm.polarity_inversed;

	ret = pdm_hw_pwm_init(&pwm_config, &pwm);
	if (ret != OSAL_SUCCESS)
		return ret;

	ctx->hw_handle = pwm;
	return OSAL_SUCCESS;
}

static int32_t pdm_led_pwm_apply(pdm_led_context_t *ctx)
{
	pdm_pwm_state_t state;
	uint64_t duty;

	if (!ctx->hw_handle || ctx->config->max_brightness == 0)
		return OSAL_ERR_INVALID_PARAM;

	duty = (uint64_t)ctx->config->hw.pwm.period_ns * ctx->brightness;
	duty = div_u64(duty, ctx->config->max_brightness);

	state.period_ns = ctx->config->hw.pwm.period_ns;
	state.duty_ns = (uint32_t)duty;
	state.enabled = ctx->enabled && ctx->brightness > 0;
	state.polarity_inversed = ctx->config->hw.pwm.polarity_inversed;

	return pdm_hw_pwm_apply(ctx->hw_handle, &state);
}

static void pdm_led_pwm_deinit(pdm_led_context_t *ctx)
{
	if (ctx->hw_handle)
		pdm_hw_pwm_deinit(ctx->hw_handle);
	ctx->hw_handle = NULL;
}

const pdm_led_control_ops_t pdm_led_pwm_ops = {
	.control = PDM_CONFIG_LED_CONTROL_PWM,
	.init = pdm_led_pwm_init,
	.apply = pdm_led_pwm_apply,
	.deinit = pdm_led_pwm_deinit,
};
