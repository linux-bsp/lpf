/************************************************************************
 * OSAL POSIX实现 - 信号量
 ************************************************************************/

#include "osal.h"
#include <semaphore.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <limits.h>

struct osal_semaphore_s
{
    sem_t sem;
};

int32_t OSAL_SemaphoreCreate(osal_semaphore_t **sem, uint32_t initial_value)
{
    if (NULL == sem)
        return OSAL_ERR_INVALID_POINTER;

    if (initial_value > (uint32_t)INT32_MAX)
        return OSAL_ERR_INVALID_SEM_VALUE;

    osal_semaphore_t *new_sem = (osal_semaphore_t *)malloc(sizeof(osal_semaphore_t));
    if (NULL == new_sem)
        return OSAL_ERR_GENERIC;

    if (0 != sem_init(&new_sem->sem, 0, initial_value))
    {
        free(new_sem);
        return OSAL_ERR_GENERIC;
    }

    *sem = new_sem;
    return OSAL_SUCCESS;
}

int32_t OSAL_SemaphoreDelete(osal_semaphore_t *sem)
{
    if (NULL == sem)
        return OSAL_ERR_INVALID_POINTER;

    sem_destroy(&sem->sem);
    free(sem);
    return OSAL_SUCCESS;
}

int32_t OSAL_SemaphoreWait(osal_semaphore_t *sem)
{
    if (NULL == sem)
        return OSAL_ERR_INVALID_POINTER;

    if (0 != sem_wait(&sem->sem))
        return OSAL_ERR_GENERIC;

    return OSAL_SUCCESS;
}

int32_t OSAL_SemaphoreTimedWait(osal_semaphore_t *sem, uint32_t timeout_ms)
{
    if (NULL == sem)
        return OSAL_ERR_INVALID_POINTER;

    if (0 == timeout_ms)
    {
        /* 非阻塞模式 */
        if (0 != sem_trywait(&sem->sem))
        {
            if (EAGAIN == errno || EWOULDBLOCK == errno)
                return OSAL_ERR_TIMEOUT;
            return OSAL_ERR_GENERIC;
        }
        return OSAL_SUCCESS;
    }

    /* 计算超时时间点 */
    struct timespec ts;
    if (0 != clock_gettime(CLOCK_REALTIME, &ts))
        return OSAL_ERR_GENERIC;

    ts.tv_sec += timeout_ms / 1000;
    ts.tv_nsec += (timeout_ms % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000)
    {
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000;
    }

    if (0 != sem_timedwait(&sem->sem, &ts))
    {
        if (ETIMEDOUT == errno)
            return OSAL_ERR_TIMEOUT;
        return OSAL_ERR_GENERIC;
    }

    return OSAL_SUCCESS;
}

int32_t OSAL_SemaphorePost(osal_semaphore_t *sem)
{
    if (NULL == sem)
        return OSAL_ERR_INVALID_POINTER;

    if (0 != sem_post(&sem->sem))
        return OSAL_ERR_GENERIC;

    return OSAL_SUCCESS;
}
