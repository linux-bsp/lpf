/************************************************************************
 * OSAL POSIX实现 - 时间延迟操作
 *
 * 修复：统一错误码返回，符合 OSAL 规范
 ************************************************************************/

#define _DEFAULT_SOURCE  /* 启用usleep等函数 */
#include "sys/osal_time.h"
#include <unistd.h>
#include <time.h>
#include <errno.h>

int32_t OSAL_msleep(uint32_t msec)
{
    if (usleep(msec * OSAL_USEC_PER_MSEC) == 0)
        return OSAL_SUCCESS;
    return OSAL_ERR_GENERIC;
}

int32_t OSAL_usleep(uint32_t usec)
{
    if (usleep(usec) == 0)
        return OSAL_SUCCESS;
    return OSAL_ERR_GENERIC;
}

int32_t OSAL_sleep(uint32_t sec)
{
    /* sleep() 返回剩余未睡眠的秒数，0 表示成功 */
    if (sleep(sec) == 0)
        return OSAL_SUCCESS;
    return OSAL_ERR_GENERIC;
}

int32_t OSAL_nanosleep(uint64_t nsec)
{
    struct timespec req;
    req.tv_sec = nsec / OSAL_NSEC_PER_SEC;
    req.tv_nsec = nsec % OSAL_NSEC_PER_SEC;

    if (nanosleep(&req, NULL) == 0)
        return OSAL_SUCCESS;
    return OSAL_ERR_GENERIC;
}
