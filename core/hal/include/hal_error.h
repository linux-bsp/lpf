/************************************************************************
 * HAL层 - 错误码定义和错误处理
 *
 * 提供统一的HAL错误码体系和errno映射机制
 ************************************************************************/

#ifndef HAL_ERROR_H
#define HAL_ERROR_H

#include <stdint.h>
#include "lib/osal_errno.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************
 * HAL错误码定义
 ************************************************************************/

/* 成功 */
#define HAL_SUCCESS             0

/* 通用错误 */
#define HAL_ERR_GENERIC         -1
#define HAL_ERR_INVALID_PARAM   -2
#define HAL_ERR_INVALID_POINTER -3
#define HAL_ERR_INVALID_ID      -4
#define HAL_ERR_NO_MEMORY       -5
#define HAL_ERR_TIMEOUT         -6
#define HAL_ERR_BUSY            -7
#define HAL_ERR_NOT_SUPPORTED   -8
#define HAL_ERR_PERMISSION      -9
#define HAL_ERR_NAME_TOO_LONG   -10

/* 设备相关错误 */
#define HAL_ERR_NO_DEVICE       -20
#define HAL_ERR_DEVICE_NOT_FOUND -21
#define HAL_ERR_DEVICE_BUSY     -22
#define HAL_ERR_DEVICE_ERROR    -23

/* I/O错误 */
#define HAL_ERR_IO              -30
#define HAL_ERR_READ_FAILED     -31
#define HAL_ERR_WRITE_FAILED    -32
#define HAL_ERR_PARTIAL_IO      -33

/* 通信错误 */
#define HAL_ERR_COMM_FAILED     -40
#define HAL_ERR_NO_RESPONSE     -41
#define HAL_ERR_CHECKSUM        -42

/************************************************************************
 * 错误上下文管理
 ************************************************************************/

#define HAL_ERROR_MSG_SIZE 256

typedef struct {
    int32_t hal_error;      /* HAL错误码 */
    int32_t sys_errno;      /* 系统errno */
    char message[HAL_ERROR_MSG_SIZE];  /* 错误消息 */
} hal_error_context_t;

/**
 * @brief 设置HAL错误上下文
 *
 * @param hal_err HAL错误码
 * @param sys_err 系统errno
 * @param fmt 格式化字符串
 * @param ... 可变参数
 */
void HAL_SetErrorContext(int32_t hal_err, int32_t sys_err, const char *fmt, ...);

/**
 * @brief 获取最后的HAL错误上下文
 *
 * @return 错误上下文指针（线程局部存储）
 */
const hal_error_context_t* HAL_GetLastError(void);

/**
 * @brief 清除错误上下文
 */
void HAL_ClearError(void);

/************************************************************************
 * errno到HAL错误码映射
 ************************************************************************/

/**
 * @brief 将系统errno映射为HAL错误码
 *
 * @param sys_errno 系统errno值
 * @return HAL错误码
 */
int32_t HAL_ErrnoToError(int32_t sys_errno);

/************************************************************************
 * 便捷宏
 ************************************************************************/

/**
 * @brief 设置错误并记录上下文
 *
 * 用法：
 *   HAL_SET_ERROR(hal_err, errno, "Failed to open %s", device);
 */
#define HAL_SET_ERROR(hal_err, sys_err, ...) \
    HAL_SetErrorContext(hal_err, sys_err, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* HAL_ERROR_H */
