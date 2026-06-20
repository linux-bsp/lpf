// SPDX-License-Identifier: GPL-2.0

#include "lpf/config/lpf_config.h"

extern const lpf_config_platform_config_t
	g_lpf_config_kernel_x86_modules_v1;
extern const lpf_config_platform_config_t
	g_lpf_config_kernel_x86_mock_modules_v1;

static const lpf_config_platform_config_t *const g_lpf_config_configs[] = {
	&g_lpf_config_kernel_x86_modules_v1,
	&g_lpf_config_kernel_x86_mock_modules_v1,
};

const lpf_config_platform_table_t g_lpf_config_platform_table = {
	.configs = g_lpf_config_configs,
	.count = OSAL_ARRAY_SIZE(g_lpf_config_configs),
	.current_index = 0,
};
