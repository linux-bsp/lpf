// SPDX-License-Identifier: GPL-2.0

#include "osal.h"
#include "hal_serial.h"

int32_t hal_serial_open(const char *device, const hal_serial_config_t *config,
			hal_serial_handle_t *handle)
{
	(void)device;
	(void)config;

	if (!handle)
		return OSAL_ERR_INVALID_PARAM;

	*handle = NULL;
	return OSAL_ERR_NOT_SUPPORTED;
}

int32_t hal_serial_close(hal_serial_handle_t handle)
{
	if (!handle)
		return OSAL_ERR_INVALID_PARAM;

	return OSAL_ERR_NOT_SUPPORTED;
}

int32_t hal_serial_write(hal_serial_handle_t handle, const void *buffer,
			 uint32_t size, int32_t timeout)
{
	(void)size;
	(void)timeout;

	if (!handle || !buffer)
		return OSAL_ERR_INVALID_PARAM;

	return OSAL_ERR_NOT_SUPPORTED;
}

int32_t hal_serial_read(hal_serial_handle_t handle, void *buffer,
			uint32_t size, int32_t timeout)
{
	(void)size;
	(void)timeout;

	if (!handle || !buffer)
		return OSAL_ERR_INVALID_PARAM;

	return OSAL_ERR_NOT_SUPPORTED;
}

int32_t hal_serial_flush(hal_serial_handle_t handle)
{
	if (!handle)
		return OSAL_ERR_INVALID_PARAM;

	return OSAL_ERR_NOT_SUPPORTED;
}

int32_t hal_serial_set_config(hal_serial_handle_t handle,
			      const hal_serial_config_t *config)
{
	if (!handle || !config)
		return OSAL_ERR_INVALID_PARAM;

	return OSAL_ERR_NOT_SUPPORTED;
}
