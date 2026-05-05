/************************************************************************
 * OSAL 错误码实现
 ************************************************************************/

#include <osal.h>
#include <string.h>

const char* OSAL_StatusToString(osal_status_t status)
{
    /* 成功 */
    if (status == OSAL_SUCCESS) {
        return "Success";
    }

    /* 通用错误码 - 映射到 errno */
    switch (status) {
        case EFAULT:
            return "Invalid pointer / Bad address";
        case ENOMEM:
            return "Out of memory";
        case EINVAL:
            return "Invalid argument / Invalid size / Invalid ID";
        case ENAMETOOLONG:
            return "Name too long";
        case EEXIST:
            return "Name already taken / Already exists";
        case ENOENT:
            return "Name not found";
        case ETIMEDOUT:
            return "Timeout";
        case ENOSYS:
            return "Not implemented";
        case EBUSY:
            return "Resource busy";
        case EPERM:
            return "Permission denied";
        case ENOTSUP:
            return "Operation not supported";
        case EWOULDBLOCK:
            return "Operation would block";
        case EINTR:
            return "Operation interrupted";
        case EMFILE:
            return "Resource limit exceeded";
        case EIO:
            return "I/O error / Generic error";
        /* EAGAIN 和 EWOULDBLOCK 在某些系统上值相同，只保留一个 */

        /* OSAL 特定错误码 (200+) */
        case OSAL_ERR_ADDRESS_MISALIGNED:
            return "Address misaligned";
        case OSAL_ERR_INVALID_INT_NUM:
            return "Invalid interrupt number";
        case OSAL_ERR_INVALID_PRIORITY:
            return "Invalid priority";
        case OSAL_ERR_INVALID_STATE:
            return "Invalid state";
        case OSAL_ERR_NO_FREE_IDS:
            return "No free IDs available";

        /* 信号量错误 */
        case OSAL_ERR_SEM_FAILURE:
            return "Semaphore operation failed";
        case OSAL_ERR_SEM_NOT_FULL:
            return "Semaphore not full";
        case OSAL_ERR_INVALID_SEM_VALUE:
            return "Invalid semaphore value";

        /* 队列错误 */
        case OSAL_ERR_QUEUE_EMPTY:
            return "Queue is empty";
        case OSAL_ERR_QUEUE_FULL:
            return "Queue is full";
        case OSAL_ERR_QUEUE_ID:
            return "Invalid queue ID";

        /* 定时器错误 */
        case OSAL_ERR_TIMER_INVALID_ARGS:
            return "Invalid timer arguments";
        case OSAL_ERR_TIMER_ID:
            return "Invalid timer ID";
        case OSAL_ERR_TIMER_INTERNAL:
            return "Timer internal error";

        default:
            /* 对于未明确处理的 errno 值，使用系统的 strerror */
            if (status > 0 && status < 200) {
                return strerror(status);
            }
            return "Unknown error";
    }
}
