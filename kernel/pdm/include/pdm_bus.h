// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_BUS_H
#define PDM_BUS_H

#include "pdm_internal.h"

int32_t pdm_bus_init(void);
void pdm_bus_deinit(void);
int32_t pdm_bus_register_driver(const pdm_driver_t *driver);
void pdm_bus_unregister_driver(const pdm_driver_t *driver);
int32_t pdm_bus_register_device(const pconfig_device_config_t *device);
void pdm_bus_remove_devices(void);

#endif /* PDM_BUS_H */
