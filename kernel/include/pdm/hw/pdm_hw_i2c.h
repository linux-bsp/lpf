// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_HW_I2C_H
#define PDM_HW_I2C_H

#include "pdm/types/pdm_i2c_types.h"

int32_t pdm_hw_bus_i2c_open(const pdm_i2c_config_t *config,
			    pdm_hw_bus_i2c_handle_t *handle);
int32_t pdm_hw_bus_i2c_close(pdm_hw_bus_i2c_handle_t handle);
int32_t pdm_hw_bus_i2c_write(pdm_hw_bus_i2c_handle_t handle,
			     uint16_t slave_addr, const uint8_t *buffer,
			     uint32_t size);
int32_t pdm_hw_bus_i2c_read(pdm_hw_bus_i2c_handle_t handle,
			    uint16_t slave_addr, uint8_t *buffer,
			    uint32_t size);
int32_t pdm_hw_bus_i2c_write_reg(pdm_hw_bus_i2c_handle_t handle,
				 uint16_t slave_addr, uint8_t reg_addr,
				 const uint8_t *buffer, uint32_t size);
int32_t pdm_hw_bus_i2c_read_reg(pdm_hw_bus_i2c_handle_t handle,
				uint16_t slave_addr, uint8_t reg_addr,
				uint8_t *buffer, uint32_t size);
int32_t pdm_hw_bus_i2c_transfer(pdm_hw_bus_i2c_handle_t handle,
				pdm_i2c_msg_t *msgs, uint32_t num);

#endif /* PDM_HW_I2C_H */
