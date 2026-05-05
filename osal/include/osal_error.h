/************************************************************************
 * OSAL 错误码定义
 *
 * 设计原则：
 * 1. 成功返回 0
 * 2. 错误返回正数，与 Linux errno 保持一致
 * 3. 使用 OSAL_ERR_ 前缀提供语义化别名
 * 4. 便于问题定位，无需重新查表
 ************************************************************************/

#ifndef OSAL_ERROR_H
#define OSAL_ERROR_H

#include <stdint.h>
#include <errno.h>

/*
 * 状态码类型
 */
typedef int32_t osal_status_t;

/*
 * 成功
 */
#define OSAL_SUCCESS                    0

/*
 * 通用错误码 - 直接映射到 Linux errno
 */
#define OSAL_ERR_INVALID_POINTER        EFAULT          /* 14: 无效指针 */
#define OSAL_ERR_NO_MEMORY              ENOMEM          /* 12: 内存不足 */
#define OSAL_ERR_INVALID_SIZE           EINVAL          /* 22: 无效参数/大小 */
#define OSAL_ERR_INVALID_ID             EINVAL          /* 22: 无效ID */
#define OSAL_ERR_NAME_TOO_LONG          ENAMETOOLONG    /* 63: 名称过长 */
#define OSAL_ERR_NAME_TAKEN             EEXIST          /* 17: 名称已存在 */
#define OSAL_ERR_NAME_NOT_FOUND         ENOENT          /* 2: 名称未找到 */
#define OSAL_ERR_TIMEOUT                ETIMEDOUT       /* 60(macOS)/110(Linux): 超时 */
#define OSAL_ERR_NOT_IMPLEMENTED        ENOSYS          /* 78: 未实现 */
#define OSAL_ERR_BUSY                   EBUSY           /* 16: 资源忙 */
#define OSAL_ERR_PERMISSION             EPERM           /* 1: 权限不足 */
#define OSAL_ERR_NOT_SUPPORTED          ENOTSUP         /* 45: 不支持的操作 */
#define OSAL_ERR_ALREADY_EXISTS         EEXIST          /* 17: 已存在 */
#define OSAL_ERR_WOULD_BLOCK            EWOULDBLOCK     /* 35: 操作会阻塞 */
#define OSAL_ERR_INTERRUPTED            EINTR           /* 4: 操作被中断 */
#define OSAL_ERR_BAD_ADDRESS            EFAULT          /* 14: 错误的地址 */
#define OSAL_ERR_RESOURCE_LIMIT         EMFILE          /* 24: 资源限制 */
#define OSAL_ERR_GENERIC                EIO             /* 5: 通用I/O错误 */

/*
 * 特定模块错误码 - 使用未被 errno 占用的值 (200+)
 */
#define OSAL_ERR_ADDRESS_MISALIGNED     200             /* 地址未对齐 */
#define OSAL_ERR_INVALID_INT_NUM        201             /* 无效中断号 */
#define OSAL_ERR_INVALID_PRIORITY       202             /* 无效优先级 */
#define OSAL_ERR_INVALID_STATE          203             /* 无效状态 */
#define OSAL_ERR_NO_FREE_IDS            204             /* 无可用ID */

/*
 * 信号量相关错误码
 */
#define OSAL_ERR_SEM_FAILURE            210             /* 信号量失败 */
#define OSAL_ERR_SEM_TIMEOUT            ETIMEDOUT       /* 60/110: 信号量超时 */
#define OSAL_ERR_SEM_NOT_FULL           211             /* 信号量未满 */
#define OSAL_ERR_INVALID_SEM_VALUE      212             /* 无效信号量值 */

/*
 * 队列相关错误码
 */
#define OSAL_ERR_QUEUE_EMPTY            220             /* 队列为空 */
#define OSAL_ERR_QUEUE_FULL             221             /* 队列已满 */
#define OSAL_ERR_QUEUE_TIMEOUT          ETIMEDOUT       /* 60/110: 队列超时 */
#define OSAL_ERR_QUEUE_INVALID_SIZE     EINVAL          /* 22: 队列大小无效 */
#define OSAL_ERR_QUEUE_ID               222             /* 队列ID错误 */

/*
 * 文件相关错误码
 */
#define OSAL_ERR_FILE                   EIO             /* 5: 文件错误 */

/*
 * 定时器相关错误码
 */
#define OSAL_ERR_TIMER_INVALID_ARGS     230             /* 定时器参数无效 */
#define OSAL_ERR_TIMER_ID               231             /* 定时器ID错误 */
#define OSAL_ERR_TIMER_UNAVAILABLE      EAGAIN          /* 35: 定时器不可用 */
#define OSAL_ERR_TIMER_INTERNAL         232             /* 定时器内部错误 */

/*
 * 错误码转字符串
 */
const char* OSAL_StatusToString(osal_status_t status);

#endif /* OSAL_ERROR_H */
