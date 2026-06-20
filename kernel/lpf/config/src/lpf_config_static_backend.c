// SPDX-License-Identifier: GPL-2.0

#include "osal.h"
#include "lpf_config_backend.h"

static bool lpf_config_static_available(void)
{
	return g_lpf_config_platform_table.configs &&
	       g_lpf_config_platform_table.count > 0 &&
	       g_lpf_config_platform_table.current_index <
		       g_lpf_config_platform_table.count;
}

static int32_t lpf_config_static_load(void)
{
	return lpf_config_static_available() ? OSAL_SUCCESS : OSAL_ERR_GENERIC;
}

static void lpf_config_static_unload(void)
{
}

static const lpf_config_platform_config_t *lpf_config_static_active(void)
{
	if (!lpf_config_static_available())
		return NULL;

	return g_lpf_config_platform_table
		.configs[g_lpf_config_platform_table.current_index];
}

static const lpf_config_platform_config_t *
lpf_config_static_find(const char *product, const char *project,
		    const char *version)
{
	uint32_t i;

	if (NULL == product || NULL == project ||
	    NULL == g_lpf_config_platform_table.configs)
		return NULL;

	for (i = 0; i < g_lpf_config_platform_table.count; i++) {
		const lpf_config_platform_config_t *config;

		config = g_lpf_config_platform_table.configs[i];
		if (NULL == config)
			continue;

		if (NULL == config->product_name || NULL == config->project_name)
			continue;

		if (0 != osal_strcmp(config->product_name, product))
			continue;

		if (0 != osal_strcmp(config->project_name, project))
			continue;

		if (version) {
			if (!config->version)
				continue;
			if (0 != osal_strcmp(config->version, version))
				continue;
		}

		return config;
	}

	return NULL;
}

static int32_t lpf_config_static_list(const lpf_config_platform_config_t **configs,
				   uint32_t *count)
{
	uint32_t actual_count;
	uint32_t max_count;
	uint32_t i;

	if (NULL == configs || NULL == count)
		return OSAL_ERR_GENERIC;

	if (NULL == g_lpf_config_platform_table.configs) {
		*count = 0;
		return OSAL_SUCCESS;
	}

	max_count = *count;
	actual_count = (g_lpf_config_platform_table.count < max_count) ?
			       g_lpf_config_platform_table.count :
			       max_count;

	for (i = 0; i < actual_count; i++)
		configs[i] = g_lpf_config_platform_table.configs[i];

	*count = actual_count;
	return OSAL_SUCCESS;
}

const lpf_config_backend_ops_t g_lpf_config_static_backend = {
	.name = "static",
	.available = lpf_config_static_available,
	.load = lpf_config_static_load,
	.unload = lpf_config_static_unload,
	.active = lpf_config_static_active,
	.find = lpf_config_static_find,
	.list = lpf_config_static_list,
};
