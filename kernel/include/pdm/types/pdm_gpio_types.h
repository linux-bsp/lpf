// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_GPIO_TYPES_H
#define PDM_GPIO_TYPES_H

#include "osal.h"

typedef enum {
	PDM_GPIO_DIR_INPUT = 0,
	PDM_GPIO_DIR_OUTPUT,
} pdm_gpio_direction_t;

typedef enum {
	PDM_GPIO_LEVEL_LOW = 0,
	PDM_GPIO_LEVEL_HIGH,
} pdm_gpio_level_t;

typedef enum {
	PDM_GPIO_EDGE_NONE = 0,
	PDM_GPIO_EDGE_RISING,
	PDM_GPIO_EDGE_FALLING,
	PDM_GPIO_EDGE_BOTH,
} pdm_gpio_edge_t;

typedef void (*pdm_gpio_irq_callback_t)(uint32_t gpio_num,
					pdm_gpio_level_t level,
					void *user_data);

typedef struct {
	pdm_gpio_direction_t direction;
	pdm_gpio_level_t initial_level;
	pdm_gpio_edge_t edge;
	pdm_gpio_irq_callback_t callback;
	void *user_data;
} pdm_gpio_config_t;

#endif /* PDM_GPIO_TYPES_H */
