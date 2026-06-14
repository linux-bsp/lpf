/************************************************************************
 * OSAL 读写锁实现（POSIX 薄封装）
 ************************************************************************/

#include "osal.h"
#include <pthread.h>
#include <errno.h>

int32_t OSAL_pthread_rwlock_init(osal_rwlock_t *rwlock, const osal_rwlockattr_t *attr)
{
    if (rwlock == NULL) {
        errno = EINVAL;
        return -1;
    }

    return pthread_rwlock_init(rwlock, attr);
}

int32_t OSAL_pthread_rwlock_destroy(osal_rwlock_t *rwlock)
{
    if (rwlock == NULL) {
        errno = EINVAL;
        return -1;
    }

    return pthread_rwlock_destroy(rwlock);
}

int32_t OSAL_pthread_rwlock_rdlock(osal_rwlock_t *rwlock)
{
    if (rwlock == NULL) {
        errno = EINVAL;
        return -1;
    }

    return pthread_rwlock_rdlock(rwlock);
}

int32_t OSAL_pthread_rwlock_wrlock(osal_rwlock_t *rwlock)
{
    if (rwlock == NULL) {
        errno = EINVAL;
        return -1;
    }

    return pthread_rwlock_wrlock(rwlock);
}

int32_t OSAL_pthread_rwlock_tryrdlock(osal_rwlock_t *rwlock)
{
    if (rwlock == NULL) {
        errno = EINVAL;
        return -1;
    }

    return pthread_rwlock_tryrdlock(rwlock);
}

int32_t OSAL_pthread_rwlock_trywrlock(osal_rwlock_t *rwlock)
{
    if (rwlock == NULL) {
        errno = EINVAL;
        return -1;
    }

    return pthread_rwlock_trywrlock(rwlock);
}

int32_t OSAL_pthread_rwlock_unlock(osal_rwlock_t *rwlock)
{
    if (rwlock == NULL) {
        errno = EINVAL;
        return -1;
    }

    return pthread_rwlock_unlock(rwlock);
}
