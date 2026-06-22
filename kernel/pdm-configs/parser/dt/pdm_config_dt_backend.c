// SPDX-License-Identifier: GPL-2.0

#include "osal.h"
#include "pdm_config_backend.h"
#include "pdm_config_dt_parser.h"

#include <linux/of.h>

#define PDM_CONFIG_DT_ROOT_PATH "/lpf"
#define PDM_CONFIG_DT_COMPATIBLE "lpf,linux-peripheral-framework"
#define PDM_CONFIG_DT_LEGACY_COMPATIBLE "linux-peripheral-framework"
#define PDM_CONFIG_DT_OLD_COMPATIBLE "lpf,platform-config"

static pdm_config_dt_parse_result_t g_lpf_config_dt;
static bool g_lpf_config_dt_loaded;

static const char *pdm_config_dt_of_name(const void *node)
{
	const struct device_node *of_node = node;

	return of_node ? of_node->name : NULL;
}

static const void *pdm_config_dt_of_next_available_child(
	const void *node, const void *previous)
{
	return of_get_next_available_child((const struct device_node *)node,
					   (struct device_node *)previous);
}

static void pdm_config_dt_of_put_node(const void *node)
{
	of_node_put((struct device_node *)node);
}

static int32_t pdm_config_dt_of_read_string(
	const void *node, const char *property, const char **value)
{
	if (of_property_read_string(node, property, value) != 0)
		return OSAL_ERR_NAME_NOT_FOUND;

	return OSAL_SUCCESS;
}

static bool pdm_config_dt_of_read_bool(const void *node, const char *property)
{
	return of_property_read_bool(node, property);
}

static int32_t pdm_config_dt_of_read_u32(const void *node, const char *property,
					 uint32_t *value)
{
	if (of_property_read_u32(node, property, value) != 0)
		return OSAL_ERR_NAME_NOT_FOUND;

	return OSAL_SUCCESS;
}

static const pdm_config_dt_node_ops_t g_lpf_config_dt_of_ops = {
	.name = pdm_config_dt_of_name,
	.next_available_child = pdm_config_dt_of_next_available_child,
	.put_node = pdm_config_dt_of_put_node,
	.read_string = pdm_config_dt_of_read_string,
	.read_bool = pdm_config_dt_of_read_bool,
	.read_u32 = pdm_config_dt_of_read_u32,
	.alloc = osal_zalloc,
	.free = osal_free,
};

static struct device_node *pdm_config_dt_find_root(void)
{
	struct device_node *root;

	root = of_find_node_by_path(PDM_CONFIG_DT_ROOT_PATH);
	if (root)
		return root;

	root = of_find_compatible_node(NULL, NULL, PDM_CONFIG_DT_COMPATIBLE);
	if (root)
		return root;

	root = of_find_compatible_node(NULL, NULL,
				       PDM_CONFIG_DT_LEGACY_COMPATIBLE);
	if (root)
		return root;

	return of_find_compatible_node(NULL, NULL, PDM_CONFIG_DT_OLD_COMPATIBLE);
}

static void pdm_config_dt_free_entries(void)
{
	pdm_config_dt_parse_result_clear(&g_lpf_config_dt_of_ops,
					 &g_lpf_config_dt);
}

static bool pdm_config_dt_available(void)
{
	struct device_node *root;

	root = pdm_config_dt_find_root();
	if (!root)
		return false;

	of_node_put(root);
	return true;
}

static int32_t pdm_config_dt_load(void)
{
	struct device_node *root;
	int32_t ret;

	if (g_lpf_config_dt_loaded)
		return OSAL_SUCCESS;

	root = pdm_config_dt_find_root();
	if (!root)
		return OSAL_ERR_NOT_SUPPORTED;

	ret = pdm_config_dt_parse_platform(&g_lpf_config_dt_of_ops, root,
					   &g_lpf_config_dt);
	of_node_put(root);
	if (ret != OSAL_SUCCESS)
		return ret;

	g_lpf_config_dt_loaded = true;
	return OSAL_SUCCESS;
}

static void pdm_config_dt_unload(void)
{
	pdm_config_dt_free_entries();
	g_lpf_config_dt_loaded = false;
}

static const pdm_config_platform_config_t *pdm_config_dt_active(void)
{
	return g_lpf_config_dt_loaded ? &g_lpf_config_dt.platform : NULL;
}

static const pdm_config_platform_config_t *
pdm_config_dt_find(const char *product, const char *project, const char *version)
{
	const pdm_config_platform_config_t *config;

	config = pdm_config_dt_active();
	if (!config || !product || !project)
		return NULL;

	if (osal_strcmp(config->product_name, product) != 0)
		return NULL;

	if (osal_strcmp(config->project_name, project) != 0)
		return NULL;

	if (version && osal_strcmp(config->version, version) != 0)
		return NULL;

	return config;
}

static int32_t pdm_config_dt_list(const pdm_config_platform_config_t **configs,
				  uint32_t *count)
{
	if (!configs || !count)
		return OSAL_ERR_GENERIC;

	if (*count == 0) {
		*count = 0;
		return OSAL_SUCCESS;
	}

	if (!g_lpf_config_dt_loaded) {
		*count = 0;
		return OSAL_SUCCESS;
	}

	configs[0] = &g_lpf_config_dt.platform;
	*count = 1;
	return OSAL_SUCCESS;
}

const pdm_config_backend_ops_t g_lpf_config_dt_backend = {
	.name = "dt",
	.available = pdm_config_dt_available,
	.load = pdm_config_dt_load,
	.unload = pdm_config_dt_unload,
	.active = pdm_config_dt_active,
	.find = pdm_config_dt_find,
	.list = pdm_config_dt_list,
};
