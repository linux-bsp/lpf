/* SPDX-License-Identifier: GPL-2.0 */

#ifndef PDM_CONFIG_NORMALIZER_H
#define PDM_CONFIG_NORMALIZER_H

#include "pdm/config/pdm_config.h"

int32_t pdm_config_normalize_devices(
	const pdm_config_platform_config_t *platform,
	pdm_config_device_config_t *devices, uint32_t *count);

int32_t pdm_config_build_device_nodes(
	const pdm_config_platform_config_t *platform,
	pdm_config_device_node_t *nodes, uint32_t *count);

#endif /* PDM_CONFIG_NORMALIZER_H */
