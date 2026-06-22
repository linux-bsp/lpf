// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_COMPAT_I2C_H
#define PDM_COMPAT_I2C_H

#include "pdm/types/pdm_i2c_types.h"

int32_t pdm_compat_i2c_open(const char *device, pdm_i2c_handle_t *handle);
void pdm_compat_i2c_close(pdm_i2c_handle_t handle);
int32_t pdm_compat_i2c_transfer(pdm_i2c_handle_t handle,
				pdm_i2c_msg_t *msgs, uint32_t num);

#endif /* PDM_COMPAT_I2C_H */
