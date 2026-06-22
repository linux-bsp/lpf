// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_I2C_TYPES_H
#define PDM_I2C_TYPES_H

#include "osal.h"

typedef void *pdm_hw_bus_i2c_handle_t;
typedef void *pdm_i2c_handle_t;

#define PDM_I2C_M_RD      0x0001
#define PDM_I2C_M_TEN     0x0010
#define PDM_I2C_M_NOSTART 0x4000

typedef struct {
	const char *device;
	uint32_t timeout;
} pdm_i2c_config_t;

typedef struct {
	uint16_t addr;
	uint16_t flags;
	uint16_t len;
	uint8_t *buf;
} pdm_i2c_msg_t;

#endif /* PDM_I2C_TYPES_H */
