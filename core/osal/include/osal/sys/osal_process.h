/************************************************************************
 * OSAL - 进程管理接口
 *
 * 功能：
 * - 封装进程创建、等待、信号等操作
 * - 支持多进程架构
 * - 使用固定大小类型，避免平台相关类型
 *
 * 设计原则：
 * - 提供标准进程管理函数的封装
 * - 返回值统一使用OSAL错误码
 * - 便于RTOS移植（RTOS可能不支持进程）
 ************************************************************************/

#ifndef OSAL_PROCESS_H
#define OSAL_PROCESS_H

#include <sys/types.h>

/*===========================================================================
 * 进程类型定义
 *===========================================================================*/

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    /* POSIX 平台 */
    typedef pid_t osal_pid_t;      /* 进程ID */
    typedef uid_t osal_uid_t;      /* 用户ID */
    typedef gid_t osal_gid_t;      /* 组ID */
#else
    /* 其他平台（RTOS 等）- 需要提供对应的类型定义 */
    #error "Unsupported platform - please define process types for your platform"
#endif

/*===========================================================================
 * 进程管理接口
 *===========================================================================*/
/* 基础进程控制函数（保持兼容） */
void OSAL_exit(int32_t status);
osal_pid_t OSAL_getpid(void);
osal_pid_t OSAL_getppid(void);
int32_t OSAL_kill(osal_pid_t pid, int32_t sig);
void OSAL_abort(void);
osal_pid_t OSAL_fork(void);
int32_t OSAL_execvp(const char *file, char *const argv[]);
int32_t OSAL_waitpid(osal_pid_t pid, int32_t *status, int32_t options);

/* 等待选项 */
#define OSAL_WNOHANG  0x01  /* 非阻塞等待 */

#endif /* OSAL_PROCESS_H */
