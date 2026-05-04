/************************************************************************
 * OSAL - POSIX线程封装
 *
 * 功能：
 * - 封装pthread基本操作
 * - 1:1映射系统调用，不引入业务逻辑
 * - 使用固定大小类型，避免平台相关类型
 *
 * 设计原则：
 * - 提供标准pthread函数的封装
 * - 返回值统一为OSAL错误码
 * - 便于RTOS移植
 ************************************************************************/

#ifndef OSAL_THREAD_H
#define OSAL_THREAD_H

#include "osal_types.h"

/*
 * 线程类型（平台无关）
 */
typedef uint64_t osal_thread_t;

/*
 * 线程函数类型
 */
typedef void *(*osal_thread_func_t)(void *arg);

/**
 * @brief 创建线程
 *
 * @param[out] thread    线程句柄
 * @param[in]  attr      线程属性（NULL表示默认）
 * @param[in]  start_routine 线程函数
 * @param[in]  arg       传递给线程函数的参数
 *
 * @return 0 成功
 * @return 非0 错误码
 */
int32_t OSAL_pthread_create(osal_thread_t *thread,
                            void *attr,
                            osal_thread_func_t start_routine,
                            void *arg);

/**
 * @brief 等待线程结束
 *
 * @param[in]  thread   线程句柄
 * @param[out] retval   线程返回值（NULL表示不关心）
 *
 * @return 0 成功
 * @return 非0 错误码
 */
int32_t OSAL_pthread_join(osal_thread_t thread, void **retval);

/**
 * @brief 创建线程（简化接口）
 *
 * @param[out] thread 线程句柄
 * @param[in]  func   线程函数
 * @param[in]  arg    线程参数
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_INVALID_POINTER thread或func为NULL
 * @return OSAL_ERR_GENERIC 创建失败
 */
int32_t OSAL_ThreadCreate(osal_thread_t *thread, osal_thread_func_t func, void *arg);

/**
 * @brief 等待线程结束（简化接口）
 *
 * @param[in] thread 线程句柄
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_GENERIC 失败
 */
int32_t OSAL_ThreadJoin(osal_thread_t thread);

#endif /* OSAL_THREAD_H */
