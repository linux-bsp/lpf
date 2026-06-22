// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>

#include "osal.h"
#include "pdm/hw/pdm_hw_gpio.h"
#include "pdm_hw_internal.h"
#include "pdm/soc/pdm_soc_adapter.h"

typedef struct pdm_hw_gpio_context {
	uint32_t gpio_num;
	void *irq_handle;
	pdm_gpio_direction_t direction;
	pdm_gpio_irq_callback_t callback;
	void *user_data;
	bool requested;
	bool interrupt_configured;
	bool interrupt_enabled;
	struct pdm_hw_gpio_context *next;
} pdm_hw_gpio_context_t;

static osal_mutex_t g_lpf_hw_gpio_lock;
static pdm_hw_gpio_context_t *g_lpf_hw_gpio_list;

static bool pdm_hw_gpio_valid_direction(pdm_gpio_direction_t direction)
{
	return direction == PDM_GPIO_DIR_INPUT || direction == PDM_GPIO_DIR_OUTPUT;
}

static bool pdm_hw_gpio_valid_level(pdm_gpio_level_t level)
{
	return level == PDM_GPIO_LEVEL_LOW || level == PDM_GPIO_LEVEL_HIGH;
}

static bool pdm_hw_gpio_valid_edge(pdm_gpio_edge_t edge)
{
	switch (edge) {
	case PDM_GPIO_EDGE_NONE:
	case PDM_GPIO_EDGE_RISING:
	case PDM_GPIO_EDGE_FALLING:
	case PDM_GPIO_EDGE_BOTH:
		return true;
	default:
		return false;
	}
}

static pdm_gpio_direction_t
pdm_hw_gpio_to_lpf_direction(pdm_gpio_direction_t direction)
{
	return direction == PDM_GPIO_DIR_INPUT ? PDM_GPIO_DIR_INPUT :
						 PDM_GPIO_DIR_OUTPUT;
}

static pdm_gpio_level_t pdm_hw_gpio_to_lpf_level(pdm_gpio_level_t level)
{
	return level == PDM_GPIO_LEVEL_HIGH ? PDM_GPIO_LEVEL_HIGH :
					      PDM_GPIO_LEVEL_LOW;
}

static pdm_gpio_level_t pdm_hw_gpio_from_lpf_level(pdm_gpio_level_t level)
{
	return level == PDM_GPIO_LEVEL_HIGH ? PDM_GPIO_LEVEL_HIGH :
					      PDM_GPIO_LEVEL_LOW;
}

static pdm_gpio_edge_t pdm_hw_gpio_to_lpf_edge(pdm_gpio_edge_t edge)
{
	switch (edge) {
	case PDM_GPIO_EDGE_NONE:
		return PDM_GPIO_EDGE_NONE;
	case PDM_GPIO_EDGE_RISING:
		return PDM_GPIO_EDGE_RISING;
	case PDM_GPIO_EDGE_FALLING:
		return PDM_GPIO_EDGE_FALLING;
	case PDM_GPIO_EDGE_BOTH:
		return PDM_GPIO_EDGE_BOTH;
	default:
		return PDM_GPIO_EDGE_NONE;
	}
}

static void pdm_hw_gpio_irq_callback(uint32_t gpio_num,
				  pdm_gpio_level_t pdm_level,
				  void *data)
{
	pdm_hw_gpio_context_t *ctx = data;
	pdm_gpio_level_t level;

	if (!ctx || !ctx->callback)
		return;

	level = pdm_hw_gpio_from_lpf_level(pdm_level);
	ctx->callback(gpio_num, level, ctx->user_data);
}

static void pdm_hw_gpio_free_interrupt(pdm_hw_gpio_context_t *ctx)
{
	if (!ctx || !ctx->interrupt_configured)
		return;

	pdm_soc_gpio_free_interrupt(ctx->irq_handle);
	ctx->interrupt_configured = false;
	ctx->interrupt_enabled = false;
	ctx->irq_handle = NULL;
	ctx->callback = NULL;
	ctx->user_data = NULL;
}

static int32_t pdm_hw_gpio_set_interrupt_locked(pdm_hw_gpio_context_t *ctx,
					     pdm_gpio_edge_t edge,
					     pdm_gpio_irq_callback_t callback,
					     void *user_data)
{
	int32_t ret;

	if (!ctx)
		return OSAL_ERR_INVALID_PARAM;

	if (!pdm_hw_gpio_valid_edge(edge) ||
	    (edge != PDM_GPIO_EDGE_NONE && !callback))
		return OSAL_ERR_INVALID_PARAM;

	pdm_hw_gpio_free_interrupt(ctx);
	if (edge == PDM_GPIO_EDGE_NONE)
		return OSAL_SUCCESS;

	ctx->callback = callback;
	ctx->user_data = user_data;
	ret = pdm_soc_gpio_request_interrupt(ctx->gpio_num,
					     pdm_hw_gpio_to_lpf_edge(edge),
					     pdm_hw_gpio_irq_callback, ctx,
					     &ctx->irq_handle);
	if (ret != OSAL_SUCCESS) {
		ctx->callback = NULL;
		ctx->user_data = NULL;
		return ret;
	}

	ctx->interrupt_configured = true;
	ctx->interrupt_enabled = true;
	return OSAL_SUCCESS;
}

static pdm_hw_gpio_context_t *pdm_hw_gpio_get_context(uint32_t gpio_num)
{
	pdm_hw_gpio_context_t *ctx;

	for (ctx = g_lpf_hw_gpio_list; ctx; ctx = ctx->next) {
		if (ctx->gpio_num == gpio_num)
			return ctx;
	}

	return NULL;
}

static void pdm_hw_gpio_add_context(pdm_hw_gpio_context_t *ctx)
{
	ctx->next = g_lpf_hw_gpio_list;
	g_lpf_hw_gpio_list = ctx;
}

static void pdm_hw_gpio_remove_context(pdm_hw_gpio_context_t *target)
{
	pdm_hw_gpio_context_t **slot = &g_lpf_hw_gpio_list;

	while (*slot) {
		if (*slot == target) {
			*slot = target->next;
			target->next = NULL;
			return;
		}

		slot = &(*slot)->next;
	}
}

static void pdm_hw_gpio_free_context(pdm_hw_gpio_context_t *ctx)
{
	if (!ctx)
		return;

	pdm_hw_gpio_free_interrupt(ctx);
	if (ctx->requested)
		pdm_soc_gpio_free(ctx->gpio_num);
	osal_free(ctx);
}

int32_t pdm_hw_gpio_init(uint32_t gpio_num, const pdm_gpio_config_t *config)
{
	pdm_hw_gpio_context_t *ctx;
	bool new_context = false;
	int ret;

	if (!config || !pdm_hw_gpio_valid_direction(config->direction) ||
	    !pdm_hw_gpio_valid_level(config->initial_level) ||
	    !pdm_hw_gpio_valid_edge(config->edge) ||
	    (config->edge != PDM_GPIO_EDGE_NONE && !config->callback))
		return OSAL_ERR_INVALID_PARAM;

	osal_mutex_lock(&g_lpf_hw_gpio_lock);
	ctx = pdm_hw_gpio_get_context(gpio_num);
	if (!ctx) {
		ctx = osal_zalloc(sizeof(*ctx));
		if (!ctx) {
			osal_mutex_unlock(&g_lpf_hw_gpio_lock);
			return OSAL_ERR_NO_MEMORY;
		}

		ret = pdm_soc_gpio_request(gpio_num, "lpf-hw");
		if (ret != OSAL_SUCCESS) {
			osal_free(ctx);
			osal_mutex_unlock(&g_lpf_hw_gpio_lock);
			return ret;
		}

		ctx->gpio_num = gpio_num;
		ctx->requested = true;
		pdm_hw_gpio_add_context(ctx);
		new_context = true;
	}

	ret = pdm_soc_gpio_set_direction(
		gpio_num, pdm_hw_gpio_to_lpf_direction(config->direction),
		pdm_hw_gpio_to_lpf_level(config->initial_level));
	if (ret != OSAL_SUCCESS)
		goto err_existing;

	ctx->direction = config->direction;

	if (config->edge != PDM_GPIO_EDGE_NONE) {
		ret = pdm_hw_gpio_set_interrupt_locked(ctx, config->edge,
						    config->callback,
						    config->user_data);
		if (ret != OSAL_SUCCESS)
			goto err_existing;
	}

	osal_mutex_unlock(&g_lpf_hw_gpio_lock);
	return OSAL_SUCCESS;

err_existing:
	if (new_context) {
		pdm_hw_gpio_remove_context(ctx);
		pdm_hw_gpio_free_context(ctx);
	}
	osal_mutex_unlock(&g_lpf_hw_gpio_lock);
	return ret;
}
EXPORT_SYMBOL_GPL(pdm_hw_gpio_init);

int32_t pdm_hw_gpio_deinit(uint32_t gpio_num)
{
	pdm_hw_gpio_context_t *ctx;

	osal_mutex_lock(&g_lpf_hw_gpio_lock);
	ctx = pdm_hw_gpio_get_context(gpio_num);
	if (!ctx) {
		osal_mutex_unlock(&g_lpf_hw_gpio_lock);
		return OSAL_ERR_INVALID_ID;
	}

	pdm_hw_gpio_remove_context(ctx);
	pdm_hw_gpio_free_context(ctx);
	osal_mutex_unlock(&g_lpf_hw_gpio_lock);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(pdm_hw_gpio_deinit);

int32_t pdm_hw_gpio_set_direction(uint32_t gpio_num,
			       pdm_gpio_direction_t direction)
{
	pdm_hw_gpio_context_t *ctx;
	int ret;

	if (!pdm_hw_gpio_valid_direction(direction))
		return OSAL_ERR_INVALID_PARAM;

	osal_mutex_lock(&g_lpf_hw_gpio_lock);
	ctx = pdm_hw_gpio_get_context(gpio_num);
	if (!ctx) {
		osal_mutex_unlock(&g_lpf_hw_gpio_lock);
		return OSAL_ERR_INVALID_ID;
	}

	ret = pdm_soc_gpio_set_direction(
		gpio_num, pdm_hw_gpio_to_lpf_direction(direction),
		PDM_GPIO_LEVEL_LOW);

	if (ret == OSAL_SUCCESS)
		ctx->direction = direction;
	osal_mutex_unlock(&g_lpf_hw_gpio_lock);
	return ret;
}
EXPORT_SYMBOL_GPL(pdm_hw_gpio_set_direction);

int32_t pdm_hw_gpio_get_direction(uint32_t gpio_num,
			       pdm_gpio_direction_t *direction)
{
	pdm_hw_gpio_context_t *ctx;

	if (!direction)
		return OSAL_ERR_INVALID_PARAM;

	osal_mutex_lock(&g_lpf_hw_gpio_lock);
	ctx = pdm_hw_gpio_get_context(gpio_num);
	if (!ctx) {
		osal_mutex_unlock(&g_lpf_hw_gpio_lock);
		return OSAL_ERR_INVALID_ID;
	}

	*direction = ctx->direction;
	osal_mutex_unlock(&g_lpf_hw_gpio_lock);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(pdm_hw_gpio_get_direction);

int32_t pdm_hw_gpio_set_level(uint32_t gpio_num, pdm_gpio_level_t level)
{
	pdm_hw_gpio_context_t *ctx;
	int32_t ret;

	if (!pdm_hw_gpio_valid_level(level))
		return OSAL_ERR_INVALID_PARAM;

	osal_mutex_lock(&g_lpf_hw_gpio_lock);
	ctx = pdm_hw_gpio_get_context(gpio_num);
	if (!ctx) {
		osal_mutex_unlock(&g_lpf_hw_gpio_lock);
		return OSAL_ERR_INVALID_ID;
	}

	ret = pdm_soc_gpio_set_level(gpio_num, pdm_hw_gpio_to_lpf_level(level));
	osal_mutex_unlock(&g_lpf_hw_gpio_lock);
	return ret;
}
EXPORT_SYMBOL_GPL(pdm_hw_gpio_set_level);

int32_t pdm_hw_gpio_get_level(uint32_t gpio_num, pdm_gpio_level_t *level)
{
	pdm_hw_gpio_context_t *ctx;
	pdm_gpio_level_t pdm_level;
	int32_t ret;

	if (!level)
		return OSAL_ERR_INVALID_PARAM;

	osal_mutex_lock(&g_lpf_hw_gpio_lock);
	ctx = pdm_hw_gpio_get_context(gpio_num);
	if (!ctx) {
		osal_mutex_unlock(&g_lpf_hw_gpio_lock);
		return OSAL_ERR_INVALID_ID;
	}

	ret = pdm_soc_gpio_get_level(gpio_num, &pdm_level);
	if (ret == OSAL_SUCCESS)
		*level = pdm_hw_gpio_from_lpf_level(pdm_level);
	osal_mutex_unlock(&g_lpf_hw_gpio_lock);
	return ret;
}
EXPORT_SYMBOL_GPL(pdm_hw_gpio_get_level);

int32_t pdm_hw_gpio_set_interrupt(uint32_t gpio_num, pdm_gpio_edge_t edge,
			       pdm_gpio_irq_callback_t callback,
			       void *user_data)
{
	pdm_hw_gpio_context_t *ctx;
	int ret;

	if (!pdm_hw_gpio_valid_edge(edge) ||
	    (edge != PDM_GPIO_EDGE_NONE && !callback))
		return OSAL_ERR_INVALID_PARAM;

	osal_mutex_lock(&g_lpf_hw_gpio_lock);
	ctx = pdm_hw_gpio_get_context(gpio_num);
	if (!ctx) {
		osal_mutex_unlock(&g_lpf_hw_gpio_lock);
		return OSAL_ERR_INVALID_ID;
	}

	ret = pdm_hw_gpio_set_interrupt_locked(ctx, edge, callback, user_data);
	osal_mutex_unlock(&g_lpf_hw_gpio_lock);
	return ret;
}
EXPORT_SYMBOL_GPL(pdm_hw_gpio_set_interrupt);

int32_t pdm_hw_gpio_enable_interrupt(uint32_t gpio_num)
{
	pdm_hw_gpio_context_t *ctx;
	int32_t ret;

	osal_mutex_lock(&g_lpf_hw_gpio_lock);
	ctx = pdm_hw_gpio_get_context(gpio_num);
	if (!ctx || !ctx->interrupt_configured) {
		osal_mutex_unlock(&g_lpf_hw_gpio_lock);
		return OSAL_ERR_INVALID_ID;
	}

	if (!ctx->interrupt_enabled) {
		ret = pdm_soc_gpio_enable_interrupt(ctx->irq_handle);
		if (ret != OSAL_SUCCESS) {
			osal_mutex_unlock(&g_lpf_hw_gpio_lock);
			return ret;
		}
		ctx->interrupt_enabled = true;
	}
	osal_mutex_unlock(&g_lpf_hw_gpio_lock);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(pdm_hw_gpio_enable_interrupt);

int32_t pdm_hw_gpio_disable_interrupt(uint32_t gpio_num)
{
	pdm_hw_gpio_context_t *ctx;
	int32_t ret;

	osal_mutex_lock(&g_lpf_hw_gpio_lock);
	ctx = pdm_hw_gpio_get_context(gpio_num);
	if (!ctx || !ctx->interrupt_configured) {
		osal_mutex_unlock(&g_lpf_hw_gpio_lock);
		return OSAL_ERR_INVALID_ID;
	}

	if (ctx->interrupt_enabled) {
		ret = pdm_soc_gpio_disable_interrupt(ctx->irq_handle);
		if (ret != OSAL_SUCCESS) {
			osal_mutex_unlock(&g_lpf_hw_gpio_lock);
			return ret;
		}
		ctx->interrupt_enabled = false;
	}
	osal_mutex_unlock(&g_lpf_hw_gpio_lock);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(pdm_hw_gpio_disable_interrupt);

static int pdm_hw_gpio_module_init(void)
{
	return osal_mutex_init(&g_lpf_hw_gpio_lock, NULL);
}

static void pdm_hw_gpio_module_deinit(void)
{
	pdm_hw_gpio_context_t *ctx;

	osal_mutex_lock(&g_lpf_hw_gpio_lock);
	while (g_lpf_hw_gpio_list) {
		ctx = g_lpf_hw_gpio_list;
		pdm_hw_gpio_remove_context(ctx);
		pdm_hw_gpio_free_context(ctx);
	}
	osal_mutex_unlock(&g_lpf_hw_gpio_lock);
	osal_mutex_destroy(&g_lpf_hw_gpio_lock);
}

PDM_HW_BUILTIN_DRIVER(gpio, pdm_hw_gpio_module_init,
		      pdm_hw_gpio_module_deinit);
