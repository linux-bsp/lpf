// SPDX-License-Identifier: GPL-2.0
/**
 * @file pdm_mcu_protocol.c
 * @brief PDM MCU protocol layer between ioctl semantics and transport ops
 */

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>

#include "pdm_mcu_internal.h"

static void pdm_mcu_protocol_encode_be32(u8 *buf, u32 value)
{
	buf[0] = (u8)(value >> 24);
	buf[1] = (u8)(value >> 16);
	buf[2] = (u8)(value >> 8);
	buf[3] = (u8)value;
}

static u32 pdm_mcu_protocol_decode_be32(const u8 *buf)
{
	return ((u32)buf[0] << 24) | ((u32)buf[1] << 16) |
	       ((u32)buf[2] << 8) | (u32)buf[3];
}

static void pdm_mcu_protocol_encode_prefix(u8 *buf, u32 value, u8 bytes)
{
	u8 i;

	for (i = 0; i < bytes; i++)
		buf[i] = (u8)(value >> (8U * (bytes - i - 1U)));
}

static u8 pdm_mcu_protocol_command_prefix(const struct pdm_mcu_instance *inst)
{
	switch (inst->ops->type) {
	case PDM_MCU_BACKEND_I2C:
		return inst->transport.i2c.command_bytes;
	case PDM_MCU_BACKEND_SPI:
		return inst->transport.spi.command_bytes;
	default:
		return 0;
	}
}

static u8 pdm_mcu_protocol_address_prefix(const struct pdm_mcu_instance *inst)
{
	switch (inst->ops->type) {
	case PDM_MCU_BACKEND_I2C:
		return inst->transport.i2c.address_bytes;
	case PDM_MCU_BACKEND_SPI:
		return inst->transport.spi.address_bytes;
	default:
		return 0;
	}
}

static u32 pdm_mcu_protocol_max_tx(const struct pdm_mcu_instance *inst)
{
	if (inst->ops->max_tx_size)
		return inst->ops->max_tx_size;

	return PDM_MCU_MAX_TRANSFER_SIZE;
}

static u32 pdm_mcu_protocol_max_rx(const struct pdm_mcu_instance *inst)
{
	if (inst->ops->max_rx_size)
		return inst->ops->max_rx_size;

	return PDM_MCU_MAX_TRANSFER_SIZE;
}

static int pdm_mcu_protocol_xfer(struct pdm_mcu_instance *inst,
					 const u8 *tx, u32 tx_len,
					 u8 *rx, u32 rx_len)
{
	if (!inst->ops->xfer)
		return -EOPNOTSUPP;
	if (tx_len > pdm_mcu_protocol_max_tx(inst) ||
	    rx_len > pdm_mcu_protocol_max_rx(inst))
		return -EMSGSIZE;

	return inst->ops->xfer(inst, tx, tx_len, rx, rx_len);
}

static u32 pdm_mcu_protocol_frame_payload_max(const struct pdm_mcu_instance *inst)
{
	u32 max_rx = pdm_mcu_protocol_max_rx(inst);

	if (max_rx <= PDM_MCU_TRANSPORT_ID_BYTES)
		return 0;

	return max_rx - PDM_MCU_TRANSPORT_ID_BYTES;
}

static int pdm_mcu_protocol_write_exact(struct pdm_mcu_instance *inst,
					const u8 *buf, u32 len)
{
	int ret;

	if (!inst->ops->write)
		return -EOPNOTSUPP;
	if (len > pdm_mcu_protocol_max_tx(inst))
		return -EMSGSIZE;

	ret = inst->ops->write(inst, buf, len);
	if (ret < 0)
		return ret;
	return ret == len ? 0 : -EIO;
}

int pdm_mcu_protocol_reset(struct pdm_mcu_instance *inst)
{
	(void)inst;
	return 0;
}

int pdm_mcu_protocol_command(struct pdm_mcu_instance *inst,
			     struct pdm_mcu_command *command)
{
	u8 tx[PDM_MCU_MAX_TRANSFER_SIZE + PDM_MCU_MAX_PREFIX_BYTES];
	u8 rx[PDM_MCU_MAX_TRANSFER_SIZE + PDM_MCU_TRANSPORT_ID_BYTES];
	u32 tx_len;
	u32 rx_len;
	u8 prefix;
	int ret;

	if (command->tx_len > PDM_MCU_MAX_TRANSFER_SIZE ||
	    command->rx_len > PDM_MCU_MAX_TRANSFER_SIZE)
		return -EMSGSIZE;

	if (inst->ops->flags & PDM_MCU_TRANSPORT_F_FRAME_ID_U32) {
		if (command->tx_len + PDM_MCU_TRANSPORT_ID_BYTES > sizeof(tx) ||
		    command->rx_len + PDM_MCU_TRANSPORT_ID_BYTES > sizeof(rx))
			return -EMSGSIZE;

		pdm_mcu_protocol_encode_be32(tx, command->command);
		if (command->tx_len)
			memcpy(tx + PDM_MCU_TRANSPORT_ID_BYTES, command->tx_data,
			       command->tx_len);
		tx_len = command->tx_len + PDM_MCU_TRANSPORT_ID_BYTES;
		rx_len = command->rx_len ?
			 command->rx_len + PDM_MCU_TRANSPORT_ID_BYTES : 0;

		ret = pdm_mcu_protocol_xfer(inst, tx, tx_len, rx, rx_len);
		if (ret < 0)
			return ret;
		if (!command->rx_len)
			return 0;
		if (ret < PDM_MCU_TRANSPORT_ID_BYTES)
			return -EIO;
		command->rx_len = ret - PDM_MCU_TRANSPORT_ID_BYTES;
		memcpy(command->rx_data, rx + PDM_MCU_TRANSPORT_ID_BYTES,
		       command->rx_len);
		return 0;
	}

	prefix = pdm_mcu_protocol_command_prefix(inst);
	if (command->tx_len + prefix > sizeof(tx))
		return -EMSGSIZE;

	if (prefix)
		pdm_mcu_protocol_encode_prefix(tx, command->command, prefix);
	if (command->tx_len)
		memcpy(tx + prefix, command->tx_data, command->tx_len);
	tx_len = prefix + command->tx_len;

	ret = pdm_mcu_protocol_xfer(inst, tx, tx_len, command->rx_data,
					 command->rx_len);
	if (ret < 0)
		return ret;
	if (inst->ops->flags & PDM_MCU_TRANSPORT_F_BYTE_STREAM)
		command->rx_len = ret;
	return 0;
}

int pdm_mcu_protocol_read_data(struct pdm_mcu_instance *inst,
			       struct pdm_mcu_data *data)
{
	u8 tx[PDM_MCU_MAX_PREFIX_BYTES];
	u8 rx[PDM_MCU_MAX_TRANSFER_SIZE + PDM_MCU_TRANSPORT_ID_BYTES];
	u32 rx_len;
	u8 prefix;
	int ret;

	if (data->len > PDM_MCU_MAX_TRANSFER_SIZE)
		return -EMSGSIZE;

	if (inst->ops->flags & PDM_MCU_TRANSPORT_F_BYTE_STREAM) {
		if (!inst->ops->read)
			return -EOPNOTSUPP;
		if (data->len > pdm_mcu_protocol_max_rx(inst))
			return -EMSGSIZE;

		ret = inst->ops->read(inst, data->data, data->len);
		if (ret < 0)
			return ret;
		data->len = ret;
		return 0;
	}

	if (inst->ops->flags & PDM_MCU_TRANSPORT_F_FRAME_ID_U32) {
		if (!inst->ops->read)
			return -EOPNOTSUPP;

		rx_len = pdm_mcu_protocol_frame_payload_max(inst);
		if (data->len)
			rx_len = min_t(u32, data->len, rx_len);
		rx_len += PDM_MCU_TRANSPORT_ID_BYTES;
		if (rx_len > sizeof(rx) || rx_len > pdm_mcu_protocol_max_rx(inst))
			return -EMSGSIZE;

		ret = inst->ops->read(inst, rx, rx_len);
		if (ret < 0)
			return ret;
		if (ret < PDM_MCU_TRANSPORT_ID_BYTES)
			return -EIO;
		data->address = pdm_mcu_protocol_decode_be32(rx);
		data->len = ret - PDM_MCU_TRANSPORT_ID_BYTES;
		if (data->len)
			memcpy(data->data, rx + PDM_MCU_TRANSPORT_ID_BYTES,
			       data->len);
		return 0;
	}

	prefix = pdm_mcu_protocol_address_prefix(inst);
	if (prefix)
		pdm_mcu_protocol_encode_prefix(tx, data->address, prefix);

	return pdm_mcu_protocol_xfer(inst, tx, prefix, data->data, data->len);
}

int pdm_mcu_protocol_write_data(struct pdm_mcu_instance *inst,
				const struct pdm_mcu_data *data)
{
	u8 tx[PDM_MCU_MAX_TRANSFER_SIZE + PDM_MCU_MAX_PREFIX_BYTES];
	u8 prefix;

	if (data->len > PDM_MCU_MAX_TRANSFER_SIZE)
		return -EMSGSIZE;

	if (inst->ops->flags & PDM_MCU_TRANSPORT_F_BYTE_STREAM)
		return pdm_mcu_protocol_write_exact(inst, data->data, data->len);

	if (inst->ops->flags & PDM_MCU_TRANSPORT_F_FRAME_ID_U32) {
		if (data->len + PDM_MCU_TRANSPORT_ID_BYTES > sizeof(tx))
			return -EMSGSIZE;

		pdm_mcu_protocol_encode_be32(tx, data->address);
		if (data->len)
			memcpy(tx + PDM_MCU_TRANSPORT_ID_BYTES, data->data,
			       data->len);
		return pdm_mcu_protocol_write_exact(inst, tx,
			data->len + PDM_MCU_TRANSPORT_ID_BYTES);
	}

	prefix = pdm_mcu_protocol_address_prefix(inst);
	if (data->len + prefix > sizeof(tx))
		return -EMSGSIZE;
	if (prefix)
		pdm_mcu_protocol_encode_prefix(tx, data->address, prefix);
	if (data->len)
		memcpy(tx + prefix, data->data, data->len);

	return pdm_mcu_protocol_xfer(inst, tx, prefix + data->len, NULL, 0);
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("PDM MCU protocol layer");
