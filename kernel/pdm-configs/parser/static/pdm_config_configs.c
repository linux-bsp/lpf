// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>

#include "osal.h"
#include "pdm/config/pdm_config.h"
#include "pdm/config/pdm_config_static.h"

extern const pdm_config_platform_config_t *const pdm_config_static_start;
extern const pdm_config_platform_config_t *const pdm_config_static_end;

pdm_config_static_table_t g_lpf_config_platform_table = {
	.configs = &pdm_config_static_start + 1,
	.current_index = 0,
};
EXPORT_SYMBOL_GPL(g_lpf_config_platform_table);

static uint32_t pdm_configs_count_static_configs(void)
{
	return (uint32_t)(&pdm_config_static_end -
			  &pdm_config_static_start - 1);
}

static int __init pdm_configs_module_init(void)
{
	g_lpf_config_platform_table.count = pdm_configs_count_static_configs();
	pdm_config_print_version();
	LOG_INFO("PDM_CONFIGS", "loaded %u static config(s)",
		 g_lpf_config_platform_table.count);
	return 0;
}

static void __exit pdm_configs_module_exit(void)
{
	LOG_INFO("PDM_CONFIGS", "unloaded");
}

module_init(pdm_configs_module_init);
module_exit(pdm_configs_module_exit);

MODULE_AUTHOR("PDM");
MODULE_DESCRIPTION("PDM configuration engine and built-in data providers");
MODULE_LICENSE("GPL");
MODULE_SOFTDEP("pre: osal");
