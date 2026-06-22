// SPDX-License-Identifier: GPL-2.0

#include "osal.h"
#include "pdm_config_backend.h"

#include <linux/moduleparam.h>

static char *pdm_config_backend_name = "auto";
module_param_named(backend, pdm_config_backend_name, charp, 0444);
MODULE_PARM_DESC(backend, "PDM_CONFIG backend: auto, static, or dt");

static const pdm_config_backend_ops_t *const g_lpf_config_backends[] = {
#if IS_ENABLED(CONFIG_PDM_CONFIG_SOURCE_STATIC)
	&g_lpf_config_static_backend,
#endif
#if IS_ENABLED(CONFIG_PDM_CONFIG_SOURCE_DT)
	&g_lpf_config_dt_backend,
#endif
};

static const pdm_config_backend_ops_t *
pdm_config_backend_find_by_name(const char *name)
{
	uint32_t i;

	if (!name)
		return NULL;

	for (i = 0; i < OSAL_ARRAY_SIZE(g_lpf_config_backends); i++) {
		const pdm_config_backend_ops_t *backend;

		backend = g_lpf_config_backends[i];
		if (!backend || !backend->name)
			continue;

		if (0 == osal_strcmp(backend->name, name))
			return backend;
	}

	return NULL;
}

bool pdm_config_backend_is_auto(void)
{
	return !pdm_config_backend_name ||
	       0 == osal_strcmp(pdm_config_backend_name, "auto");
}

uint32_t pdm_config_backend_count(void)
{
	return OSAL_ARRAY_SIZE(g_lpf_config_backends);
}

const pdm_config_backend_ops_t *pdm_config_backend_at(uint32_t index)
{
	if (index >= OSAL_ARRAY_SIZE(g_lpf_config_backends))
		return NULL;

	return g_lpf_config_backends[index];
}

const pdm_config_backend_ops_t *pdm_config_backend_select(void)
{
	uint32_t i;

	if (!pdm_config_backend_is_auto()) {
		const pdm_config_backend_ops_t *backend;

		backend = pdm_config_backend_find_by_name(pdm_config_backend_name);
		if (!backend || !backend->available || !backend->available()) {
			LOG_ERROR("PDM_CONFIG", "Requested backend unavailable: %s",
				  pdm_config_backend_name);
			return NULL;
		}

		return backend;
	}

	for (i = 0; i < OSAL_ARRAY_SIZE(g_lpf_config_backends); i++) {
		const pdm_config_backend_ops_t *backend;

		backend = g_lpf_config_backends[i];
		if (!backend || !backend->available)
			continue;

		if (backend->available())
			return backend;
	}

	return NULL;
}
