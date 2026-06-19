/* SPDX-License-Identifier: GPL-2.0 */

#ifndef PCONFIG_VALIDATOR_H
#define PCONFIG_VALIDATOR_H

#include "pconfig/pconfig.h"

typedef uint32_t (*pconfig_device_count_fn)(
	const pconfig_platform_config_t *platform);
typedef const void *(*pconfig_device_entry_fn)(
	const pconfig_platform_config_t *platform, uint32_t index);
typedef bool (*pconfig_device_enabled_fn)(const void *entry);
typedef int32_t (*pconfig_device_validate_fn)(uint32_t index,
					      const void *entry);
typedef void (*pconfig_device_print_fn)(uint32_t index, const void *entry);

typedef struct {
	const char *name;
	pconfig_device_type_t type;
	pconfig_device_count_fn count;
	pconfig_device_entry_fn entry;
	pconfig_device_enabled_fn enabled;
	pconfig_device_validate_fn validate;
	pconfig_device_print_fn print;
} pconfig_device_descriptor_t;

const pconfig_device_descriptor_t *pconfig_device_descriptors(uint32_t *count);

#endif /* PCONFIG_VALIDATOR_H */
