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
 * @brief 编码设备消息（内部函数）
 * @return 成功返回编码长度（>0），失败返回 OSAL 错误码
 */
int prl_device_encode(uint8_t dev_type, uint8_t msg_type,
                      const void *payload, uint16_t payload_len,
                      uint8_t *buffer, size_t buffer_size, uint8_t flags);

/**
 * @brief 解码设备消息（内部函数）
 * @param[in] buffer 接收到的完整报文
 * @param[in] buffer_len 报文长度
 * @param[out] dev_type 设备类型（可选）
 * @param[out] msg_type 消息类型（可选）
 * @param[out] flags 标志位（可选）
 * @param[out] payload Payload 缓冲区（可选）
 * @param[in] payload_size Payload 缓冲区大小
 * @param[out] actual_len 实际 Payload 长度（可选）
 * @return OSAL_SUCCESS 成功，OSAL_ERR_* 失败
 */
int prl_device_decode(const uint8_t *buffer, size_t buffer_len,
                      uint8_t *dev_type, uint8_t *msg_type, uint8_t *flags,
                      void *payload, size_t payload_size, uint32_t *actual_len);

bool prl_device_type_valid(uint8_t dev_type);
const char *prl_device_type_name(uint8_t dev_type);

#ifdef __cplusplus
}
#endif

#endif /* PRL_PRL_DEVICE_H */
