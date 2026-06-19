// SPDX-License-Identifier: GPL-2.0

#include "osal.h"
#include "pconfig_backend.h"

static const pconfig_backend_ops_t *const g_pconfig_backends[] = {
	&g_pconfig_static_backend,
};

const pconfig_backend_ops_t *pconfig_backend_select(void)
{
	uint32_t i;

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
