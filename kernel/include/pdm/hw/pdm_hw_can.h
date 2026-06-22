// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_HW_CAN_H
#define PDM_HW_CAN_H

#include "pdm/types/pdm_can_types.h"

int32_t pdm_hw_transport_can_init(
	const pdm_can_config_t *config,
	pdm_hw_transport_can_handle_t *handle);
int32_t pdm_hw_transport_can_deinit(pdm_hw_transport_can_handle_t handle);
int32_t pdm_hw_transport_can_send(pdm_hw_transport_can_handle_t handle,
				  const pdm_can_frame_t *frame);
int32_t pdm_hw_transport_can_recv(pdm_hw_transport_can_handle_t handle,
				  pdm_can_frame_t *frame, int32_t timeout);
int32_t pdm_hw_transport_can_set_filter(
	pdm_hw_transport_can_handle_t handle, uint32_t filter_id,
	uint32_t filter_mask);

#endif /* PDM_HW_CAN_H */
