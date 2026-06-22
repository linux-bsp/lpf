// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>

#include "osal.h"
#include "pdm/hw/pdm_hw_can.h"
#include "pdm/soc/pdm_soc_adapter.h"

static void pdm_hw_transport_can_fill_lpf_config(const pdm_can_config_t *src,
				    pdm_can_config_t *dst)
{
	dst->interface = src->interface;
	dst->baudrate = src->baudrate;
	dst->rx_timeout = src->rx_timeout;
	dst->tx_timeout = src->tx_timeout;
}

static void pdm_hw_transport_can_fill_lpf_frame(const pdm_can_frame_t *src,
				   pdm_can_frame_t *dst)
{
	dst->can_id = src->can_id;
	dst->dlc = src->dlc;
	osal_memcpy(dst->data, src->data, sizeof(dst->data));
	dst->timestamp = src->timestamp;
}

static void pdm_hw_transport_can_fill_frame(const pdm_can_frame_t *src,
			       pdm_can_frame_t *dst)
{
	dst->can_id = src->can_id;
	dst->dlc = src->dlc;
	osal_memcpy(dst->data, src->data, sizeof(dst->data));
	dst->timestamp = src->timestamp;
}

int32_t pdm_hw_transport_can_init(
	const pdm_can_config_t *config, pdm_hw_transport_can_handle_t *handle)
{
	pdm_can_config_t pdm_config;

	if (!config || !handle)
		return OSAL_ERR_INVALID_PARAM;

	pdm_hw_transport_can_fill_lpf_config(config, &pdm_config);
	return pdm_soc_can_init(&pdm_config, (pdm_can_handle_t *)handle);
}
EXPORT_SYMBOL_GPL(pdm_hw_transport_can_init);

int32_t pdm_hw_transport_can_deinit(pdm_hw_transport_can_handle_t handle)
{
	return pdm_soc_can_deinit((pdm_can_handle_t)handle);
}
EXPORT_SYMBOL_GPL(pdm_hw_transport_can_deinit);

int32_t pdm_hw_transport_can_send(pdm_hw_transport_can_handle_t handle,
				  const pdm_can_frame_t *frame)
{
	pdm_can_frame_t pdm_frame;

	if (!handle || !frame)
		return OSAL_ERR_INVALID_PARAM;

	if (frame->dlc > PDM_CAN_MAX_DATA_LEN)
		return OSAL_ERR_INVALID_SIZE;

	pdm_hw_transport_can_fill_lpf_frame(frame, &pdm_frame);
	return pdm_soc_can_send((pdm_can_handle_t)handle, &pdm_frame);
}
EXPORT_SYMBOL_GPL(pdm_hw_transport_can_send);

int32_t pdm_hw_transport_can_recv(pdm_hw_transport_can_handle_t handle, pdm_can_frame_t *frame,
		     int32_t timeout)
{
	pdm_can_frame_t pdm_frame;
	int32_t ret;

	if (!handle || !frame)
		return OSAL_ERR_INVALID_PARAM;

	ret = pdm_soc_can_recv((pdm_can_handle_t)handle, &pdm_frame, timeout);
	if (ret != OSAL_SUCCESS)
		return ret;

	pdm_hw_transport_can_fill_frame(&pdm_frame, frame);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(pdm_hw_transport_can_recv);

int32_t pdm_hw_transport_can_set_filter(pdm_hw_transport_can_handle_t handle, uint32_t filter_id,
			   uint32_t filter_mask)
{
	return pdm_soc_can_set_filter((pdm_can_handle_t)handle, filter_id,
				      filter_mask);
}
EXPORT_SYMBOL_GPL(pdm_hw_transport_can_set_filter);
