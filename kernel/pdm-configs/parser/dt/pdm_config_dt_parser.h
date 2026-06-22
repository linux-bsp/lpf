// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_CONFIG_DT_PARSER_H
#define PDM_CONFIG_DT_PARSER_H

#include "pdm/config/pdm_config.h"

typedef struct {
	const char *(*name)(const void *node);
	const void *(*next_available_child)(const void *node,
					    const void *previous);
	void (*put_node)(const void *node);
	int32_t (*read_string)(const void *node, const char *property,
			       const char **value);
	bool (*read_bool)(const void *node, const char *property);
	int32_t (*read_u32)(const void *node, const char *property,
			    uint32_t *value);
	void *(*alloc)(uint32_t size);
	void (*free)(void *ptr);
} pdm_config_dt_node_ops_t;

typedef struct {
	pdm_config_platform_config_t platform;
	pdm_config_mcu_entry_t *mcu_entries;
	pdm_config_led_entry_t *led_entries;
} pdm_config_dt_parse_result_t;

int32_t pdm_config_dt_parse_platform(
	const pdm_config_dt_node_ops_t *ops, const void *root,
	pdm_config_dt_parse_result_t *result);
void pdm_config_dt_parse_result_clear(
	const pdm_config_dt_node_ops_t *ops,
	pdm_config_dt_parse_result_t *result);

#endif /* PDM_CONFIG_DT_PARSER_H */
