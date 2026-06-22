// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_COMPAT_GPIO_H
#define PDM_COMPAT_GPIO_H

#include "pdm/types/pdm_gpio_types.h"

int32_t pdm_compat_gpio_request(uint32_t gpio_num, const char *label);
void pdm_compat_gpio_free(uint32_t gpio_num);
int32_t pdm_compat_gpio_set_direction(uint32_t gpio_num,
				      pdm_gpio_direction_t direction,
				      pdm_gpio_level_t initial_level);
int32_t pdm_compat_gpio_set_level(uint32_t gpio_num,
				  pdm_gpio_level_t level);
int32_t pdm_compat_gpio_get_level(uint32_t gpio_num,
				  pdm_gpio_level_t *level);
int32_t pdm_compat_gpio_request_interrupt(
	uint32_t gpio_num, pdm_gpio_edge_t edge,
	pdm_gpio_irq_callback_t callback, void *user_data,
	void **irq_handle);
void pdm_compat_gpio_free_interrupt(void *irq_handle);
int32_t pdm_compat_gpio_enable_interrupt(void *irq_handle);
int32_t pdm_compat_gpio_disable_interrupt(void *irq_handle);

#endif /* PDM_COMPAT_GPIO_H */
