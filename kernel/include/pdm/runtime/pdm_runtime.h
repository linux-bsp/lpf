// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_RUNTIME_H
#define PDM_RUNTIME_H

#include "osal.h"

#define PDM_RUNTIME_VERSION_MAJOR 0x01
#define PDM_RUNTIME_VERSION_MINOR 0x00
#define PDM_RUNTIME_VERSION_PATCH 0x00

void pdm_runtime_print_version(void);
int32_t pdm_runtime_init(void);
void pdm_runtime_exit(void);
int32_t pdm_runtime_config_refresh(void);
void pdm_runtime_config_detach(void);

#endif /* PDM_RUNTIME_H */
