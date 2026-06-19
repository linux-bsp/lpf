// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_DRIVER_H
#define PDM_DRIVER_H

#include "pdm_internal.h"

int32_t pdm_driver_registry_init(void);
void pdm_driver_registry_deinit(void);
int pdm_drivers_init(void);
void pdm_drivers_exit(void);

#endif /* PDM_DRIVER_H */
