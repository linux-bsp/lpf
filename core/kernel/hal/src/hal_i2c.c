// SPDX-License-Identifier: GPL-2.0

#include <linux/i2c.h>
#include <linux/module.h>

#include "osal.h"
#include "hal_i2c.h"

typedef struct {
	struct i2c_adapter *adapter;
	osal_mutex_t lock;
	uint32_t timeout;
	bool initialized;
} hal_i2c_context_t;

static int32_t hal_i2c_errno_to_status(int ret)
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

static bool hal_i2c_parse_adapter_id(const char *device, int *adapter_id)
{
	const char *name;

	if (!device || !adapter_id)
		return false;

	name = strrchr(device, '/');
	name = name ? name + 1 : device;

	if (sscanf(name, "i2c-%d", adapter_id) == 1)
		return true;

	if (sscanf(name, "%d", adapter_id) == 1)
		return true;

	return false;
}

int32_t hal_i2c_open(const hal_i2c_config_t *config, hal_i2c_handle_t *handle)
{
	hal_i2c_context_t *ctx;
	int adapter_id;
	int ret;

	if (!config || !handle || !config->device)
		return OSAL_ERR_INVALID_PARAM;

	*handle = NULL;

	if (!hal_i2c_parse_adapter_id(config->device, &adapter_id))
		return OSAL_ERR_INVALID_PARAM;

	ctx = osal_zalloc(sizeof(*ctx));
	if (!ctx)
		return OSAL_ERR_NO_MEMORY;

	ret = osal_mutex_init(&ctx->lock, NULL);
	if (ret != OSAL_SUCCESS)
		goto err_free;

	ctx->adapter = i2c_get_adapter(adapter_id);
	if (!ctx->adapter) {
		ret = OSAL_ENOENT;
		goto err_mutex;
	}

	ctx->timeout = config->timeout;
	ctx->initialized = true;
	*handle = ctx;

	LOG_INFO("HAL_I2C", "opened adapter i2c-%d", adapter_id);
	return OSAL_SUCCESS;

err_mutex:
	osal_mutex_destroy(&ctx->lock);
err_free:
	osal_free(ctx);
	return ret;
}
EXPORT_SYMBOL_GPL(hal_i2c_open);

int32_t hal_i2c_close(hal_i2c_handle_t handle)
{
	hal_i2c_context_t *ctx = handle;
	struct i2c_adapter *adapter;

	if (!handle)
		return OSAL_ERR_INVALID_PARAM;

	osal_mutex_lock(&ctx->lock);
	ctx->initialized = false;
	adapter = ctx->adapter;
	ctx->adapter = NULL;
	osal_mutex_unlock(&ctx->lock);

	if (adapter)
		i2c_put_adapter(adapter);

	osal_mutex_destroy(&ctx->lock);
	osal_free(ctx);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(hal_i2c_close);

int32_t hal_i2c_write(hal_i2c_handle_t handle, uint16_t slave_addr,
		      const uint8_t *buffer, uint32_t size)
{
	struct i2c_msg msg;
	int ret;

	if (!handle || !buffer || size == 0 || size > U16_MAX)
		return OSAL_ERR_INVALID_PARAM;

	msg.addr = slave_addr;
	msg.flags = 0;
	msg.len = (u16)size;
	msg.buf = (uint8_t *)buffer;

	ret = hal_i2c_transfer(handle, (hal_i2c_msg_t *)&msg, 1);
	return ret;
}
EXPORT_SYMBOL_GPL(hal_i2c_write);

int32_t hal_i2c_read(hal_i2c_handle_t handle, uint16_t slave_addr,
		     uint8_t *buffer, uint32_t size)
{
	struct i2c_msg msg;
	int ret;

	if (!handle || !buffer || size == 0 || size > U16_MAX)
		return OSAL_ERR_INVALID_PARAM;

	msg.addr = slave_addr;
	msg.flags = I2C_M_RD;
	msg.len = (u16)size;
	msg.buf = buffer;

	ret = hal_i2c_transfer(handle, (hal_i2c_msg_t *)&msg, 1);
	return ret;
}
EXPORT_SYMBOL_GPL(hal_i2c_read);

int32_t hal_i2c_write_reg(hal_i2c_handle_t handle, uint16_t slave_addr,
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

	ret = hal_i2c_write(handle, slave_addr, write_buf, size + 1);
	osal_free(write_buf);
	return ret;
}
EXPORT_SYMBOL_GPL(hal_i2c_write_reg);

int32_t hal_i2c_read_reg(hal_i2c_handle_t handle, uint16_t slave_addr,
			 uint8_t reg_addr, uint8_t *buffer, uint32_t size)
{
	hal_i2c_msg_t msgs[2];

	if (!handle || !buffer || size == 0 || size > U16_MAX)
		return OSAL_ERR_INVALID_PARAM;

	msgs[0].addr = slave_addr;
	msgs[0].flags = 0;
	msgs[0].len = 1;
	msgs[0].buf = &reg_addr;

	msgs[1].addr = slave_addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = (uint16_t)size;
	msgs[1].buf = buffer;

	return hal_i2c_transfer(handle, msgs, 2);
}
EXPORT_SYMBOL_GPL(hal_i2c_read_reg);

int32_t hal_i2c_transfer(hal_i2c_handle_t handle, hal_i2c_msg_t *msgs,
			 uint32_t num)
{
	hal_i2c_context_t *ctx = handle;
	struct i2c_msg *kernel_msgs;
	uint32_t i;
	int ret;

	if (!ctx || !msgs || num == 0 || num > 64)
		return OSAL_ERR_INVALID_PARAM;

	kernel_msgs = osal_zalloc(sizeof(*kernel_msgs) * num);
	if (!kernel_msgs)
		return OSAL_ERR_NO_MEMORY;

	for (i = 0; i < num; i++) {
		if (!msgs[i].buf || msgs[i].len == 0) {
			osal_free(kernel_msgs);
			return OSAL_ERR_INVALID_PARAM;
		}

		kernel_msgs[i].addr = msgs[i].addr;
		kernel_msgs[i].flags = msgs[i].flags;
		kernel_msgs[i].len = msgs[i].len;
		kernel_msgs[i].buf = msgs[i].buf;
	}

	osal_mutex_lock(&ctx->lock);
	if (!ctx->initialized || !ctx->adapter) {
		osal_mutex_unlock(&ctx->lock);
		osal_free(kernel_msgs);
		return OSAL_ERR_INVALID_ID;
	}

	ret = i2c_transfer(ctx->adapter, kernel_msgs, (int)num);
	osal_mutex_unlock(&ctx->lock);
	osal_free(kernel_msgs);

	if (ret == (int)num)
		return OSAL_SUCCESS;
	if (ret >= 0)
		return OSAL_ERR_GENERIC;

	LOG_ERROR("HAL_I2C", "transfer failed: %d", ret);
	return hal_i2c_errno_to_status(ret);
}
EXPORT_SYMBOL_GPL(hal_i2c_transfer);
