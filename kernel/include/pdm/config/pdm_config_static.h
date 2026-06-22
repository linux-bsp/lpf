// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_CONFIG_STATIC_H
#define PDM_CONFIG_STATIC_H

#include "pdm/config/pdm_config.h"

#define PDM_CONFIG_STATIC_SECTION ".pdm_config_static"

typedef struct {
	const pdm_config_platform_config_t *const *configs;
	uint32_t count;
	uint32_t current_index;
} pdm_config_static_table_t;

#define pdm_config_static_register(_id, _config)                    \
	static const pdm_config_platform_config_t *const             \
		__lpf_config_static_##_id __attribute__((used,       \
		aligned(sizeof(void *)),                             \
		section(PDM_CONFIG_STATIC_SECTION))) = _config

extern pdm_config_static_table_t g_lpf_config_platform_table;

#endif /* PDM_CONFIG_STATIC_H */
