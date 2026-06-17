/**
 * @file prl_device.c
 * @brief Protocol Layer Device Messages Implementation
 * @details 统一设备协议消息实现
 */

#include "osal.h"

#include "prl.h"


/* 设备类型名称映射表 */
static const char *device_type_names[] = {
	"UNKNOWN",      /* 0x00 */
	"MCU",          /* 0x01 */
	"CCM",          /* 0x02 */
	"PMC",          /* 0x03 */
	"GSC",          /* 0x04 */
	"POWER",        /* 0x05 */
};

bool prl_device_type_valid(uint8_t dev_type)
{
	return (dev_type >= PRL_DEV_TYPE_MCU && dev_type <= PRL_DEV_TYPE_POWER);
}

const char *prl_device_type_name(uint8_t dev_type)
{
	if (dev_type < OSAL_sizeof(device_type_names) / OSAL_sizeof(device_type_names[0])) {
		return device_type_names[dev_type];
	}
	return "INVALID";
}

int prl_device_encode(uint8_t dev_type, uint8_t msg_type,
                      const void *payload, uint16_t payload_len,
                      uint8_t *buffer, size_t buffer_size, uint8_t flags)
{
	prl_header_t *hdr;
	size_t total_len;

	/* 参数检查 */
	if (!buffer) {
		return OSAL_ERR_INVALID_PARAM;
	}

	if (!prl_device_type_valid(dev_type)) {
		return OSAL_ERR_INVALID_PARAM;
	}

	total_len = PRL_HEADER_SIZE + payload_len;
	if (total_len > buffer_size) {
		return OSAL_ERR_NO_MEMORY;
	}

	/* 填充协议头 */
	hdr = (prl_header_t *)buffer;
	hdr->magic = OSAL_htons(PRL_MAGIC);
	hdr->version = PRL_VERSION;
	hdr->dev_type = dev_type;
	hdr->msg_type = msg_type;
	hdr->flags = flags;
	hdr->length = OSAL_htons(payload_len);
	hdr->seq = 0;
	hdr->timestamp = OSAL_htonl((uint32_t)(OSAL_get_monotonic_time() / 1000000));  /* 微秒转秒 */
	hdr->reserved = 0;

	/* 复制payload */
	if (payload && payload_len > 0) {
		OSAL_memcpy(buffer + PRL_HEADER_SIZE, payload, payload_len);
	}

	/* 计算CRC（覆盖整个包，CRC字段除外） */
	hdr->crc16 = 0;
	hdr->crc16 = OSAL_htons(OSAL_crc16_modbus(buffer, total_len));

	return (int)total_len;
}

int prl_device_decode(const uint8_t *buffer, size_t buffer_len,
                      uint8_t *dev_type, uint8_t *msg_type, uint8_t *flags,
                      void *payload, size_t payload_size, uint32_t *actual_len)
{
	const prl_header_t *hdr;
	uint16_t magic;
	uint16_t payload_len;
	uint16_t calc_crc;
	uint16_t recv_crc;

	/* 参数检查 */
	if (!buffer || buffer_len < PRL_HEADER_SIZE) {
		return OSAL_ERR_INVALID_PARAM;
	}

	hdr = (const prl_header_t *)buffer;

	/* 验证魔数 */
	magic = OSAL_ntohs(hdr->magic);
	if (magic != PRL_MAGIC) {
		return OSAL_ERR_GENERIC;
	}

	/* 验证版本 */
	if (hdr->version != PRL_VERSION) {
		return OSAL_ERR_GENERIC;
	}

	/* 验证设备类型 */
	if (!prl_device_type_valid(hdr->dev_type)) {
		return OSAL_ERR_GENERIC;
	}

	/* 获取payload长度 */
	payload_len = OSAL_ntohs(hdr->length);
	if (PRL_HEADER_SIZE + payload_len > buffer_len) {
		return OSAL_ERR_GENERIC;
	}

	/* 验证CRC */
	recv_crc = OSAL_ntohs(hdr->crc16);
	((prl_header_t *)hdr)->crc16 = 0;  /* 临时清零 */
	calc_crc = OSAL_crc16_modbus(buffer, PRL_HEADER_SIZE + payload_len);
	((prl_header_t *)hdr)->crc16 = OSAL_htons(recv_crc);  /* 恢复 */

	if (calc_crc != recv_crc) {
		return OSAL_ERR_GENERIC;
	}

	/* 输出参数 */
	if (dev_type) {
		*dev_type = hdr->dev_type;
	}
	if (msg_type) {
		*msg_type = hdr->msg_type;
	}
	if (flags) {
		*flags = hdr->flags;
	}

	/* 复制payload */
	if (payload && payload_size > 0 && payload_len > 0) {
		size_t copy_len = (payload_len < payload_size) ? payload_len : payload_size;
		OSAL_memcpy(payload, buffer + PRL_HEADER_SIZE, copy_len);
		if (actual_len) {
			*actual_len = copy_len;
		}
	} else if (actual_len) {
		*actual_len = payload_len;
	}

	return OSAL_SUCCESS;
}
