// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_SERIAL_TYPES_H
#define PDM_SERIAL_TYPES_H

#include "osal.h"

typedef void *pdm_hw_transport_uart_handle_t;
typedef void *pdm_serial_handle_t;

#define PDM_SERIAL_PARITY_NONE 0x00
#define PDM_SERIAL_PARITY_ODD  0x01
#define PDM_SERIAL_PARITY_EVEN 0x02

#define PDM_SERIAL_FLOW_NONE 0x00
#define PDM_SERIAL_FLOW_HW   0x01
#define PDM_SERIAL_FLOW_SW   0x02

typedef struct {
	uint32_t baud_rate;
	uint8_t data_bits;
	uint8_t stop_bits;
	uint8_t parity;
	uint8_t flow_control;
} pdm_serial_config_t;

#endif /* PDM_SERIAL_TYPES_H */
