// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_COMPAT_SERIAL_H
#define PDM_COMPAT_SERIAL_H

#include "pdm/types/pdm_serial_types.h"

int32_t pdm_compat_serial_open(const char *device,
			       const pdm_serial_config_t *config,
			       pdm_serial_handle_t *handle);
int32_t pdm_compat_serial_close(pdm_serial_handle_t handle);
int32_t pdm_compat_serial_write(pdm_serial_handle_t handle,
				const void *buffer, uint32_t size,
				int32_t timeout);
int32_t pdm_compat_serial_read(pdm_serial_handle_t handle, void *buffer,
			       uint32_t size, int32_t timeout);
int32_t pdm_compat_serial_flush(pdm_serial_handle_t handle);
int32_t pdm_compat_serial_set_config(pdm_serial_handle_t handle,
				     const pdm_serial_config_t *config);

#endif /* PDM_COMPAT_SERIAL_H */
