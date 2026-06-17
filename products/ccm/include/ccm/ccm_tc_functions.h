/**
 * @file ccm_tc_functions.h
 * @brief CCM 遥控功能定义
 * @note 产品层：定义 CCM 项目的遥控功能枚举
 *       从 core/aconfig/aconfig_tc.h 移动到产品层
 */

#ifndef CCM_TC_FUNCTIONS_H
#define CCM_TC_FUNCTIONS_H

#include <stdint.h>

/**
 * @brief CCM 遥控功能枚举
 * @note 按功能类别分组，预留足够 ID 空间支持扩展
 */
typedef enum {
    /* 电源控制类 (0-99) */
    CCM_TC_POWER_ON = 0x0,
    CCM_TC_POWER_OFF = 0x1,
    CCM_TC_POWER_RESET = 0x2,
    CCM_TC_POWER_CYCLE = 0x3,

    /* 复位控制类 (100-199) */
    CCM_TC_SOFT_RESET = 0x64,
    CCM_TC_HARD_RESET = 0x65,
    CCM_TC_MCU_RESET = 0x66,
    CCM_TC_MCU_POWER_CTRL = 0x67,
    CCM_TC_FPGA_RESET = 0x68,
    CCM_TC_FPGA_CONFIG_LOAD = 0x69,

    /* 固件升级类 (200-299) */
    CCM_TC_FIRMWARE_UPGRADE_START = 0xC8,
    CCM_TC_FIRMWARE_UPGRADE_DATA = 0xC9,
    CCM_TC_FIRMWARE_UPGRADE_VERIFY = 0xCA,
    CCM_TC_FIRMWARE_UPGRADE_COMMIT = 0xCB,

    /* 系统控制类 (300-399) */
    CCM_TC_SYSTEM_RESET = 0x12C,

    /* 预留扩展空间 */
    CCM_TC_FUNC_MAX = 0x3E8
} ccm_tc_function_t;

/* 兼容性别名（逐步迁移） */
#define ACONFIG_TC_POWER_ON                CCM_TC_POWER_ON
#define ACONFIG_TC_POWER_OFF               CCM_TC_POWER_OFF
#define ACONFIG_TC_POWER_RESET             CCM_TC_POWER_RESET
#define ACONFIG_TC_POWER_CYCLE             CCM_TC_POWER_CYCLE
#define ACONFIG_TC_SOFT_RESET              CCM_TC_SOFT_RESET
#define ACONFIG_TC_HARD_RESET              CCM_TC_HARD_RESET
#define ACONFIG_TC_MCU_RESET               CCM_TC_MCU_RESET
#define ACONFIG_TC_MCU_POWER_CTRL          CCM_TC_MCU_POWER_CTRL
#define ACONFIG_TC_FPGA_RESET              CCM_TC_FPGA_RESET
#define ACONFIG_TC_FPGA_CONFIG_LOAD        CCM_TC_FPGA_CONFIG_LOAD
#define ACONFIG_TC_FIRMWARE_UPGRADE_START  CCM_TC_FIRMWARE_UPGRADE_START
#define ACONFIG_TC_FIRMWARE_UPGRADE_DATA   CCM_TC_FIRMWARE_UPGRADE_DATA
#define ACONFIG_TC_FIRMWARE_UPGRADE_VERIFY CCM_TC_FIRMWARE_UPGRADE_VERIFY
#define ACONFIG_TC_FIRMWARE_UPGRADE_COMMIT CCM_TC_FIRMWARE_UPGRADE_COMMIT
#define ACONFIG_TC_SYSTEM_RESET            CCM_TC_SYSTEM_RESET
#define ACONFIG_TC_FUNC_MAX                CCM_TC_FUNC_MAX

#endif /* CCM_TC_FUNCTIONS_H */
