// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_COMPAT_SPI_H
#define PDM_COMPAT_SPI_H

#include "pdm/types/pdm_spi_types.h"

int32_t pdm_compat_spi_open(const pdm_spi_config_t *config,
			    pdm_spi_handle_t *handle);
void pdm_compat_spi_close(pdm_spi_handle_t handle);
int32_t pdm_compat_spi_transfer(pdm_spi_handle_t handle,
				const pdm_spi_transfer_t *transfers,
				uint32_t num);
int32_t pdm_compat_spi_set_config(pdm_spi_handle_t handle,
				  const pdm_spi_config_t *config);

#endif /* PDM_COMPAT_SPI_H */
