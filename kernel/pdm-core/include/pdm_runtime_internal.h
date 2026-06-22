// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_RUNTIME_INTERNAL_H
#define PDM_RUNTIME_INTERNAL_H

#include "osal.h"
#include "pdm/config/pdm_config.h"
#include "pdm/core/pdm_core.h"
#include "pdm/core/pdm_device.h"

#define PDM_RUNTIME_ENTRY_SECTION ".pdm_runtime_entry"
#define PDM_RUNTIME_CONFIG_DRIVER_SECTION ".pdm_runtime_config_driver"

typedef enum {
	PDM_RUNTIME_ENTRY_CLASS_CORE = 0,
	PDM_RUNTIME_ENTRY_CLASS_SERVICE,
	PDM_RUNTIME_ENTRY_CLASS_SELFTEST,
	PDM_RUNTIME_ENTRY_CLASS_COUNT,
} pdm_runtime_entry_class_t;

typedef struct {
	const char *name;
	pdm_runtime_entry_class_t class;
	int32_t (*init)(void);
	void (*exit)(void);
} pdm_runtime_entry_t;

typedef struct {
	pdm_config_device_type_t type;
	const char *compatible;
	int32_t (*probe)(const pdm_config_device_node_t *node);
} pdm_runtime_config_driver_t;

#define pdm_runtime_entry_register(_class, _id, _init, _exit)         \
	static const pdm_runtime_entry_t __lpf_runtime_entry_##_id \
		__attribute__((used, aligned(sizeof(void *)),             \
			       section(PDM_RUNTIME_ENTRY_SECTION))) = { \
			.name = #_id,                                    \
			.class = _class,                                 \
			.init = _init,                                   \
			.exit = _exit,                                   \
		}

#define pdm_runtime_core_register(_id, _init, _exit) \
	pdm_runtime_entry_register(PDM_RUNTIME_ENTRY_CLASS_CORE, _id, _init, _exit)

#define pdm_runtime_service_register(_id, _init, _exit) \
	pdm_runtime_entry_register(PDM_RUNTIME_ENTRY_CLASS_SERVICE, _id, _init, _exit)

#define pdm_runtime_selftest_register(_id, _init, _exit) \
	pdm_runtime_entry_register(PDM_RUNTIME_ENTRY_CLASS_SELFTEST, _id, _init, _exit)

#define pdm_runtime_config_driver_register(_id, _type, _probe)           \
	static const pdm_runtime_config_driver_t                         \
		__lpf_runtime_config_driver_##_id __attribute__((used,   \
		aligned(sizeof(void *)),                                 \
		section(PDM_RUNTIME_CONFIG_DRIVER_SECTION))) = {         \
			.type = _type,                                   \
			.probe = _probe,                                 \
		}

#define pdm_runtime_config_compatible_driver_register(_id, _type, _compat, \
						      _probe)               \
	static const pdm_runtime_config_driver_t                         \
		__lpf_runtime_config_driver_##_id __attribute__((used,   \
		aligned(sizeof(void *)),                                 \
		section(PDM_RUNTIME_CONFIG_DRIVER_SECTION))) = {         \
			.type = _type,                                   \
			.compatible = _compat,                           \
			.probe = _probe,                                 \
		}

extern const pdm_runtime_entry_t pdm_runtime_entry_start;
extern const pdm_runtime_entry_t pdm_runtime_entry_end;
extern const pdm_runtime_config_driver_t pdm_runtime_config_driver_start;
extern const pdm_runtime_config_driver_t pdm_runtime_config_driver_end;
int32_t pdm_runtime_probe_devices(void);

#endif /* PDM_RUNTIME_INTERNAL_H */
