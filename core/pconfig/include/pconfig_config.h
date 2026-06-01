/**
 * @file pconfig_config.h
 * @brief PCL配置总入口头文件
 * @note 包含所有配置相关的类型和定义
 */

#ifndef PCONFIG_CONFIG_H
#define PCONFIG_CONFIG_H

/* 基础类型 */
#include "pconfig_common.h"

/* 硬件接口 */
#include "pconfig_hardware_interface.h"

/* 外设配置 */
#include "pconfig_mcu.h"
#include "pconfig_bmc.h"
#include "pconfig_fpga.h"
#include "pconfig_switch.h"

/* 板级配置 */
#include "pconfig_board.h"

#endif /* PCONFIG_CONFIG_H */
