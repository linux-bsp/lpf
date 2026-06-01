/************************************************************************
 * PCL BMC外设配置
 *
 * 功能：
 * - BMC外设配置容器
 * - 直接使用 types 模块的 pdl_bmc_config_t 结构体
 * - PCL 只负责配置的存储、查询和管理
 *
 * 设计原则：
 * - 配置结构体由 types 模块定义（pdl_types.h）
 * - PCL 层只添加配置管理字段（name, description, enabled）
 * - 避免重复定义，零拷贝传递配置
 * - 打破 PCL 和 PDL 之间的循环依赖
 ************************************************************************/

#ifndef PCONFIG_BMC_H
#define PCONFIG_BMC_H

#include "pdl_types.h"  /* 使用 PDL 模块的配置类型定义 */
#include "pconfig_common.h"

/*===========================================================================
 * BMC外设配置条目（PCL层配置容器）
 *===========================================================================*/

/**
 * @brief BMC外设配置条目
 *
 * PCL 层只负责配置管理，实际配置结构由 types 模块定义
 */
typedef struct {
    /* PCL 配置管理字段 */
    const char *name;             /* BMC名称（用于查询，如"payload_bmc"） */
    const char *description;      /* 描述信息 */
    bool        enabled;          /* 是否启用此BMC */

    /* PDL 配置（来自 types 模块） */
    pdl_bmc_config_t config;      /* BMC配置（来自 pdl_types.h） */

    /* GPIO控制（可选，PCL层扩展） */
    pconfig_gpio_config_t *power_gpio;  /* 电源控制GPIO */
    pconfig_gpio_config_t *reset_gpio;  /* 复位GPIO */
} pconfig_bmc_entry_t;

#endif /* PCONFIG_BMC_H */
