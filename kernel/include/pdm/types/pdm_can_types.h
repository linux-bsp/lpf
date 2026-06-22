// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_CAN_TYPES_H
#define PDM_CAN_TYPES_H

#include "osal.h"

typedef void *pdm_hw_transport_can_handle_t;
typedef void *pdm_can_handle_t;

#define PDM_CAN_DEFAULT_INTERFACE "can0"
#define PDM_CAN_DEFAULT_BAUDRATE 500000
#define PDM_CAN_MAX_DATA_LEN 8

typedef struct {
	const char *interface;
	uint32_t baudrate;
	uint32_t rx_timeout;
	uint32_t tx_timeout;
} pdm_can_config_t;

typedef struct {
	uint32_t can_id;
	uint8_t dlc;
	uint8_t data[PDM_CAN_MAX_DATA_LEN];
	uint32_t timestamp;
} pdm_can_frame_t;

#endif /* PDM_CAN_TYPES_H */
