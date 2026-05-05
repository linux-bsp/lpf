/************************************************************************
 * OSAL - POSIX线程封装实现
 ************************************************************************/

#include <osal.h>
#include <pthread.h>

int32_t OSAL_pthread_create(osal_thread_t *thread,
                            void *attr,
                            osal_thread_func_t start_routine,
                            void *arg)
{
    pthread_t pt;

    union {
        void *osal_attr;
        pthread_attr_t *posix_attr;
    } attr_union;

    attr_union.osal_attr = attr;

    int32_t ret = pthread_create(&pt, attr_union.posix_attr, start_routine, arg);
    if (0 == ret && NULL != thread) {
        union {
            pthread_t posix_thread;
            osal_thread_t osal_thread;
        } thread_union;
        thread_union.posix_thread = pt;
        *thread = thread_union.osal_thread;
    }
    return ret;
}

int32_t OSAL_pthread_join(osal_thread_t thread, void **retval)
{
    union {
        osal_thread_t osal_thread;
        pthread_t posix_thread;
    } thread_union;

    thread_union.osal_thread = thread;
    return pthread_join(thread_union.posix_thread, retval);
}

int32_t OSAL_ThreadCreate(osal_thread_t *thread, osal_thread_func_t func, void *arg)
{
    if (NULL == thread || NULL == func)
        return OSAL_ERR_INVALID_POINTER;

    int32_t ret = OSAL_pthread_create(thread, NULL, func, arg);
    if (0 != ret)
        return OSAL_ERR_GENERIC;

    return OSAL_SUCCESS;
}

int32_t OSAL_ThreadJoin(osal_thread_t thread)
{
    int32_t ret = OSAL_pthread_join(thread, NULL);
    if (0 != ret)
        return OSAL_ERR_GENERIC;

    return OSAL_SUCCESS;
}
