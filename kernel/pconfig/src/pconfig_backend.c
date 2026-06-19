// SPDX-License-Identifier: GPL-2.0

#include "osal.h"
#include "pconfig_backend.h"

#include <linux/moduleparam.h>

static char *pconfig_backend_name = "auto";
module_param_named(backend, pconfig_backend_name, charp, 0444);
MODULE_PARM_DESC(backend, "PCONFIG backend: auto, dt, or static");

static const pconfig_backend_ops_t *const g_pconfig_backends[] = {
	&g_pconfig_dt_backend,
	&g_pconfig_static_backend,
};

static const pconfig_backend_ops_t *
pconfig_backend_find_by_name(const char *name)
{
	uint32_t i;

	if (!name)
		return NULL;

	for (i = 0; i < OSAL_ARRAY_SIZE(g_pconfig_backends); i++) {
		const pconfig_backend_ops_t *backend;

		backend = g_pconfig_backends[i];
		if (!backend || !backend->name)
			continue;

		if (0 == osal_strcmp(backend->name, name))
			return backend;
	}

	return NULL;
}

const pconfig_backend_ops_t *pconfig_backend_select(void)
{
	uint32_t i;

	if (pconfig_backend_name &&
	    0 != osal_strcmp(pconfig_backend_name, "auto")) {
		const pconfig_backend_ops_t *backend;

		backend = pconfig_backend_find_by_name(pconfig_backend_name);
		if (!backend || !backend->available || !backend->available()) {
			LOG_ERROR("PCONFIG", "Requested backend unavailable: %s",
				  pconfig_backend_name);
			return NULL;
		}

		return backend;
	}

	for (i = 0; i < OSAL_ARRAY_SIZE(g_pconfig_backends); i++) {
		const pconfig_backend_ops_t *backend;

		backend = g_pconfig_backends[i];
		if (!backend || !backend->available)
			continue;

		if (backend->available())
			return backend;
	}

	return NULL;
}
