// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_SPI_TYPES_H
#define PDM_SPI_TYPES_H

#include "osal.h"

typedef void *pdm_hw_bus_spi_handle_t;
typedef void *pdm_spi_handle_t;

#define PDM_SPI_MODE_0 0x00
#define PDM_SPI_MODE_1 0x01
#define PDM_SPI_MODE_2 0x02
#define PDM_SPI_MODE_3 0x03

typedef struct {
	const char *device;
	uint8_t mode;
	uint8_t bits_per_word;
	uint32_t max_speed_hz;
	uint32_t timeout;
} pdm_spi_config_t;

typedef struct {
	const uint8_t *tx_buf;
	uint8_t *rx_buf;
	uint32_t len;
	uint32_t speed_hz;
	uint16_t delay_usecs;
	uint8_t bits_per_word;
	uint8_t cs_change;
} pdm_spi_transfer_t;

#endif /* PDM_SPI_TYPES_H */
