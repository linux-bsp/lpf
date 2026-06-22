// SPDX-License-Identifier: GPL-2.0

#include "pdm/config/pdm_config_static.h"

const pdm_config_platform_config_t *const pdm_config_static_end
	__attribute__((used, aligned(sizeof(void *)),
		       section(PDM_CONFIG_STATIC_SECTION))) = NULL;
