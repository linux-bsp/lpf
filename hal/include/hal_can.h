/************************************************************************
 * HAL层 - CAN驱动接口
 *
 * 提供统一的CAN总线访问接口
 ************************************************************************/

#ifndef HAL_CAN_H
#define HAL_CAN_H

#include "osal_types.h"
#include "config/can_types.h"

typedef void* hal_can_handle_t;

typedef struct
{
    const char *interface;  /* e.g., "can0", "vcan0" */
    uint32_t      baudrate;
    uint32_t      rx_timeout;
    uint32_t      tx_timeout;
} hal_can_config_t;

/**
 * @brief 初始化CAN驱动
 *
 * @param[in] config CAN配置
 * @param[out] handle 返回的CAN句柄
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_GENERIC 失败
 */
int32_t HAL_CAN_Init(const hal_can_config_t *config, hal_can_handle_t *handle);

/**
 * @brief 关闭CAN驱动
 *
 * @param[in] handle CAN句柄
 *
 * @return OSAL_SUCCESS 成功
 */
int32_t HAL_CAN_Deinit(hal_can_handle_t handle);

/**
 * @brief 发送CAN帧
 *
 * @param[in] handle CAN句柄
 * @param[in] frame CAN帧
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_TIMEOUT 超时
 * @return OSAL_ERR_GENERIC 其他错误
 */
int32_t HAL_CAN_Send(hal_can_handle_t handle, const can_frame_t *frame);

/**
 * @brief 接收CAN帧
 *
 * @param[in] handle CAN句柄
 * @param[out] frame 接收到的CAN帧
 * @param[in] timeout 超时时间(ms), OS_PEND表示永久等待
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_TIMEOUT 超时
 * @return OSAL_ERR_GENERIC 其他错误
 */
int32_t HAL_CAN_Recv(hal_can_handle_t handle, can_frame_t *frame, int32_t timeout);

/**
 * @brief 设置CAN过滤器
 *
 * @param[in] handle CAN句柄
 * @param[in] filter_id 过滤ID
 * @param[in] filter_mask 过滤掩码
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_GENERIC 失败
 */
int32_t HAL_CAN_SetFilter(hal_can_handle_t handle, uint32_t filter_id, uint32_t filter_mask);

/**
 * @brief 获取CAN统计信息
 *
 * @param[in] handle CAN句柄
 * @param[out] tx_count 发送计数
 * @param[out] rx_count 接收计数
 * @param[out] err_count 错误计数
 *
 * @return OSAL_SUCCESS 成功
 */
int32_t HAL_CAN_GetStats(hal_can_handle_t handle,
                       uint32_t *tx_count,
                       uint32_t *rx_count,
                       uint32_t *err_count);

/**
 * @brief 设置CAN错误回调函数
 *
 * @param[in] handle CAN句柄
 * @param[in] callback 错误回调函数
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_POINTER 参数无效
 */
int32_t HAL_CAN_SetErrorCallback(hal_can_handle_t handle,
                                  void (*callback)(hal_can_handle_t handle, int32_t error_code));

/**
 * @brief 设置CAN错误恢复阈值
 *
 * @param[in] handle CAN句柄
 * @param[in] threshold 连续错误次数阈值
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_POINTER 参数无效
 */
int32_t HAL_CAN_SetErrorThreshold(hal_can_handle_t handle, uint32_t threshold);

/**
 * @brief CAN错误状态
 */
typedef enum
{
    CAN_ERROR_ACTIVE = 0,   /* 正常状态 */
    CAN_ERROR_PASSIVE = 1,  /* 被动错误状态（错误计数>127） */
    CAN_BUS_OFF = 2         /* 总线关闭状态（错误计数>255） */
} hal_can_error_state_t;

/**
 * @brief CAN错误信息
 */
typedef struct
{
    uint32_t tx_error_count;        /* 发送错误计数 */
    uint32_t rx_error_count;        /* 接收错误计数 */
    hal_can_error_state_t state;    /* CAN控制器状态 */
} hal_can_error_info_t;

/**
 * @brief 获取CAN错误信息
 *
 * @param[in] handle CAN句柄
 * @param[out] info 错误信息
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_POINTER 参数无效
 * @return OSAL_ERR_NOT_SUPPORTED 平台不支持
 */
int32_t HAL_CAN_GetErrorInfo(hal_can_handle_t handle, hal_can_error_info_t *info);

#endif /* HAL_CAN_H */
