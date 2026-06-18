// SPDX-License-Identifier: GPL-2.0

#include "osal.h"
#include "hal_can.h"

int32_t hal_can_init(const hal_can_config_t *config, hal_can_handle_t *handle)
{
	(void)config;

	if (!handle)
		return OSAL_ERR_INVALID_PARAM;

	*handle = NULL;
	return OSAL_ERR_NOT_SUPPORTED;
}

int32_t hal_can_deinit(hal_can_handle_t handle)
{
	if (!handle)
		return OSAL_ERR_INVALID_PARAM;

	return OSAL_ERR_NOT_SUPPORTED;
}

int32_t hal_can_send(hal_can_handle_t handle, const hal_can_frame_t *frame)
{
	if (!handle || !frame)
		return OSAL_ERR_INVALID_PARAM;

	return OSAL_ERR_NOT_SUPPORTED;
}

int32_t hal_can_recv(hal_can_handle_t handle, hal_can_frame_t *frame,
		     int32_t timeout)
{
	(void)timeout;

	if (!handle || !frame)
		return OSAL_ERR_INVALID_PARAM;

	return OSAL_ERR_NOT_SUPPORTED;
}

int32_t hal_can_set_filter(hal_can_handle_t handle, uint32_t filter_id,
			   uint32_t filter_mask)
{
	(void)filter_id;
	(void)filter_mask;

	if (!handle)
		return OSAL_ERR_INVALID_PARAM;

	return OSAL_ERR_NOT_SUPPORTED;
}
