/* SPDX-License-Identifier: GPL-2.0 */

#ifndef PCONFIG_BACKEND_H
#define PCONFIG_BACKEND_H

#include "pconfig/pconfig.h"

typedef struct {
	const char *name;
	bool (*available)(void);
	int32_t (*load)(void);
	void (*unload)(void);
	const pconfig_platform_config_t *(*active)(void);
	const pconfig_platform_config_t *(*find)(const char *product,
						  const char *project,
						  const char *version);
	int32_t (*list)(const pconfig_platform_config_t **configs,
			uint32_t *count);
} pconfig_backend_ops_t;

const pconfig_backend_ops_t *pconfig_backend_select(void);

extern const pconfig_backend_ops_t g_pconfig_dt_backend;
extern const pconfig_backend_ops_t g_pconfig_static_backend;

#endif /* PCONFIG_BACKEND_H */
