/************************************************************************
 * Kernel OSAL common types
 ************************************************************************/

#ifndef OSAL_TYPES_H
#define OSAL_TYPES_H

#include <linux/stddef.h>
#include <linux/types.h>

#include "osal_platform.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

#define OS_OBJECT_ID_UNDEFINED ((osal_id_t)0)
#define OSAL_PLATFORM_BITS OSAL_ARCH_BITS
#define OSAL_CACHE_LINE_SIZE 0x40

#ifdef CONFIG_OSAL_MAX_TASKS
#define OS_MAX_TASKS CONFIG_OSAL_MAX_TASKS
#else
#define OS_MAX_TASKS 0x40
#endif

#ifdef CONFIG_OSAL_MAX_QUEUES
#define OS_MAX_QUEUES CONFIG_OSAL_MAX_QUEUES
#else
#define OS_MAX_QUEUES 0x40
#endif

#ifdef CONFIG_OSAL_MAX_MUTEXES
#define OS_MAX_MUTEXES CONFIG_OSAL_MAX_MUTEXES
#else
#define OS_MAX_MUTEXES 0x40
#endif

#define OS_MAX_API_NAME 0x14
#define OS_PEND 0x0
#define OS_CHECK (-1)
#define OS_TASK_PRIORITY_MIN 0x1
#define OS_TASK_PRIORITY_MAX 0xFF

typedef uint32_t osal_id_t;
typedef size_t osal_size_t;
typedef ssize_t osal_ssize_t;
typedef uintptr_t osal_uintptr_t;
typedef intptr_t osal_intptr_t;
typedef ptrdiff_t osal_ptrdiff_t;
typedef int64_t osal_off_t;
typedef int64_t osal_time_t;
typedef int64_t osal_usec_t;
typedef int64_t osal_nsec_t;

#define OSAL_sizeof(x) ((osal_size_t)sizeof(x))
#define OSAL_ARRAY_SIZE(arr) (OSAL_sizeof(arr) / OSAL_sizeof((arr)[0]))

#endif /* OSAL_TYPES_H */
