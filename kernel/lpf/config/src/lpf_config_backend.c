// SPDX-License-Identifier: GPL-2.0

#include "osal.h"
#include "lpf_config_backend.h"

#include <linux/moduleparam.h>

static char *lpf_config_backend_name = "auto";
module_param_named(backend, lpf_config_backend_name, charp, 0444);
MODULE_PARM_DESC(backend, "LPF_CONFIG backend: auto, dt, or static");

static const lpf_config_backend_ops_t *const g_lpf_config_backends[] = {
	&g_lpf_config_dt_backend,
	&g_lpf_config_static_backend,
};

static const lpf_config_backend_ops_t *
lpf_config_backend_find_by_name(const char *name)
{
	uint32_t i;

	if (!name)
		return NULL;

	for (i = 0; i < OSAL_ARRAY_SIZE(g_lpf_config_backends); i++) {
		const lpf_config_backend_ops_t *backend;

		backend = g_lpf_config_backends[i];
		if (!backend || !backend->name)
			continue;

		if (0 == osal_strcmp(backend->name, name))
			return backend;
	}

	return NULL;
}

const lpf_config_backend_ops_t *lpf_config_backend_select(void)
{
	uint32_t i;

	if (lpf_config_backend_name &&
	    0 != osal_strcmp(lpf_config_backend_name, "auto")) {
		const lpf_config_backend_ops_t *backend;

		backend = lpf_config_backend_find_by_name(lpf_config_backend_name);
		if (!backend || !backend->available || !backend->available()) {
			LOG_ERROR("LPF_CONFIG", "Requested backend unavailable: %s",
				  lpf_config_backend_name);
			return NULL;
		}

		return backend;
	}

	for (i = 0; i < OSAL_ARRAY_SIZE(g_lpf_config_backends); i++) {
		const lpf_config_backend_ops_t *backend;

		backend = g_lpf_config_backends[i];
		if (!backend || !backend->available)
			continue;

		if (backend->available())
			return backend;
	}

	return NULL;
}
