/************************************************************************
 * OSAL 互斥锁接口
 *
 * 简单封装 pthread_mutex，提供跨平台互斥锁支持
 ************************************************************************/

#ifndef OSAL_MUTEX_H
#define OSAL_MUTEX_H

#include "osal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 互斥锁句柄类型
 *
 * 不透明句柄，内部实现为 pthread_mutex_t*
 */
typedef struct osal_mutex_s osal_mutex_t;

/**
 * @brief 互斥锁类型
 */
typedef enum {
    OSAL_MUTEX_NORMAL = 0,      /* 普通互斥锁 */
    OSAL_MUTEX_RECURSIVE = 1,   /* 递归锁（可重入） */
    OSAL_MUTEX_ERRORCHECK = 2   /* 错误检查锁 */
} osal_mutex_type_t;

/**
 * @brief 互斥锁优先级协议
 */
typedef enum {
    OSAL_MUTEX_PRIO_NONE = 0,     /* 无优先级协议 */
    OSAL_MUTEX_PRIO_INHERIT = 1,  /* 优先级继承（防止优先级反转） */
    OSAL_MUTEX_PRIO_PROTECT = 2   /* 优先级天花板 */
} osal_mutex_protocol_t;

/**
 * @brief 互斥锁属性
 */
typedef struct {
    osal_mutex_type_t type;           /* 互斥锁类型 */
    osal_mutex_protocol_t protocol;   /* 优先级协议 */
    int32_t prio_ceiling;             /* 优先级天花板值（仅PRIO_PROTECT时使用） */
} osal_mutex_attr_t;

/**
 * @brief 创建互斥锁（默认属性：NORMAL + PRIO_INHERIT）
 *
 * @param[out] mutex 互斥锁句柄指针
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_POINTER mutex 为 NULL
 * @return OSAL_ERR_GENERIC 系统调用失败
 */
int32_t OSAL_MutexCreate(osal_mutex_t **mutex);

/**
 * @brief 创建互斥锁（自定义属性）
 *
 * @param[out] mutex 互斥锁句柄指针
 * @param[in] attr 互斥锁属性
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_POINTER mutex 为 NULL
 * @return OSAL_ERR_GENERIC 系统调用失败
 *
 * @note 使用优先级继承可以防止优先级反转问题
 */
int32_t OSAL_MutexCreateEx(osal_mutex_t **mutex, const osal_mutex_attr_t *attr);

/**
 * @brief 销毁互斥锁
 *
 * @param[in] mutex 互斥锁句柄
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_POINTER mutex 为 NULL
 * @return OSAL_ERR_GENERIC 系统调用失败
 */
int32_t OSAL_MutexDelete(osal_mutex_t *mutex);

/**
 * @brief 获取互斥锁
 *
 * @param[in] mutex 互斥锁句柄
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_POINTER mutex 为 NULL
 * @return OSAL_ERR_GENERIC 系统调用失败
 */
int32_t OSAL_MutexLock(osal_mutex_t *mutex);

/**
 * @brief 释放互斥锁
 *
 * @param[in] mutex 互斥锁句柄
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_POINTER mutex 为 NULL
 * @return OSAL_ERR_GENERIC 系统调用失败
 */
int32_t OSAL_MutexUnlock(osal_mutex_t *mutex);

/**
 * @brief 尝试获取互斥锁（非阻塞）
 *
 * @param[in] mutex 互斥锁句柄
 * @return OSAL_SUCCESS 成功获取锁
 * @return OSAL_ERR_INVALID_POINTER mutex 为 NULL
 * @return OSAL_ERR_BUSY 锁已被占用
 * @return OSAL_ERR_GENERIC 系统调用失败
 *
 * @note 对应pthread_mutex_trylock，不会阻塞
 */
int32_t OSAL_MutexTryLock(osal_mutex_t *mutex);

/**
 * @brief 带超时的获取互斥锁
 *
 * @param[in] mutex 互斥锁句柄
 * @param[in] timeout_ms 超时时间（毫秒）
 * @return OSAL_SUCCESS 成功获取锁
 * @return OSAL_ERR_INVALID_POINTER mutex 为 NULL
 * @return OSAL_ERR_TIMEOUT 超时
 * @return OSAL_ERR_GENERIC 系统调用失败
 *
 * @note 对应pthread_mutex_timedlock
 */
int32_t OSAL_MutexTimedLock(osal_mutex_t *mutex, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* OSAL_MUTEX_H */
