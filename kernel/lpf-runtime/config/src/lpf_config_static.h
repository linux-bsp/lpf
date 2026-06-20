// SPDX-License-Identifier: GPL-2.0

#ifndef LPF_CONFIG_STATIC_H
#define LPF_CONFIG_STATIC_H

#include "lpf/config/lpf_config.h"

typedef struct {
	const lpf_config_platform_config_t *const *configs;
	uint32_t count;
	uint32_t current_index;
} lpf_config_static_table_t;

extern const lpf_config_static_table_t g_lpf_config_platform_table;

#endif /* LPF_CONFIG_STATIC_H */
