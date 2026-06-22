// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_HW_SPI_H
#define PDM_HW_SPI_H

#include "pdm/types/pdm_spi_types.h"

int32_t pdm_hw_bus_spi_open(const pdm_spi_config_t *config,
			    pdm_hw_bus_spi_handle_t *handle);
int32_t pdm_hw_bus_spi_close(pdm_hw_bus_spi_handle_t handle);
int32_t pdm_hw_bus_spi_write(pdm_hw_bus_spi_handle_t handle,
			     const uint8_t *buffer, uint32_t size);
int32_t pdm_hw_bus_spi_read(pdm_hw_bus_spi_handle_t handle,
			    uint8_t *buffer, uint32_t size);
int32_t pdm_hw_bus_spi_transfer(pdm_hw_bus_spi_handle_t handle,
				const uint8_t *tx_buffer, uint8_t *rx_buffer,
				uint32_t size);
int32_t pdm_hw_bus_spi_transfer_multi(pdm_hw_bus_spi_handle_t handle,
				      pdm_spi_transfer_t *transfers,
				      uint32_t num);
int32_t pdm_hw_bus_spi_set_config(pdm_hw_bus_spi_handle_t handle,
				  const pdm_spi_config_t *config);

#endif /* PDM_HW_SPI_H */
