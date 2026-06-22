// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>

#include "osal.h"
#include "pdm/hw/pdm_hw_uart.h"
#include "pdm/soc/pdm_soc_adapter.h"

static void pdm_hw_transport_uart_fill_lpf_config(const pdm_serial_config_t *src,
				       pdm_serial_config_t *dst)
{
	dst->baud_rate = src->baud_rate;
	dst->data_bits = src->data_bits;
	dst->stop_bits = src->stop_bits;
	dst->parity = src->parity;
	dst->flow_control = src->flow_control;
}

int32_t pdm_hw_transport_uart_open(const char *device, const pdm_serial_config_t *config,
			pdm_hw_transport_uart_handle_t *handle)
{
	pdm_serial_config_t pdm_config;

	if (!device || !config || !handle)
		return OSAL_ERR_INVALID_PARAM;

	pdm_hw_transport_uart_fill_lpf_config(config, &pdm_config);
	return pdm_soc_serial_open(device, &pdm_config,
				   (pdm_serial_handle_t *)handle);
}
EXPORT_SYMBOL_GPL(pdm_hw_transport_uart_open);

int32_t pdm_hw_transport_uart_close(pdm_hw_transport_uart_handle_t handle)
{
	return pdm_soc_serial_close((pdm_serial_handle_t)handle);
}
EXPORT_SYMBOL_GPL(pdm_hw_transport_uart_close);

int32_t pdm_hw_transport_uart_write(pdm_hw_transport_uart_handle_t handle, const void *buffer,
			 uint32_t size, int32_t timeout)
{
	return pdm_soc_serial_write((pdm_serial_handle_t)handle, buffer, size,
				    timeout);
}
EXPORT_SYMBOL_GPL(pdm_hw_transport_uart_write);

int32_t pdm_hw_transport_uart_read(pdm_hw_transport_uart_handle_t handle, void *buffer,
			uint32_t size, int32_t timeout)
{
	return pdm_soc_serial_read((pdm_serial_handle_t)handle, buffer, size,
				   timeout);
}
EXPORT_SYMBOL_GPL(pdm_hw_transport_uart_read);

int32_t pdm_hw_transport_uart_flush(pdm_hw_transport_uart_handle_t handle)
{
	return pdm_soc_serial_flush((pdm_serial_handle_t)handle);
}
EXPORT_SYMBOL_GPL(pdm_hw_transport_uart_flush);

int32_t pdm_hw_transport_uart_set_config(pdm_hw_transport_uart_handle_t handle,
			      const pdm_serial_config_t *config)
{
	pdm_serial_config_t pdm_config;

	if (!config)
		return OSAL_ERR_INVALID_PARAM;

	pdm_hw_transport_uart_fill_lpf_config(config, &pdm_config);
	return pdm_soc_serial_set_config((pdm_serial_handle_t)handle,
					 &pdm_config);
}
EXPORT_SYMBOL_GPL(pdm_hw_transport_uart_set_config);
