/* SPDX-License-Identifier: GPL-2.0 */

#ifndef PDM_CONFIG_BACKEND_H
#define PDM_CONFIG_BACKEND_H

#include "pdm/config/pdm_config.h"

typedef struct {
	const char *name;
	bool (*available)(void);
	int32_t (*load)(void);
	void (*unload)(void);
	const pdm_config_platform_config_t *(*active)(void);
	const pdm_config_platform_config_t *(*find)(const char *product,
						  const char *project,
						  const char *version);
	int32_t (*list)(const pdm_config_platform_config_t **configs,
			uint32_t *count);
} pdm_config_backend_ops_t;

bool pdm_config_backend_is_auto(void);
const pdm_config_backend_ops_t *pdm_config_backend_select(void);
const pdm_config_backend_ops_t *pdm_config_backend_at(uint32_t index);
uint32_t pdm_config_backend_count(void);

extern const pdm_config_backend_ops_t g_lpf_config_dt_backend;
extern const pdm_config_backend_ops_t g_lpf_config_static_backend;

#endif /* PDM_CONFIG_BACKEND_H */
