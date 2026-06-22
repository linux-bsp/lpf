// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>

#include "osal.h"
#include "pdm/hw/pdm_hw_i2c.h"
#include "pdm/soc/pdm_soc_adapter.h"

typedef struct {
	pdm_i2c_handle_t adapter;
	osal_mutex_t lock;
	uint32_t timeout;
	bool initialized;
} pdm_hw_bus_i2c_context_t;

static uint16_t pdm_hw_bus_i2c_to_lpf_flags(uint16_t flags)
{
	uint16_t pdm_flags = 0;

	if (flags & PDM_I2C_M_RD)
		pdm_flags |= PDM_I2C_M_RD;
	if (flags & PDM_I2C_M_TEN)
		pdm_flags |= PDM_I2C_M_TEN;
	if (flags & PDM_I2C_M_NOSTART)
		pdm_flags |= PDM_I2C_M_NOSTART;

	return pdm_flags;
}

int32_t pdm_hw_bus_i2c_open(const pdm_i2c_config_t *config, pdm_hw_bus_i2c_handle_t *handle)
{
	pdm_hw_bus_i2c_context_t *ctx;
	int32_t ret;

	if (!config || !handle || !config->device)
		return OSAL_ERR_INVALID_PARAM;

	*handle = NULL;

	ctx = osal_zalloc(sizeof(*ctx));
	if (!ctx)
		return OSAL_ERR_NO_MEMORY;

	ret = osal_mutex_init(&ctx->lock, NULL);
	if (ret != OSAL_SUCCESS)
		goto err_free;

	ret = pdm_soc_i2c_open(config->device, &ctx->adapter);
	if (ret != OSAL_SUCCESS)
		goto err_mutex;

	ctx->timeout = config->timeout;
	ctx->initialized = true;
	*handle = ctx;

	LOG_INFO("PDM_HW_BUS_I2C", "opened adapter %s", config->device);
	return OSAL_SUCCESS;

err_mutex:
	osal_mutex_destroy(&ctx->lock);
err_free:
	osal_free(ctx);
	return ret;
}
EXPORT_SYMBOL_GPL(pdm_hw_bus_i2c_open);

int32_t pdm_hw_bus_i2c_close(pdm_hw_bus_i2c_handle_t handle)
{
	pdm_hw_bus_i2c_context_t *ctx = handle;
	pdm_i2c_handle_t adapter;

	if (!handle)
		return OSAL_ERR_INVALID_PARAM;

	osal_mutex_lock(&ctx->lock);
	ctx->initialized = false;
	adapter = ctx->adapter;
	ctx->adapter = NULL;
	osal_mutex_unlock(&ctx->lock);

	if (adapter)
		pdm_soc_i2c_close(adapter);

	osal_mutex_destroy(&ctx->lock);
	osal_free(ctx);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(pdm_hw_bus_i2c_close);

int32_t pdm_hw_bus_i2c_write(pdm_hw_bus_i2c_handle_t handle, uint16_t slave_addr,
		      const uint8_t *buffer, uint32_t size)
{
	pdm_i2c_msg_t msg;
	int ret;

	if (!handle || !buffer || size == 0 || size > U16_MAX)
		return OSAL_ERR_INVALID_PARAM;

	msg.addr = slave_addr;
	msg.flags = 0;
	msg.len = (u16)size;
	msg.buf = (uint8_t *)buffer;

	ret = pdm_hw_bus_i2c_transfer(handle, (pdm_i2c_msg_t *)&msg, 1);
	return ret;
}
EXPORT_SYMBOL_GPL(pdm_hw_bus_i2c_write);

int32_t pdm_hw_bus_i2c_read(pdm_hw_bus_i2c_handle_t handle, uint16_t slave_addr,
		     uint8_t *buffer, uint32_t size)
{
	pdm_i2c_msg_t msg;
	int ret;

	if (!handle || !buffer || size == 0 || size > U16_MAX)
		return OSAL_ERR_INVALID_PARAM;

	msg.addr = slave_addr;
	msg.flags = PDM_I2C_M_RD;
	msg.len = (u16)size;
	msg.buf = buffer;

	ret = pdm_hw_bus_i2c_transfer(handle, (pdm_i2c_msg_t *)&msg, 1);
	return ret;
}
EXPORT_SYMBOL_GPL(pdm_hw_bus_i2c_read);

int32_t pdm_hw_bus_i2c_write_reg(pdm_hw_bus_i2c_handle_t handle, uint16_t slave_addr,
			  uint8_t reg_addr, const uint8_t *buffer,
			  uint32_t size)
{
	uint8_t *write_buf;
	int32_t ret;

	if (!handle || !buffer || size == 0 || size > U16_MAX - 1)
		return OSAL_ERR_INVALID_PARAM;

	write_buf = osal_malloc(size + 1);
	if (!write_buf)
		return OSAL_ERR_NO_MEMORY;

	write_buf[0] = reg_addr;
	osal_memcpy(&write_buf[1], buffer, size);

	ret = pdm_hw_bus_i2c_write(handle, slave_addr, write_buf, size + 1);
	osal_free(write_buf);
	return ret;
}
EXPORT_SYMBOL_GPL(pdm_hw_bus_i2c_write_reg);

int32_t pdm_hw_bus_i2c_read_reg(pdm_hw_bus_i2c_handle_t handle, uint16_t slave_addr,
			 uint8_t reg_addr, uint8_t *buffer, uint32_t size)
{
	pdm_i2c_msg_t msgs[2];

	if (!handle || !buffer || size == 0 || size > U16_MAX)
		return OSAL_ERR_INVALID_PARAM;

	msgs[0].addr = slave_addr;
	msgs[0].flags = 0;
	msgs[0].len = 1;
	msgs[0].buf = &reg_addr;

	msgs[1].addr = slave_addr;
	msgs[1].flags = PDM_I2C_M_RD;
	msgs[1].len = (uint16_t)size;
	msgs[1].buf = buffer;

	return pdm_hw_bus_i2c_transfer(handle, msgs, 2);
}
EXPORT_SYMBOL_GPL(pdm_hw_bus_i2c_read_reg);

int32_t pdm_hw_bus_i2c_transfer(pdm_hw_bus_i2c_handle_t handle, pdm_i2c_msg_t *msgs,
			 uint32_t num)
{
	pdm_hw_bus_i2c_context_t *ctx = handle;
	pdm_i2c_msg_t *pdm_msgs;
	uint32_t i;
	int32_t ret;

	if (!ctx || !msgs || num == 0 || num > 64)
		return OSAL_ERR_INVALID_PARAM;

	pdm_msgs = osal_zalloc(sizeof(*pdm_msgs) * num);
	if (!pdm_msgs)
		return OSAL_ERR_NO_MEMORY;

	for (i = 0; i < num; i++) {
		if (!msgs[i].buf || msgs[i].len == 0) {
			osal_free(pdm_msgs);
			return OSAL_ERR_INVALID_PARAM;
		}

		pdm_msgs[i].addr = msgs[i].addr;
		pdm_msgs[i].flags = pdm_hw_bus_i2c_to_lpf_flags(msgs[i].flags);
		pdm_msgs[i].len = msgs[i].len;
		pdm_msgs[i].buf = msgs[i].buf;
	}

	osal_mutex_lock(&ctx->lock);
	if (!ctx->initialized || !ctx->adapter) {
		osal_mutex_unlock(&ctx->lock);
		osal_free(pdm_msgs);
		return OSAL_ERR_INVALID_ID;
	}

	ret = pdm_soc_i2c_transfer(ctx->adapter, pdm_msgs, num);
	osal_mutex_unlock(&ctx->lock);
	osal_free(pdm_msgs);

	if (ret == OSAL_SUCCESS)
		return OSAL_SUCCESS;

	LOG_ERROR("PDM_HW_BUS_I2C", "transfer failed: %d", ret);
	return ret;
}
EXPORT_SYMBOL_GPL(pdm_hw_bus_i2c_transfer);
