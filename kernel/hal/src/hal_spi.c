// SPDX-License-Identifier: GPL-2.0

#include <linux/device/bus.h>
#include <linux/module.h>
#include <linux/spi/spi.h>

#include "osal.h"
#include "hal_spi.h"

typedef struct {
	struct spi_device *spi;
	osal_mutex_t lock;
	hal_spi_config_t config;
	bool initialized;
} hal_spi_context_t;

static int32_t hal_spi_errno_to_status(int ret)
{
	int err;

	if (ret >= 0)
		return OSAL_SUCCESS;

	err = -ret;
	if (err == EAGAIN || err == ETIMEDOUT)
		return OSAL_ERR_TIMEOUT;
	if (err == ENODEV || err == ENOENT)
		return OSAL_ENOENT;
	if (err == EOPNOTSUPP || err == ENOSYS)
		return OSAL_ERR_NOT_SUPPORTED;

	return err;
}

static struct spi_device *hal_spi_find_device(const char *name)
{
	struct device *dev;

	dev = bus_find_device_by_name(&spi_bus_type, NULL, name);
	if (!dev)
		return NULL;

	return to_spi_device(dev);
}

static int32_t hal_spi_apply_config(hal_spi_context_t *ctx,
				    const hal_spi_config_t *config)
{
	int ret;

	if (!ctx || !ctx->spi || !config)
		return OSAL_ERR_INVALID_PARAM;

	ctx->spi->mode = config->mode;
	ctx->spi->bits_per_word = config->bits_per_word ? config->bits_per_word : 8;
	ctx->spi->max_speed_hz = config->max_speed_hz;

	ret = spi_setup(ctx->spi);
	if (ret < 0)
		return hal_spi_errno_to_status(ret);

	ctx->config = *config;
	return OSAL_SUCCESS;
}

int32_t hal_spi_open(const hal_spi_config_t *config, hal_spi_handle_t *handle)
{
	hal_spi_context_t *ctx;
	const char *name;
	const char *slash;
	int ret;

	if (!config || !handle || !config->device)
		return OSAL_ERR_INVALID_PARAM;

	*handle = NULL;

	name = config->device;
	slash = strrchr(config->device, '/');
	if (slash)
		name = slash + 1;

	ctx = osal_zalloc(sizeof(*ctx));
	if (!ctx)
		return OSAL_ERR_NO_MEMORY;

	ret = osal_mutex_init(&ctx->lock, NULL);
	if (ret != OSAL_SUCCESS)
		goto err_free;

	ctx->spi = hal_spi_find_device(name);
	if (!ctx->spi) {
		ret = OSAL_ENOENT;
		goto err_mutex;
	}

	ret = hal_spi_apply_config(ctx, config);
	if (ret != OSAL_SUCCESS)
		goto err_device;

	ctx->initialized = true;
	*handle = ctx;

	LOG_INFO("HAL_SPI", "opened %s", name);
	return OSAL_SUCCESS;

err_device:
	spi_dev_put(ctx->spi);
err_mutex:
	osal_mutex_destroy(&ctx->lock);
err_free:
	osal_free(ctx);
	return ret;
}
EXPORT_SYMBOL_GPL(hal_spi_open);

int32_t hal_spi_close(hal_spi_handle_t handle)
{
	hal_spi_context_t *ctx = handle;
	struct spi_device *spi;

	if (!handle)
		return OSAL_ERR_INVALID_PARAM;

	osal_mutex_lock(&ctx->lock);
	ctx->initialized = false;
	spi = ctx->spi;
	ctx->spi = NULL;
	osal_mutex_unlock(&ctx->lock);

	if (spi)
		spi_dev_put(spi);

	osal_mutex_destroy(&ctx->lock);
	osal_free(ctx);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(hal_spi_close);

int32_t hal_spi_write(hal_spi_handle_t handle, const uint8_t *buffer,
		      uint32_t size)
{
	if (!handle || !buffer)
		return OSAL_ERR_INVALID_PARAM;

	return hal_spi_transfer(handle, buffer, NULL, size);
}
EXPORT_SYMBOL_GPL(hal_spi_write);

int32_t hal_spi_read(hal_spi_handle_t handle, uint8_t *buffer, uint32_t size)
{
	if (!handle || !buffer)
		return OSAL_ERR_INVALID_PARAM;

	return hal_spi_transfer(handle, NULL, buffer, size);
}
EXPORT_SYMBOL_GPL(hal_spi_read);

int32_t hal_spi_transfer(hal_spi_handle_t handle, const uint8_t *tx_buffer,
			 uint8_t *rx_buffer, uint32_t size)
{
	hal_spi_transfer_t transfer = {
		.tx_buf = tx_buffer,
		.rx_buf = rx_buffer,
		.len = size,
	};

	if (!handle || (!tx_buffer && !rx_buffer) || size == 0)
		return OSAL_ERR_INVALID_PARAM;

	return hal_spi_transfer_multi(handle, &transfer, 1);
}
EXPORT_SYMBOL_GPL(hal_spi_transfer);

int32_t hal_spi_transfer_multi(hal_spi_handle_t handle,
			       hal_spi_transfer_t *transfers, uint32_t num)
{
	hal_spi_context_t *ctx = handle;
	struct spi_transfer *xfers;
	struct spi_message message;
	uint32_t i;
	int ret;

	if (!ctx || !transfers || num == 0)
		return OSAL_ERR_INVALID_PARAM;

	xfers = osal_zalloc(sizeof(*xfers) * num);
	if (!xfers)
		return OSAL_ERR_NO_MEMORY;

	spi_message_init(&message);
	for (i = 0; i < num; i++) {
		if ((!transfers[i].tx_buf && !transfers[i].rx_buf) ||
		    transfers[i].len == 0) {
			osal_free(xfers);
			return OSAL_ERR_INVALID_PARAM;
		}

		xfers[i].tx_buf = transfers[i].tx_buf;
		xfers[i].rx_buf = transfers[i].rx_buf;
		xfers[i].len = transfers[i].len;
		xfers[i].speed_hz = transfers[i].speed_hz ?
					    transfers[i].speed_hz :
					    ctx->config.max_speed_hz;
		xfers[i].bits_per_word = transfers[i].bits_per_word ?
						 transfers[i].bits_per_word :
						 ctx->config.bits_per_word;
		xfers[i].delay.value = transfers[i].delay_usecs;
		xfers[i].delay.unit = SPI_DELAY_UNIT_USECS;
		xfers[i].cs_change = transfers[i].cs_change ? 1 : 0;
		spi_message_add_tail(&xfers[i], &message);
	}

	osal_mutex_lock(&ctx->lock);
	if (!ctx->initialized || !ctx->spi) {
		osal_mutex_unlock(&ctx->lock);
		osal_free(xfers);
		return OSAL_ERR_INVALID_ID;
	}

	ret = spi_sync(ctx->spi, &message);
	osal_mutex_unlock(&ctx->lock);
	osal_free(xfers);

	if (ret < 0) {
		LOG_ERROR("HAL_SPI", "transfer failed: %d", ret);
		return hal_spi_errno_to_status(ret);
	}

	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(hal_spi_transfer_multi);

int32_t hal_spi_set_config(hal_spi_handle_t handle,
			   const hal_spi_config_t *config)
{
	hal_spi_context_t *ctx = handle;
	int ret;

	if (!handle || !config)
		return OSAL_ERR_INVALID_PARAM;

	osal_mutex_lock(&ctx->lock);
	if (!ctx->initialized || !ctx->spi) {
		osal_mutex_unlock(&ctx->lock);
		return OSAL_ERR_INVALID_ID;
	}

	ret = hal_spi_apply_config(ctx, config);
	osal_mutex_unlock(&ctx->lock);
	return ret;
}
EXPORT_SYMBOL_GPL(hal_spi_set_config);
