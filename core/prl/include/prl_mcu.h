/**
 * @file prl_mcu.h
 * @brief MCU Communication Protocol (Deprecated - Use prl_device.h)
 * @details 本文件已废弃，请使用统一设备协议 prl_device.h
 *          保留此文件仅为兼容性，将在未来版本中删除
 *
 * @deprecated 使用 prl_device.h 中的统一接口替代：
 *   - prl_device_encode() 替代所有编码函数
 *   - prl_device_decode() 替代所有解码函数
 *   - PRL_MCU_MSG_* 替代 PRL_MCU_CMD_*
 */

#ifndef PRL_MCU_H
#define PRL_MCU_H

#include "prl_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 兼容性定义：将旧的命令码映射到新的消息类型 */
#define PRL_MCU_CMD_GET_VERSION     PRL_MCU_MSG_GET_VERSION
#define PRL_MCU_CMD_GET_STATUS      PRL_MCU_MSG_GET_STATUS
#define PRL_MCU_CMD_RESET           PRL_MCU_MSG_RESET
#define PRL_MCU_CMD_POWER_ON        PRL_MCU_MSG_POWER_ON
#define PRL_MCU_CMD_POWER_OFF       PRL_MCU_MSG_POWER_OFF

/* 兼容性类型定义 */
typedef prl_mcu_msg_type_t prl_mcu_cmd_t;
typedef prl_mcu_version_t prl_mcu_version_legacy_t;
typedef prl_mcu_status_t prl_mcu_status_legacy_t;

#ifdef __cplusplus
}
#endif

#endif /* PRL_MCU_H */
