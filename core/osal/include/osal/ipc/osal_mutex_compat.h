/************************************************************************
 * OSAL Mutex 兼容层 - 临时过渡方案
 *
 * 提供旧 API 的兼容包装，使代码可以逐步迁移
 *
 * 使用方式：
 * 1. 临时包含此头文件以保持旧代码编译通过
 * 2. 逐步迁移到新的 pthread_mutex_t API
 * 3. 迁移完成后删除此文件
 ************************************************************************/

#ifndef OSAL_MUTEX_COMPAT_H
#define OSAL_MUTEX_COMPAT_H

#include <pthread.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
 * 兼容类型定义
 *===========================================================================*/

/* 旧的不透明指针 -> 新的直接类型 */
typedef pthread_mutex_t osal_mutex_t;

/* 旧的枚举类型 */
typedef enum {
    OSAL_MUTEX_NORMAL = PTHREAD_MUTEX_NORMAL,
    OSAL_MUTEX_RECURSIVE = PTHREAD_MUTEX_RECURSIVE
} osal_mutex_type_t;

typedef pthread_mutexattr_t osal_mutex_attr_t;

/*===========================================================================
 * 兼容 API（基于新的薄封装）
 *===========================================================================*/

/**
 * @brief 创建互斥锁（兼容旧 API）
 *
 * 注意：旧 API 使用指针的指针，新 API 直接使用栈上对象
 * 此兼容层需要调用者修改使用方式
 */
static inline int32_t OSAL_MutexCreate(osal_mutex_t **mutex)
{
    if (mutex == NULL) {
        return -1;
    }

    /* 警告：此兼容层不分配内存，调用者需要提供已分配的 mutex */
    /* 新代码应该直接使用栈上的 pthread_mutex_t */
    return OSAL_pthread_mutex_init(*mutex, NULL);
}

static inline int32_t OSAL_MutexDelete(osal_mutex_t *mutex)
{
    if (mutex == NULL) {
        return -1;
    }

    return OSAL_pthread_mutex_destroy(mutex);
}

static inline int32_t OSAL_MutexLock(osal_mutex_t *mutex)
{
    if (mutex == NULL) {
        return -1;
    }

    return OSAL_pthread_mutex_lock(mutex);
}

static inline int32_t OSAL_MutexUnlock(osal_mutex_t *mutex)
{
    if (mutex == NULL) {
        return -1;
    }

    return OSAL_pthread_mutex_unlock(mutex);
}

static inline int32_t OSAL_MutexTryLock(osal_mutex_t *mutex)
{
    if (mutex == NULL) {
        return -1;
    }

    return OSAL_pthread_mutex_trylock(mutex);
}

static inline int32_t OSAL_MutexTimedLock(osal_mutex_t *mutex, uint32_t timeout_ms)
{
    if (mutex == NULL) {
        return -1;
    }

    return OSAL_pthread_mutex_timedlock(mutex, timeout_ms);
}

/*===========================================================================
 * 属性管理兼容 API
 *===========================================================================*/

static inline int32_t OSAL_MutexAttrCreate(osal_mutex_attr_t **attr)
{
    if (attr == NULL) {
        return -1;
    }

    return OSAL_pthread_mutexattr_init(*attr);
}

static inline int32_t OSAL_MutexAttrDestroy(osal_mutex_attr_t *attr)
{
    if (attr == NULL) {
        return -1;
    }

    return OSAL_pthread_mutexattr_destroy(attr);
}

static inline int32_t OSAL_MutexAttrSetType(osal_mutex_attr_t *attr, osal_mutex_type_t type)
{
    if (attr == NULL) {
        return -1;
    }

    return OSAL_pthread_mutexattr_settype(attr, type);
}

static inline int32_t OSAL_MutexAttrGetType(const osal_mutex_attr_t *attr, osal_mutex_type_t *type)
{
    if (attr == NULL || type == NULL) {
        return -1;
    }

    return OSAL_pthread_mutexattr_gettype(attr, (int32_t *)type);
}

static inline int32_t OSAL_MutexCreateEx(osal_mutex_t **mutex, const osal_mutex_attr_t *attr)
{
    if (mutex == NULL) {
        return -1;
    }

    return OSAL_pthread_mutex_init(*mutex, attr);
}

#ifdef __cplusplus
}
#endif

#endif /* OSAL_MUTEX_COMPAT_H */
