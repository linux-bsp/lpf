// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_DRIVER_H
#define PDM_DRIVER_H

#include "pdm/core/pdm_device.h"

typedef struct pdm_driver {
	const char *name;
	/* Match key used by PDM Core. One driver owns one PDM device type. */
	pdm_device_type_t type;
	pdm_capability_t capabilities;
	int (*init)(void);
	void (*exit)(void);
	int32_t (*probe)(const pdm_device_t *device);
	void (*remove)(const pdm_device_t *device);
} pdm_driver_t;

#endif /* PDM_DRIVER_H */
