// SPDX-License-Identifier: GPL-2.0

#include "pdm/hw/pdm_hw_gpio.h"
#include "pdm_led_internal.h"

static pdm_gpio_level_t pdm_led_gpio_level(const pdm_config_led_config_t *config,
					   bool enabled)
{
	bool active = enabled;

	if (config->hw.gpio.active_low)
		active = !active;

	return active ? PDM_GPIO_LEVEL_HIGH : PDM_GPIO_LEVEL_LOW;
}

static int32_t pdm_led_gpio_init(pdm_led_context_t *ctx)
{
	pdm_gpio_config_t gpio_config;

	gpio_config.direction = PDM_GPIO_DIR_OUTPUT;
	gpio_config.initial_level = pdm_led_gpio_level(ctx->config,
						       ctx->enabled);
	gpio_config.edge = PDM_GPIO_EDGE_NONE;
	gpio_config.callback = NULL;
	gpio_config.user_data = NULL;

	return pdm_hw_gpio_init(ctx->config->hw.gpio.gpio_num, &gpio_config);
}

static int32_t pdm_led_gpio_apply(pdm_led_context_t *ctx)
{
	pdm_gpio_level_t level;

	level = pdm_led_gpio_level(ctx->config, ctx->enabled);
	return pdm_hw_gpio_set_level(ctx->config->hw.gpio.gpio_num, level);
}

static void pdm_led_gpio_deinit(pdm_led_context_t *ctx)
{
	pdm_hw_gpio_deinit(ctx->config->hw.gpio.gpio_num);
}

const pdm_led_control_ops_t pdm_led_gpio_ops = {
	.control = PDM_CONFIG_LED_CONTROL_GPIO,
	.init = pdm_led_gpio_init,
	.apply = pdm_led_gpio_apply,
	.deinit = pdm_led_gpio_deinit,
};
