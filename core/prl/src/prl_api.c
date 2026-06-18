/**
 * @file prl_api.c
 * @brief Protocol Layer Public API Implementation
 * @details PRL 协议层对外 API 实现
 */

#include "osal.h"

#include "prl.h"

/* 全局初始化标志 */
static bool g_prl_initialized = false;

/* Secondary API functions */

bool PRL_IsDeviceTypeValid(uint8_t dev_type)
{
    return prl_device_type_valid(dev_type);
}

const char *PRL_GetDeviceTypeName(uint8_t dev_type)
{
    return prl_device_type_name(dev_type);
}

const char *PRL_GetErrorString(int error_code)
{
    /* 现在使用 OSAL 错误码，直接返回 OSAL 错误描述 */
    return OSAL_get_status_name(error_code);
}

void PRL_get_version(uint8_t *major, uint8_t *minor)
{
    if (major) {
        *major = PRL_VERSION_MAJOR;
    }
    if (minor) {
        *minor = PRL_VERSION_MINOR;
    }
}

void PRL_reset_sequence(uint32_t seq)
{
    extern osal_atomic_uint32_t g_seq_number; /* 定义在 prl_common.c */
    OSAL_atomic_store(&g_seq_number, seq);
}

uint32_t PRL_GetCurrentSequence(void)
{
    extern osal_atomic_uint32_t g_seq_number;
    return OSAL_atomic_load(&g_seq_number);
}

/* Initialization and cleanup functions */

int PRL_init(void)
{
    if (g_prl_initialized) {
        return OSAL_SUCCESS;
    }

    /* 初始化序列号（可选） */
    /* 这里可以从持久化存储中恢复序列号 */

    g_prl_initialized = true;
    return OSAL_SUCCESS;
}

int PRL_deinit(void)
{
    if (!g_prl_initialized) {
        return OSAL_SUCCESS;
    }

    /* 清理资源（如果有） */

    g_prl_initialized = false;
    return OSAL_SUCCESS;
}
