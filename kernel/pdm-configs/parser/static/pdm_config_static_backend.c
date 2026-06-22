// SPDX-License-Identifier: GPL-2.0

#include "osal.h"
#include "pdm_config_backend.h"
#include "pdm/config/pdm_config_static.h"

#include <linux/module.h>
#include <linux/moduleparam.h>

#ifndef CONFIG_PROJECT_NAME
#define CONFIG_PROJECT_NAME ""
#endif

#ifndef CONFIG_PROJECT_VERSION
#define CONFIG_PROJECT_VERSION ""
#endif

static int pdm_config_static_index = -1;
static char *pdm_config_static_product;
static char *pdm_config_static_project;
static char *pdm_config_static_version;
static const pdm_config_static_table_t *g_lpf_config_static_table;
static bool g_lpf_config_static_loaded;

module_param_named(config_index, pdm_config_static_index, int, 0444);
MODULE_PARM_DESC(config_index, "PDM static config index override");
module_param_named(config_product, pdm_config_static_product, charp, 0444);
MODULE_PARM_DESC(config_product, "PDM static config product selector");
module_param_named(config_project, pdm_config_static_project, charp, 0444);
MODULE_PARM_DESC(config_project, "PDM static config project selector");
module_param_named(config_version, pdm_config_static_version, charp, 0444);
MODULE_PARM_DESC(config_version, "PDM static config version selector");

static const char *pdm_config_static_selector(const char *value)
{
	return value && value[0] ? value : NULL;
}

static bool pdm_config_static_has_param_identity_selector(void)
{
	return pdm_config_static_selector(pdm_config_static_product) ||
	       pdm_config_static_selector(pdm_config_static_project) ||
	       pdm_config_static_selector(pdm_config_static_version);
}

static const char *pdm_config_static_effective_project(void)
{
	const char *project;

	project = pdm_config_static_selector(pdm_config_static_project);
	return project ? project : pdm_config_static_selector(CONFIG_PROJECT_NAME);
}

static const char *pdm_config_static_effective_version(void)
{
	const char *version;

	version = pdm_config_static_selector(pdm_config_static_version);
	return version ? version : pdm_config_static_selector(CONFIG_PROJECT_VERSION);
}

static bool pdm_config_static_has_effective_identity_selector(
	const char *product, const char *project, const char *version)
{
	return product || project || version;
}

static const pdm_config_static_table_t *pdm_config_static_get_table(void)
{
	if (!g_lpf_config_static_table)
		g_lpf_config_static_table = &g_lpf_config_platform_table;

	return g_lpf_config_static_table;
}

static void pdm_config_static_put_table(void)
{
	g_lpf_config_static_table = NULL;
}

static void pdm_config_static_put_table_if_unloaded(void)
{
	if (!g_lpf_config_static_loaded)
		pdm_config_static_put_table();
}

static bool
pdm_config_static_identity_matches(const pdm_config_platform_config_t *config,
				   const char *product, const char *project,
				   const char *version)
{
	if (!config)
		return false;

	if (product) {
		if (!config->product_name ||
		    0 != osal_strcmp(config->product_name, product))
			return false;
	}

	if (project) {
		if (!config->project_name ||
		    0 != osal_strcmp(config->project_name, project))
			return false;
	}

	if (version) {
		if (!config->version ||
		    0 != osal_strcmp(config->version, version))
			return false;
	}

	return true;
}

static const pdm_config_platform_config_t *
pdm_config_static_config_at(const pdm_config_static_table_t *table,
			    uint32_t index)
{
	if (!table || !table->configs || index >= table->count)
		return NULL;

	return table->configs[index];
}

static const pdm_config_platform_config_t *
pdm_config_static_find_by_identity(const pdm_config_static_table_t *table,
				   const char *product, const char *project,
				   const char *version)
{
	uint32_t i;

	if (!table || !table->configs)
		return NULL;

	for (i = 0; i < table->count; i++) {
		const pdm_config_platform_config_t *config;

		config = pdm_config_static_config_at(table, i);
		if (pdm_config_static_identity_matches(config, product, project,
						       version))
			return config;
	}

	return NULL;
}

static const pdm_config_platform_config_t *
pdm_config_static_selected(const pdm_config_static_table_t *table)
{
	const pdm_config_platform_config_t *config;
	const char *product = pdm_config_static_selector(pdm_config_static_product);
	const char *project = pdm_config_static_effective_project();
	const char *version = pdm_config_static_effective_version();

	if (pdm_config_static_index < -1)
		return NULL;

	if (pdm_config_static_index >= 0) {
		config = pdm_config_static_config_at(
			table, (uint32_t)pdm_config_static_index);
		if (!pdm_config_static_has_param_identity_selector())
			return config;

		return pdm_config_static_identity_matches(config, product,
							  project, version) ?
			       config :
			       NULL;
	}

	if (pdm_config_static_has_effective_identity_selector(product, project,
							     version))
		return pdm_config_static_find_by_identity(table, product, project,
							  version);

	return pdm_config_static_config_at(table, table->current_index);
}

static bool pdm_config_static_available(void)
{
	const pdm_config_static_table_t *table;
	bool available;

	table = pdm_config_static_get_table();
	available = table && table->configs && table->count > 0 &&
		    pdm_config_static_selected(table);
	pdm_config_static_put_table_if_unloaded();

	return available;
}

static int32_t pdm_config_static_load(void)
{
	const pdm_config_static_table_t *table;
	const pdm_config_platform_config_t *config;

	table = pdm_config_static_get_table();
	if (!table)
		return OSAL_ERR_NOT_SUPPORTED;

	config = pdm_config_static_selected(table);
	if (!config) {
		LOG_ERROR("PDM_CONFIG", "No matching static config");
		pdm_config_static_put_table();
		return OSAL_ERR_GENERIC;
	}

	g_lpf_config_static_loaded = true;
	LOG_INFO("PDM_CONFIG", "Static config selected: %s/%s/%s",
		 config->product_name ? config->product_name : "unknown",
		 config->project_name ? config->project_name : "unknown",
		 config->version ? config->version : "unknown");
	return OSAL_SUCCESS;
}

static void pdm_config_static_unload(void)
{
	g_lpf_config_static_loaded = false;
	pdm_config_static_put_table();
}

static const pdm_config_platform_config_t *pdm_config_static_active(void)
{
	if (!g_lpf_config_static_loaded)
		return NULL;

	return pdm_config_static_selected(g_lpf_config_static_table);
}

static const pdm_config_platform_config_t *
pdm_config_static_find(const char *product, const char *project,
		    const char *version)
{
	const pdm_config_static_table_t *table;
	const pdm_config_platform_config_t *config;

	if (NULL == product || NULL == project)
		return NULL;
	if (!g_lpf_config_static_loaded)
		return NULL;

	table = g_lpf_config_static_table;
	config = pdm_config_static_find_by_identity(table, product, project,
						   version);

	return config;
}

static int32_t pdm_config_static_list(const pdm_config_platform_config_t **configs,
				   uint32_t *count)
{
	const pdm_config_static_table_t *table;
	uint32_t actual_count;
	uint32_t max_count;
	uint32_t i;

	if (NULL == configs || NULL == count)
		return OSAL_ERR_GENERIC;
	if (!g_lpf_config_static_loaded) {
		*count = 0;
		return OSAL_ERR_INVALID_STATE;
	}

	table = g_lpf_config_static_table;
	if (!table || NULL == table->configs) {
		*count = 0;
		return OSAL_SUCCESS;
	}

	max_count = *count;
	actual_count = (table->count < max_count) ? table->count : max_count;

	for (i = 0; i < actual_count; i++)
		configs[i] = table->configs[i];

	*count = actual_count;
	return OSAL_SUCCESS;
}

const pdm_config_backend_ops_t g_lpf_config_static_backend = {
	.name = "static",
	.available = pdm_config_static_available,
	.load = pdm_config_static_load,
	.unload = pdm_config_static_unload,
	.active = pdm_config_static_active,
	.find = pdm_config_static_find,
	.list = pdm_config_static_list,
};
