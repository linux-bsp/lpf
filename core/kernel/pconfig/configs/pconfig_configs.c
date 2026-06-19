// SPDX-License-Identifier: GPL-2.0

#include "pconfig/pconfig.h"

extern const pconfig_platform_config_t
	g_pconfig_kernel_x86_modules_1_0_0;

static const pconfig_platform_config_t *const g_pconfig_configs[] = {
	&g_pconfig_kernel_x86_modules_1_0_0,
};

const pconfig_platform_table_t g_pconfig_platform_table = {
	.configs = g_pconfig_configs,
	.count = OSAL_ARRAY_SIZE(g_pconfig_configs),
	.current_index = 0,
};
