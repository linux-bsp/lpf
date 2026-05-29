/************************************************************************
 * PCL MCU外设配置
 *
 * 功能：
 * - MCU外设配置容器
 * - 直接使用 PDL 层的 pdl_mcu_config_t 结构体
 * - PCL 只负责配置的存储、查询和管理
 *
 * 设计原则：
 * - 配置结构体由 PDL 层定义（pdl_mcu.h）
 * - PCL 层只添加配置管理字段（name, description, enabled）
 * - 避免重复定义，零拷贝传递配置
 ************************************************************************/

#ifndef PCL_MCU_H
#define PCL_MCU_H

#include "pdl_mcu.h"  /* 直接使用 PDL 层配置 */
#include "pcl_common.h"

/*===========================================================================
 * MCU外设配置条目（PCL层配置容器）
 *===========================================================================*/

/**
 * @brief MCU外设配置条目
 *
 * PCL 层只负责配置管理，实际配置结构由 PDL 层定义
 */
typedef struct {
    /* PCL 配置管理字段 */
    const char *name;             /* MCU名称（用于查询，如"power_mcu"） */
    const char *description;      /* 描述信息 */
    bool        enabled;          /* 是否启用此MCU */

    /* PDL 配置（直接引用） */
    pdl_mcu_config_t config;      /* MCU配置（来自 pdl_mcu.h） */

    /* GPIO控制（可选，PCL层扩展） */
    pcl_gpio_config_t *reset_gpio;  /* 复位GPIO */
    pcl_gpio_config_t *irq_gpio;    /* 中断GPIO */
} pcl_mcu_entry_t;

#endif /* PCL_MCU_H */
