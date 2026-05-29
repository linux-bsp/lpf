/************************************************************************
 * PCL BMC外设配置
 *
 * 功能：
 * - BMC外设配置容器
 * - 直接使用 PDL 层的 pdl_bmc_config_t 结构体
 * - PCL 只负责配置的存储、查询和管理
 *
 * 设计原则：
 * - 配置结构体由 PDL 层定义（pdl_bmc.h）
 * - PCL 层只添加配置管理字段（name, description, enabled）
 * - 避免重复定义，零拷贝传递配置
 ************************************************************************/

#ifndef PCL_BMC_H
#define PCL_BMC_H

#include "pdl_bmc.h"  /* 直接使用 PDL 层配置 */
#include "pcl_common.h"

/*===========================================================================
 * BMC外设配置条目（PCL层配置容器）
 *===========================================================================*/

/**
 * @brief BMC外设配置条目
 *
 * PCL 层只负责配置管理，实际配置结构由 PDL 层定义
 */
typedef struct {
    /* PCL 配置管理字段 */
    const char *name;             /* BMC名称（用于查询，如"payload_bmc"） */
    const char *description;      /* 描述信息 */
    bool        enabled;          /* 是否启用此BMC */

    /* PDL 配置（直接引用） */
    pdl_bmc_config_t config;      /* BMC配置（来自 pdl_bmc.h） */

    /* GPIO控制（可选，PCL层扩展） */
    pcl_gpio_config_t *power_gpio;  /* 电源控制GPIO */
    pcl_gpio_config_t *reset_gpio;  /* 复位GPIO */
} pcl_bmc_entry_t;

#endif /* PCL_BMC_H */
