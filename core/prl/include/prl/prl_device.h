/**
 * @file prl_device.h
 * @brief Protocol Layer Device Message Utilities
 */

#ifndef PRL_PRL_DEVICE_H
#define PRL_PRL_DEVICE_H

#include "prl_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========== Internal Device Message Functions ========== */

/**
 * @brief 设备消息编码参数
 */
typedef struct {
	uint8_t dev_type; /**< 设备类型 */
	uint8_t msg_type; /**< 消息类型 */
	uint8_t flags; /**< 标志位 */
	const void *payload; /**< payload 数据 */
	uint16_t payload_len; /**< payload 长度 */
	uint8_t *buffer; /**< 编码后的缓冲区 */
	size_t buffer_size; /**< 缓冲区大小 */
} prl_encode_ctx_t;

/**
 * @brief 设备消息解码参数
 */
typedef struct {
	const uint8_t *buffer; /**< 编码的数据 */
	size_t buffer_len; /**< 数据长度 */
	uint8_t dev_type; /**< 设备类型（输出） */
	uint8_t msg_type; /**< 消息类型（输出） */
	uint8_t flags; /**< 标志位（输出） */
	void *payload; /**< 解码后的 payload 缓冲区 */
	size_t payload_size; /**< payload 缓冲区大小 */
	uint32_t payload_len; /**< 实际 payload 长度（输出） */
} prl_decode_ctx_t;

/**
 * @brief 编码设备消息
 * @param[in,out] ctx 编码参数
 * @return 成功返回编码长度（>0），失败返回 OSAL 错误码
 */
int prl_device_encode(prl_encode_ctx_t *ctx);

/**
 * @brief 解码设备消息
 * @param[in,out] ctx 解码参数
 * @return OSAL_SUCCESS 成功，OSAL_ERR_* 失败
 */
int prl_device_decode(prl_decode_ctx_t *ctx);

bool prl_device_type_valid(uint8_t dev_type);
const char *prl_device_type_name(uint8_t dev_type);

#ifdef __cplusplus
}
#endif

#endif /* PRL_PRL_DEVICE_H */
