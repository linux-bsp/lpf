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
#include "osal/osal_mutex_api.h"
#include "osal/osal_rwlock_api.h"
#include "osal/osal_semaphore_api.h"
#include "../src/posix/ipc/osal_cond_internal.h"
#include "osal/osal_shm_api.h"
#include "../src/posix/ipc/osal_shm_cache_internal.h"
#include "osal/osal_atomic_api.h"

/* SYS - 系统调用封装 */
#include "../src/posix/sys/osal_clock_internal.h"
#include "../src/posix/sys/osal_signal_internal.h"
#include "osal/osal_file_api.h"
#include "../src/posix/sys/osal_poll_internal.h"
#include "../src/posix/sys/osal_select_internal.h"
#include "../src/posix/sys/osal_env_internal.h"
#include "osal/osal_time_api.h"
#include "../src/posix/sys/osal_process_internal.h"
#include "osal/osal_thread_api.h"
#include "../src/posix/sys/osal_sched_internal.h"

/* NET - 网络相关 */
#include "../src/posix/net/osal_socket_internal.h"
#include "../src/posix/net/osal_termios_internal.h"

/* LIB - 标准库封装 */
#include "osal/osal_string_api.h"
#include "../src/posix/lib/osal_stdio_internal.h"
#include "osal/osal_heap_api.h"
#include "osal/osal_errno_api.h"

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
