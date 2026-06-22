/* SPDX-License-Identifier: GPL-2.0 */

#ifndef PDM_CONFIG_VALIDATOR_H
#define PDM_CONFIG_VALIDATOR_H

#include "pdm/config/pdm_config.h"

typedef uint32_t (*pdm_config_device_count_fn)(
	const pdm_config_platform_config_t *platform);
typedef const void *(*pdm_config_device_entry_fn)(
	const pdm_config_platform_config_t *platform, uint32_t index);
typedef bool (*pdm_config_device_enabled_fn)(const void *entry);
typedef int32_t (*pdm_config_device_validate_fn)(uint32_t index,
					      const void *entry);
typedef void (*pdm_config_device_print_fn)(uint32_t index, const void *entry);

typedef struct {
	const char *name;
	const char *compatible;
	pdm_config_device_type_t type;
	uint32_t payload_size;
	pdm_config_device_count_fn count;
	pdm_config_device_entry_fn entry;
	pdm_config_device_enabled_fn enabled;
	const char *(*node_name)(const void *entry);
	pdm_config_device_validate_fn validate;
	pdm_config_device_print_fn print;
} pdm_config_device_descriptor_t;

const pdm_config_device_descriptor_t *pdm_config_device_descriptors(uint32_t *count);

#endif /* PDM_CONFIG_VALIDATOR_H */
