/**
 * @file aconfig_types.h
 * @brief ACONFIG 核心类型定义 - 优化版
 * @note 本文件定义 ACONFIG 模块的核心数据类型
 *       优化要点：
 *       1. 使用设备索引引用替代字符串查找
 *       2. 采用稀疏数组存储配置
 *       3. 失效映射内嵌到 TC 配置中
 */

#ifndef ACONFIG_TYPES_H
#define ACONFIG_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/* 条件包含 PDL HWID 类型（如果 CONFIG_PDL 启用） */
#ifdef CONFIG_PDL
#include "pdl_misc.h"
#else
/* PDL 未启用时的占位类型 */
typedef struct pdl_hwid {
    uint32_t product_id;
    uint32_t project_id;
    uint8_t  board_type;
    uint8_t  hw_revision;
} pdl_hwid_t;
#endif

/**
 * @brief 设备类型枚举（从 PConfig 复用）
 */
typedef enum {
    PCONFIG_DEVICE_MCU = 0,      /* 微控制器 */
    PCONFIG_DEVICE_BMC,           /* 板级管理控制器 */
    PCONFIG_DEVICE_FPGA,          /* 可编程逻辑器件 */
    PCONFIG_DEVICE_SWITCH,        /* 网络交换机 */
    PCONFIG_DEVICE_SENSOR,        /* 传感器 */
    PCONFIG_DEVICE_HOST,          /* 主机接口 */
    PCONFIG_DEVICE_SATELLITE,     /* 卫星设备 */
    PCONFIG_DEVICE_MAX
} pconfig_device_type_t;

/**
 * @brief 设备引用结构（统一索引方式）
 * @note 通过类型+索引的方式引用 PConfig 中的设备，实现 O(1) 查找
 */
typedef struct {
    pconfig_device_type_t type;  /* 设备类型: MCU/BMC/FPGA/... */
    uint32_t index;              /* 在 PConfig 对应类型表中的索引 */
} aconfig_device_ref_t;

/**
 * @brief 遥测数据新鲜度标记
 */
typedef enum {
    ACONFIG_TM_STATUS_INVALID = 0x00,  /* 无效（从未更新） */
    ACONFIG_TM_STATUS_FRESH = 0x01,    /* 新鲜（在有效期内） */
    ACONFIG_TM_STATUS_STALE = 0x02     /* 过期（超过有效期但可用） */
} aconfig_tm_status_t;

/**
 * @brief 遥控配置结构
 * @note 优化：
 *       - 使用设备索引替代字符串查找
 *       - 失效映射直接内嵌
 */
typedef struct {
    uint32_t function_id;                  /* 遥控功能 ID */
    aconfig_device_ref_t device;           /* 目标设备引用（索引方式） */

    /* 失效映射（内嵌） */
    const uint32_t *invalidated_tm_ids;    /* 受影响的遥测 ID 列表 */
    uint32_t invalidated_tm_count;         /* 受影响的遥测数量 */

    bool enabled;                          /* 是否启用 */
    void *user_context;                    /* 用户上下文 */
} aconfig_tc_config_t;

/**
 * @brief 遥测配置结构
 * @note 优化：使用设备索引替代字符串查找
 */
typedef struct {
    uint32_t function_id;                  /* 遥测功能 ID */
    aconfig_device_ref_t device;           /* 数据源设备引用（索引方式） */

    /* 遥测特有参数 */
    uint32_t poll_period_ms;               /* 采集周期（毫秒） */
    uint32_t validity_period_ms;           /* 有效期（毫秒） */

    bool enabled;                          /* 是否启用 */
    void *user_context;                    /* 用户上下文 */
} aconfig_tm_config_t;

/**
 * @brief 遥控配置条目（稀疏数组元素）
 * @note 优化：只存储实际定义的功能，不浪费内存
 */
typedef struct {
    uint32_t function_id;                  /* 功能 ID（用于查找） */
    aconfig_tc_config_t config;            /* 配置数据 */
} aconfig_tc_entry_t;

/**
 * @brief 遥测配置条目（稀疏数组元素）
 * @note 优化：只存储实际定义的功能，不浪费内存
 */
typedef struct {
    uint32_t function_id;                  /* 功能 ID（用于查找） */
    aconfig_tm_config_t config;            /* 配置数据 */
} aconfig_tm_entry_t;

/**
 * @brief ACONFIG 配置表（优化版）
 * @note 优化：采用稀疏数组 + 哈希表方式
 *       - 数组：只存储实际定义的功能
 *       - 哈希表：运行时构建，加速查找
 */
typedef struct {
    const char *name;                      /* 配置表名称 */

    /* 硬件 ID 支持 */
    uint32_t hwid_count;                   /* 支持的 HWID 数量，0 表示支持所有 HWID */
    const pdl_hwid_t *hwid_list;           /* 支持的 HWID 列表，NULL 表示支持所有 HWID */

    /* 遥控配置（稀疏数组） */
    const aconfig_tc_entry_t *tc_entries;  /* 遥控配置条目数组 */
    uint32_t tc_count;                     /* 实际配置数量（不是枚举最大值） */

    /* 遥测配置（稀疏数组） */
    const aconfig_tm_entry_t *tm_entries;  /* 遥测配置条目数组 */
    uint32_t tm_count;                     /* 实际配置数量（不是枚举最大值） */
} aconfig_config_table_t;

/**
 * @brief ACONFIG 统计信息
 */
typedef struct {
    uint32_t tc_enabled_count;             /* 启用的遥控功能数量 */
    uint32_t tc_disabled_count;            /* 禁用的遥控功能数量 */
    uint32_t tm_enabled_count;             /* 启用的遥测功能数量 */
    uint32_t tm_disabled_count;            /* 禁用的遥测功能数量 */
    uint32_t total_invalidation_maps;      /* 失效映射总数 */
} aconfig_statistics_t;

/* ========================================================================
 * 兼容性定义（逐步迁移）
 * ======================================================================== */

/**
 * @brief 旧版设备类型枚举（兼容性别名）
 */
typedef pconfig_device_type_t aconfig_device_type_t;

/* 设备类型兼容性宏 */
#define ACONFIG_DEVICE_MCU        PCONFIG_DEVICE_MCU
#define ACONFIG_DEVICE_BMC        PCONFIG_DEVICE_BMC
#define ACONFIG_DEVICE_FPGA       PCONFIG_DEVICE_FPGA
#define ACONFIG_DEVICE_SWITCH     PCONFIG_DEVICE_SWITCH
#define ACONFIG_DEVICE_SENSOR     PCONFIG_DEVICE_SENSOR
#define ACONFIG_DEVICE_HOST       PCONFIG_DEVICE_HOST
#define ACONFIG_DEVICE_SATELLITE  PCONFIG_DEVICE_SATELLITE
#define ACONFIG_DEVICE_MAX        PCONFIG_DEVICE_MAX

/**
 * @brief 旧版遥控配置结构（兼容性支持）
 * @deprecated 新代码应使用优化后的结构
 */
typedef struct {
    uint32_t function_id;
    aconfig_device_type_t device_type;  /* 兼容：设备类型枚举 */
    const char *device_name;            /* 兼容：设备名称字符串 */
    uint32_t data_validity_ms;          /* 兼容：数据有效期（遥测用） */
    uint32_t background_update_period_ms; /* 兼容：更新周期（遥测用） */
    bool enabled;
    void *user_context;
} aconfig_tc_config_legacy_t;

typedef aconfig_tc_config_legacy_t aconfig_tm_config_legacy_t;

/**
 * @brief 旧版失效映射结构（兼容性支持）
 * @deprecated 失效映射已内嵌到 TC 配置中
 */
typedef struct {
    uint32_t source_tm_id;
    uint32_t *affected_tm_ids;
    uint32_t affected_count;
} aconfig_invalidation_map_t;

/**
 * @brief 旧版配置表结构（兼容性支持）
 * @deprecated 新代码应使用优化后的结构
 */
typedef struct {
    const char *name;
    uint32_t hwid_count;
    const pdl_hwid_t *hwid_list;

    /* 旧版：密集数组 */
    const aconfig_tc_config_legacy_t *tc_table;
    uint32_t tc_count;
    const aconfig_tm_config_legacy_t *tm_table;
    uint32_t tm_count;
    const aconfig_invalidation_map_t *inv_map;
    uint32_t inv_count;
} aconfig_config_table_legacy_t;

#endif /* ACONFIG_TYPES_H */
