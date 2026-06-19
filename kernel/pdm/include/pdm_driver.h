// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_DRIVER_H
#define PDM_DRIVER_H

#include "pdm_internal.h"

int32_t pdm_driver_registry_init(void);
void pdm_driver_registry_deinit(void);
const pdm_driver_t *pdm_driver_find(pconfig_device_type_t type);
int pdm_drivers_init(void);
void pdm_drivers_exit(void);
void pdm_drivers_remove_all(void);

#endif /* PDM_DRIVER_H */
