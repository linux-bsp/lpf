// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_MCU_TRANSPORT_H
#define PDM_MCU_TRANSPORT_H

#include "osal.h"
#include "pdm/config/pdm_config.h"

typedef void *pdm_mcu_transport_handle_t;

typedef struct {
	pdm_config_mcu_interface_t interface;
	const char *name;
	int32_t (*open)(const pdm_config_mcu_config_t *config,
			pdm_mcu_transport_handle_t *handle);
	int32_t (*close)(pdm_mcu_transport_handle_t handle);
	int32_t (*transfer)(pdm_mcu_transport_handle_t handle,
			    const uint8_t *packet, uint32_t packet_len,
			    uint8_t *response, uint32_t response_size,
			    uint32_t *actual_size, uint32_t timeout_ms);
} pdm_mcu_transport_ops_t;

const pdm_mcu_transport_ops_t *
pdm_mcu_transport_get(pdm_config_mcu_interface_t interface);

#endif /* PDM_MCU_TRANSPORT_H */
