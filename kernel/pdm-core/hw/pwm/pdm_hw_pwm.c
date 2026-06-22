// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>

#include "osal.h"
#include "pdm/hw/pdm_hw_pwm.h"
#include "pdm/soc/pdm_soc_adapter.h"

typedef struct {
	pdm_pwm_handle_t pwm;
	osal_mutex_t lock;
} pdm_hw_pwm_context_t;

static void pdm_hw_pwm_fill_state(const pdm_pwm_state_t *src, pdm_pwm_state_t *dst)
{
	dst->period_ns = src->period_ns;
	dst->duty_ns = src->duty_ns;
	dst->enabled = src->enabled;
	dst->polarity_inversed = src->polarity_inversed;
}

static void pdm_hw_pwm_fill_lpf_state(const pdm_pwm_state_t *src,
				   pdm_pwm_state_t *dst)
{
	dst->period_ns = src->period_ns;
	dst->duty_ns = src->duty_ns;
	dst->enabled = src->enabled;
	dst->polarity_inversed = src->polarity_inversed;
}

int32_t pdm_hw_pwm_init(const pdm_pwm_config_t *config, pdm_hw_pwm_handle_t *handle)
{
	pdm_hw_pwm_context_t *ctx;
	pdm_pwm_state_t state;
	int32_t ret;

	if (!config || !handle || config->period_ns == 0 ||
	    config->duty_ns > config->period_ns)
		return OSAL_ERR_INVALID_PARAM;

	ctx = osal_zalloc(sizeof(*ctx));
	if (!ctx)
		return OSAL_ERR_NO_MEMORY;

	if (osal_mutex_init(&ctx->lock, NULL) != OSAL_SUCCESS) {
		osal_free(ctx);
		return OSAL_ERR_GENERIC;
	}

	ret = pdm_soc_pwm_get(config->consumer, &ctx->pwm);
	if (ret != OSAL_SUCCESS) {
		osal_mutex_destroy(&ctx->lock);
		osal_free(ctx);
		return ret;
	}

	state.period_ns = config->period_ns;
	state.duty_ns = config->duty_ns;
	state.enabled = config->enabled;
	state.polarity_inversed = config->polarity_inversed;

	ret = pdm_soc_pwm_apply(ctx->pwm, &state);
	if (ret != OSAL_SUCCESS) {
		pdm_soc_pwm_put(ctx->pwm);
		osal_mutex_destroy(&ctx->lock);
		osal_free(ctx);
		return ret;
	}

	*handle = ctx;
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(pdm_hw_pwm_init);

int32_t pdm_hw_pwm_deinit(pdm_hw_pwm_handle_t handle)
{
	pdm_hw_pwm_context_t *ctx = handle;

	if (!ctx)
		return OSAL_ERR_INVALID_PARAM;

	osal_mutex_lock(&ctx->lock);
	pdm_soc_pwm_disable(ctx->pwm);
	pdm_soc_pwm_put(ctx->pwm);
	osal_mutex_unlock(&ctx->lock);
	osal_mutex_destroy(&ctx->lock);
	osal_free(ctx);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(pdm_hw_pwm_deinit);

int32_t pdm_hw_pwm_apply(pdm_hw_pwm_handle_t handle, const pdm_pwm_state_t *state)
{
	pdm_hw_pwm_context_t *ctx = handle;
	pdm_pwm_state_t pwm_state;
	int32_t ret;

	if (!ctx || !state || state->period_ns == 0 ||
	    state->duty_ns > state->period_ns)
		return OSAL_ERR_INVALID_PARAM;

	osal_mutex_lock(&ctx->lock);
	pdm_hw_pwm_fill_lpf_state(state, &pwm_state);
	ret = pdm_soc_pwm_apply(ctx->pwm, &pwm_state);
	osal_mutex_unlock(&ctx->lock);

	return ret;
}
EXPORT_SYMBOL_GPL(pdm_hw_pwm_apply);

int32_t pdm_hw_pwm_get_state(pdm_hw_pwm_handle_t handle, pdm_pwm_state_t *state)
{
	pdm_hw_pwm_context_t *ctx = handle;
	pdm_pwm_state_t pwm_state;
	int32_t ret;

	if (!ctx || !state)
		return OSAL_ERR_INVALID_PARAM;

	osal_mutex_lock(&ctx->lock);
	ret = pdm_soc_pwm_get_state(ctx->pwm, &pwm_state);
	osal_mutex_unlock(&ctx->lock);
	if (ret != OSAL_SUCCESS)
		return ret;

	pdm_hw_pwm_fill_state(&pwm_state, state);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(pdm_hw_pwm_get_state);

int32_t pdm_hw_pwm_enable(pdm_hw_pwm_handle_t handle)
{
	pdm_hw_pwm_context_t *ctx = handle;
	int32_t ret;

	if (!ctx)
		return OSAL_ERR_INVALID_PARAM;

	osal_mutex_lock(&ctx->lock);
	ret = pdm_soc_pwm_enable(ctx->pwm);
	osal_mutex_unlock(&ctx->lock);

	return ret;
}
EXPORT_SYMBOL_GPL(pdm_hw_pwm_enable);

int32_t pdm_hw_pwm_disable(pdm_hw_pwm_handle_t handle)
{
	pdm_hw_pwm_context_t *ctx = handle;

	if (!ctx)
		return OSAL_ERR_INVALID_PARAM;

	osal_mutex_lock(&ctx->lock);
	pdm_soc_pwm_disable(ctx->pwm);
	osal_mutex_unlock(&ctx->lock);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(pdm_hw_pwm_disable);
