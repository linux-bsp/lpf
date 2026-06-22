/************************************************************************
 * 硬件配置库API实现
 *
 * 命名规范：
 * - PDM_CONFIG_*       - 通用接口
 * - PDM_CONFIG_HW_*    - 硬件配置接口
 ************************************************************************/

#include "osal.h"
#include "pdm/config/pdm_config.h"
#include "pdm_config_backend.h"
#include "pdm_config_normalizer.h"
#include "pdm_config_validator.h"
#include "generated/gen_version.h"

static const pdm_config_backend_ops_t *g_lpf_config_backend;
static bool g_lpf_config_initialized;
static pdm_config_device_node_t *g_lpf_config_device_nodes;
static uint32_t g_lpf_config_device_node_count;

const pdm_config_platform_config_t *pdm_config_get_board(void)
{
	if (!g_lpf_config_backend || !g_lpf_config_backend->active)
		return NULL;

	return g_lpf_config_backend->active();
}
EXPORT_SYMBOL_GPL(pdm_config_get_board);

const pdm_config_device_node_t *pdm_config_get_device_nodes(uint32_t *count)
{
	if (count)
		*count = g_lpf_config_device_node_count;

	if (!g_lpf_config_initialized)
		return NULL;

	return g_lpf_config_device_nodes;
}
EXPORT_SYMBOL_GPL(pdm_config_get_device_nodes);

const pdm_config_platform_config_t *pdm_config_find(const char *product,
					      const char *project,
					      const char *version)
{
	const pdm_config_backend_ops_t *backend;

	if (!g_lpf_config_initialized)
		return NULL;

	backend = g_lpf_config_backend;
	if (!backend || !backend->find)
		return NULL;

	return backend->find(product, project, version);
}
EXPORT_SYMBOL_GPL(pdm_config_find);

int32_t pdm_config_list(const pdm_config_platform_config_t **configs, uint32_t *count)
{
	const pdm_config_backend_ops_t *backend;

	if (!g_lpf_config_initialized) {
		if (count)
			*count = 0;
		return OSAL_ERR_INVALID_STATE;
	}

	backend = g_lpf_config_backend;
	if (!backend || !backend->list)
		return OSAL_ERR_GENERIC;

	return backend->list(configs, count);
}
EXPORT_SYMBOL_GPL(pdm_config_list);

static void pdm_config_clear_device_nodes(void)
{
	osal_free(g_lpf_config_device_nodes);
	g_lpf_config_device_nodes = NULL;
	g_lpf_config_device_node_count = 0;
}

static int32_t pdm_config_cache_device_nodes(
	const pdm_config_platform_config_t *platform)
{
	pdm_config_device_node_t *nodes;
	uint32_t count = 0;
	int32_t ret;

	ret = pdm_config_build_device_nodes(platform, NULL, &count);
	if (ret != OSAL_SUCCESS)
		return ret;

	pdm_config_clear_device_nodes();
	if (count == 0)
		return OSAL_SUCCESS;

	nodes = osal_zalloc(sizeof(*nodes) * (count + 1U));
	if (!nodes)
		return OSAL_ERR_NO_MEMORY;

	g_lpf_config_device_node_count = count + 1U;
	ret = pdm_config_build_device_nodes(platform, nodes,
					    &g_lpf_config_device_node_count);
	if (ret != OSAL_SUCCESS) {
		osal_free(nodes);
		g_lpf_config_device_node_count = 0;
		return ret;
	}

	g_lpf_config_device_nodes = nodes;
	return OSAL_SUCCESS;
}

static void pdm_config_unload_backend(const pdm_config_backend_ops_t *backend)
{
	pdm_config_clear_device_nodes();
	if (backend && backend->unload)
		backend->unload();
	if (g_lpf_config_backend == backend)
		g_lpf_config_backend = NULL;
}

static int32_t pdm_config_try_backend(
	const pdm_config_backend_ops_t *backend)
{
	const pdm_config_platform_config_t *platform;
	int32_t ret;

	if (!backend || !backend->load || !backend->active)
		return OSAL_ERR_INVALID_PARAM;

	if (backend->available && !backend->available())
		return OSAL_ERR_NOT_SUPPORTED;

	ret = backend->load();
	if (ret != OSAL_SUCCESS)
		return ret;

	g_lpf_config_backend = backend;
	platform = pdm_config_get_board();
	if (!platform) {
		ret = OSAL_ERR_GENERIC;
		goto out_unload;
	}

	ret = pdm_config_validate(platform);
	if (ret != OSAL_SUCCESS) {
		LOG_ERROR("PDM_CONFIG", "Invalid platform config from %s",
			  backend->name ? backend->name : "unknown");
		goto out_unload;
	}

	ret = pdm_config_cache_device_nodes(platform);
	if (ret != OSAL_SUCCESS) {
		LOG_ERROR("PDM_CONFIG", "Failed to build device nodes from %s",
			  backend->name ? backend->name : "unknown");
		goto out_unload;
	}

	LOG_INFO("PDM_CONFIG", "Backend: %s",
		 backend->name ? backend->name : "unknown");
	pdm_config_print(platform);
	return OSAL_SUCCESS;

out_unload:
	pdm_config_unload_backend(backend);
	return ret;
}

static int32_t pdm_config_load_auto(void)
{
	int32_t last_ret = OSAL_ERR_GENERIC;
	uint32_t i;

	for (i = 0; i < pdm_config_backend_count(); i++) {
		const pdm_config_backend_ops_t *backend;
		int32_t ret;

		backend = pdm_config_backend_at(i);
		if (!backend)
			continue;

		ret = pdm_config_try_backend(backend);
		if (ret == OSAL_SUCCESS)
			return OSAL_SUCCESS;

		last_ret = ret;
		LOG_WARN("PDM_CONFIG", "Backend %s unavailable: %d",
			 backend->name ? backend->name : "unknown", ret);
	}

	return last_ret;
}

static bool pdm_config_payload_size_valid(
	pdm_config_device_type_t type, uint32_t payload_size)
{
	const pdm_config_device_descriptor_t *descriptors;
	uint32_t descriptor_count;
	uint32_t i;

	descriptors = pdm_config_device_descriptors(&descriptor_count);
	for (i = 0; i < descriptor_count; i++) {
		if (descriptors[i].type == type &&
		    descriptors[i].payload_size == payload_size)
			return true;
	}

	return false;
}

static int32_t pdm_config_validate_device_nodes(
	const pdm_config_platform_config_t *config)
{
	uint32_t i;

	if (!config->device_nodes && config->device_node_count > 0) {
		LOG_ERROR("PDM_CONFIG", "Missing configured-device node table");
		return OSAL_ERR_GENERIC;
	}

	for (i = 0; config->device_nodes &&
	     i < config->device_node_count; i++) {
		const pdm_config_device_node_t *node = &config->device_nodes[i];

		if (node->device_type == PDM_CONFIG_DEVICE_TYPE_INVALID) {
			LOG_ERROR("PDM_CONFIG", "Device node[%u] invalid type", i);
			return OSAL_ERR_INVALID_PARAM;
		}

		if (!node->payload || !node->entry) {
			LOG_ERROR("PDM_CONFIG", "Device node[%u] missing payload",
				  i);
			return OSAL_ERR_INVALID_PARAM;
		}

		if (!pdm_config_payload_size_valid(node->device_type,
						   node->payload_size)) {
			LOG_ERROR("PDM_CONFIG",
				  "Device node[%u] invalid payload size", i);
			return OSAL_ERR_INVALID_PARAM;
		}
	}

	return OSAL_SUCCESS;
}

int32_t pdm_config_validate(const pdm_config_platform_config_t *config)
{
	const pdm_config_device_descriptor_t *descriptors;
	uint32_t descriptor_count;
	uint32_t desc_index;
	uint32_t i;
	int32_t ret;

	if (NULL == config)
		return OSAL_ERR_GENERIC;

	if (NULL == config->platform_name || NULL == config->chip_name ||
	    NULL == config->project_name || NULL == config->product_name ||
	    NULL == config->version) {
		LOG_ERROR("PDM_CONFIG", "Missing platform identity");
		return OSAL_ERR_GENERIC;
	}

	if (config->mcu_count > 0 && NULL == config->mcu_array) {
		LOG_ERROR("PDM_CONFIG", "Missing MCU config array");
		return OSAL_ERR_GENERIC;
	}

	if (config->led_count > 0 && NULL == config->led_array) {
		LOG_ERROR("PDM_CONFIG", "Missing LED config array");
		return OSAL_ERR_GENERIC;
	}

	ret = pdm_config_validate_device_nodes(config);
	if (ret != OSAL_SUCCESS)
		return ret;

	descriptors = pdm_config_device_descriptors(&descriptor_count);
	for (desc_index = 0; desc_index < descriptor_count; desc_index++) {
		const pdm_config_device_descriptor_t *desc;
		uint32_t count;

		desc = &descriptors[desc_index];
		count = desc->count(config);
		for (i = 0; i < count; i++) {
			ret = desc->validate(i, desc->entry(config, i));
			if (ret != OSAL_SUCCESS)
				return ret;
		}
	}

	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(pdm_config_validate);

void pdm_config_print(const pdm_config_platform_config_t *config)
{
	const pdm_config_device_descriptor_t *descriptors;
	uint32_t descriptor_count;
	uint32_t desc_index;

	if (NULL == config)
		return;

	LOG_INFO("PDM_CONFIG", "Platform: %s", config->platform_name);
	LOG_INFO("PDM_CONFIG", "Chip: %s", config->chip_name);
	LOG_INFO("PDM_CONFIG", "Project: %s", config->project_name);
	LOG_INFO("PDM_CONFIG", "Product: %s", config->product_name);
	LOG_INFO("PDM_CONFIG", "Version: %s", config->version);

	descriptors = pdm_config_device_descriptors(&descriptor_count);
	for (desc_index = 0; desc_index < descriptor_count; desc_index++) {
		const pdm_config_device_descriptor_t *desc;
		uint32_t count;
		uint32_t i;

		desc = &descriptors[desc_index];
		count = desc->count(config);
		for (i = 0; i < count; i++)
			desc->print(i, desc->entry(config, i));
	}
}
EXPORT_SYMBOL_GPL(pdm_config_print);

void pdm_config_print_version(void)
{
	osal_log(OS_LOG_LEVEL_INFO, "PDM_CONFIG",
		 "========================================");
	osal_log(OS_LOG_LEVEL_INFO, "PDM_CONFIG",
		 "module_version : %u.%u.%u",
		 PDM_CONFIG_VERSION_MAJOR, PDM_CONFIG_VERSION_MINOR,
		 PDM_CONFIG_VERSION_PATCH);
	osal_log(OS_LOG_LEVEL_INFO, "PDM_CONFIG",
		 "pdm_version    : %s", PDM_VERSION);
	osal_log(OS_LOG_LEVEL_INFO, "PDM_CONFIG",
		 "git_commit     : %s", PDM_GIT_COMMIT);
	osal_log(OS_LOG_LEVEL_INFO, "PDM_CONFIG",
		 "build_time     : %s", PDM_COMPILE_TIME);
	osal_log(OS_LOG_LEVEL_INFO, "PDM_CONFIG",
		 "build_by       : %s@%s",
		 PDM_COMPILE_BY, PDM_COMPILE_HOST);
	osal_log(OS_LOG_LEVEL_INFO, "PDM_CONFIG",
		 "compiler       : %s", PDM_COMPILER);
	osal_log(OS_LOG_LEVEL_INFO, "PDM_CONFIG",
		 "arch           : %s", PDM_BUILD_ARCH);
	osal_log(OS_LOG_LEVEL_INFO, "PDM_CONFIG",
		 "kernel         : %s", PDM_BUILD_KERNEL);
	osal_log(OS_LOG_LEVEL_INFO, "PDM_CONFIG",
		 "========================================");
}
EXPORT_SYMBOL_GPL(pdm_config_print_version);

int32_t pdm_config_load(void)
{
	const pdm_config_backend_ops_t *backend;
	int32_t ret;

	if (g_lpf_config_initialized)
		return OSAL_SUCCESS;

	if (pdm_config_backend_is_auto()) {
		ret = pdm_config_load_auto();
		if (ret != OSAL_SUCCESS) {
			LOG_ERROR("PDM_CONFIG", "No usable config backend");
			return ret;
		}

		g_lpf_config_initialized = true;
		return OSAL_SUCCESS;
	}

	backend = pdm_config_backend_select();
	if (NULL == backend) {
		LOG_ERROR("PDM_CONFIG", "No available config backend");
		return OSAL_ERR_GENERIC;
	}

	ret = pdm_config_try_backend(backend);
	if (ret != OSAL_SUCCESS) {
		LOG_ERROR("PDM_CONFIG", "Config backend load failed");
		return ret;
	}

	g_lpf_config_initialized = true;
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(pdm_config_load);

void pdm_config_unload(void)
{
	pdm_config_unload_backend(g_lpf_config_backend);
	g_lpf_config_initialized = false;
	LOG_INFO("PDM_CONFIG", "Deinitialized");
}
EXPORT_SYMBOL_GPL(pdm_config_unload);
