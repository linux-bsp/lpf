/************************************************************************
 * EMS - Embedded Middleware System
 *
 * 轻量化操作系统抽象层
 *
 * 设计原则:
 * 1. 保留核心API设计
 * 2. 移除不常用的功能(文件系统、模块加载等)
 * 3. 简化实现，减少代码量
 * 4. 保持跨平台兼容性
 ************************************************************************/

#ifndef OSAL_H
#define OSAL_H

#include "../src/posix/osal_platform_internal.h"
#include "../src/posix/osal_types_internal.h"

/* IPC - 进程间通信 */
#include "../src/posix/ipc/osal_mutex_internal.h"
#include "../src/posix/ipc/osal_rwlock_internal.h"
#include "../src/posix/ipc/osal_semaphore_internal.h"
#include "../src/posix/ipc/osal_cond_internal.h"
#include "../src/posix/ipc/osal_shm_internal.h"
#include "../src/posix/ipc/osal_shm_cache_internal.h"
#include "../src/posix/ipc/osal_atomic_internal.h"

/* SYS - 系统调用封装 */
#include "../src/posix/sys/osal_clock_internal.h"
#include "../src/posix/sys/osal_signal_internal.h"
#include "../src/posix/sys/osal_file_internal.h"
#include "../src/posix/sys/osal_select_internal.h"
#include "../src/posix/sys/osal_env_internal.h"
#include "../src/posix/sys/osal_time_internal.h"
#include "../src/posix/sys/osal_process_internal.h"
#include "../src/posix/sys/osal_thread_internal.h"
#include "../src/posix/sys/osal_sched_internal.h"

/* NET - 网络相关 */
#include "../src/posix/net/osal_socket_internal.h"
#include "../src/posix/net/osal_termios_internal.h"

/* LIB - 标准库封装 */
#include "../src/posix/lib/osal_string_internal.h"
#include "../src/posix/lib/osal_stdio_internal.h"
#include "../src/posix/lib/osal_heap_internal.h"
#include "../src/posix/lib/osal_errno_internal.h"

/* UTIL - 工具类 */
#include "../src/posix/util/osal_log_internal.h"
#include "../src/posix/util/osal_version_internal.h"

/*
 * OSAL版本信息
 */
#define OSAL_LITE_VERSION_MAJOR  1
#define OSAL_LITE_VERSION_MINOR  0
#define OSAL_LITE_VERSION_PATCH  0

#endif /* OSAL_H */
