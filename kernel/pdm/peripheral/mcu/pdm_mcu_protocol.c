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

enum pdm_mcu_protocol_cmd {
	PDM_MCU_PROTOCOL_CMD_GET_VERSION = 0x00000001U,
	PDM_MCU_PROTOCOL_CMD_GET_STATUS = 0x00000002U,
	PDM_MCU_PROTOCOL_CMD_RESET = 0x00000003U,
};

static void pdm_mcu_protocol_encode_be32(u8 *buf, u32 value)
{
	buf[0] = (u8)(value >> 24);
	buf[1] = (u8)(value >> 16);
	buf[2] = (u8)(value >> 8);
	buf[3] = (u8)value;
}

static u32 pdm_mcu_protocol_max_tx(const struct pdm_mcu_instance *inst)
{
	if (inst->ops->max_tx_size) {
		return min_t(u32, inst->ops->max_tx_size,
			     PDM_MCU_MAX_TRANSFER_SIZE);
	}

	return PDM_MCU_MAX_TRANSFER_SIZE;
}

static u32 pdm_mcu_protocol_max_rx(const struct pdm_mcu_instance *inst)
{
	if (inst->ops->max_rx_size) {
		return min_t(u32, inst->ops->max_rx_size,
			     PDM_MCU_MAX_TRANSFER_SIZE);
	}

	return PDM_MCU_MAX_TRANSFER_SIZE;
}

static int pdm_mcu_protocol_xfer(struct pdm_mcu_instance *inst,
				   u32 command, const u8 *tx, u32 tx_len,
				   u8 *rx, u32 rx_len, u32 *actual_rx_len)
{
	struct pdm_mcu_xfer xfer;
	int ret;

	if (!inst->ops->xfer) {
		return -EOPNOTSUPP;
	}
	if ((tx_len && !tx) || (rx_len && !rx)) {
		return -EINVAL;
	}
	if (tx_len > pdm_mcu_protocol_max_tx(inst) ||
	    rx_len > pdm_mcu_protocol_max_rx(inst)) {
		return -EMSGSIZE;
	}

	xfer.command = command;
	xfer.tx = tx;
	xfer.tx_len = tx_len;
	xfer.rx = rx;
	xfer.rx_len = rx_len;
	xfer.actual_rx_len = 0;

	ret = inst->ops->xfer(inst, &xfer);
	if (ret) {
		return ret;
	}
	if (xfer.actual_rx_len > rx_len) {
		return -EMSGSIZE;
	}

	if (actual_rx_len) {
		*actual_rx_len = xfer.actual_rx_len;
	}
	return 0;
}

static void pdm_mcu_protocol_encode_index(u8 *buf, u32 index)
{
	pdm_mcu_protocol_encode_be32(buf, index);
}

int pdm_mcu_protocol_get_version(struct pdm_mcu_instance *inst,
				 struct pdm_mcu_version *version)
{
	u8 payload[sizeof(version->index)];
	u32 rx_len = sizeof(*version);
	int ret;

	pdm_mcu_protocol_encode_index(payload, version->index);
	ret = pdm_mcu_protocol_xfer(inst, PDM_MCU_PROTOCOL_CMD_GET_VERSION,
				     payload, sizeof(payload),
				     (u8 *)version, rx_len, &rx_len);
	if (ret) {
		return ret;
	}
	if (rx_len < sizeof(*version)) {
		return -EIO;
	}
	return 0;
}

int pdm_mcu_protocol_get_status(struct pdm_mcu_instance *inst,
			       struct pdm_mcu_status *status)
{
	u8 payload[sizeof(status->index)];
	u32 rx_len = sizeof(*status);
	int ret;

	pdm_mcu_protocol_encode_index(payload, status->index);
	ret = pdm_mcu_protocol_xfer(inst, PDM_MCU_PROTOCOL_CMD_GET_STATUS,
				     payload, sizeof(payload),
				     (u8 *)status, rx_len, &rx_len);
	if (ret) {
		return ret;
	}
	if (rx_len < sizeof(*status)) {
		return -EIO;
	}
	return 0;
}

int pdm_mcu_protocol_reset(struct pdm_mcu_instance *inst, u32 index)
{
	u8 payload[sizeof(index)];
	u32 rx_len = 0;

	pdm_mcu_protocol_encode_index(payload, index);
	return pdm_mcu_protocol_xfer(inst, PDM_MCU_PROTOCOL_CMD_RESET,
				     payload, sizeof(payload), NULL, 0, &rx_len);
}

int pdm_mcu_protocol_command(struct pdm_mcu_instance *inst,
			     struct pdm_mcu_command *command)
{
	u32 rx_len = command->rx_len;
	bool need_response = command->flags & PDM_MCU_CMD_F_NEED_RESPONSE;
	int ret;

	if (command->flags & ~PDM_MCU_CMD_F_NEED_RESPONSE) {
		return -EINVAL;
	}
	if (command->tx_len > PDM_MCU_MAX_TRANSFER_SIZE ||
	    command->rx_len > PDM_MCU_MAX_TRANSFER_SIZE) {
		return -EMSGSIZE;
	}
	if (!need_response && command->rx_len) {
		return -EINVAL;
	}

	ret = pdm_mcu_protocol_xfer(inst, command->command,
				     command->tx_data, command->tx_len,
				     need_response ? command->rx_data : NULL,
				     need_response ? rx_len : 0,
				     need_response ? &rx_len : NULL);
	if (ret) {
		return ret;
	}

	command->actual_rx_len = need_response ? rx_len : 0;
	return 0;
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("PDM MCU protocol layer");
