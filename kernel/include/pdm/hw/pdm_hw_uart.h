// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_HW_UART_H
#define PDM_HW_UART_H

#include "pdm/types/pdm_serial_types.h"

int32_t pdm_hw_transport_uart_open(
	const char *device, const pdm_serial_config_t *config,
	pdm_hw_transport_uart_handle_t *handle);
int32_t pdm_hw_transport_uart_close(pdm_hw_transport_uart_handle_t handle);
int32_t pdm_hw_transport_uart_write(pdm_hw_transport_uart_handle_t handle,
				    const void *buffer, uint32_t size,
				    int32_t timeout);
int32_t pdm_hw_transport_uart_read(pdm_hw_transport_uart_handle_t handle,
				   void *buffer, uint32_t size,
				   int32_t timeout);
int32_t pdm_hw_transport_uart_flush(pdm_hw_transport_uart_handle_t handle);
int32_t pdm_hw_transport_uart_set_config(
	pdm_hw_transport_uart_handle_t handle,
	const pdm_serial_config_t *config);

#endif /* PDM_HW_UART_H */
