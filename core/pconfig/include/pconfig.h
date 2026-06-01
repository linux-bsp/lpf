/************************************************************************
 * PCL内部总头文件
 *
 * 功能：
 * - PCL内部使用的总头文件
 * - 包含所有公共头文件和外设头文件
 * - 对外只提供 pconfig_api.h
 *
 * 使用方式：
 *   PCL内部源文件：#include "pconfig.h"
 *   PDL层（对外）：  #include "pconfig_api.h"
 ************************************************************************/

#ifndef PCONFIG_H
#define PCONFIG_H

/* 公共头文件 */
#include "pconfig_common.h"
#include "pconfig_board.h"

/* 硬件外设头文件 */
#include "pconfig_hardware_interface.h"
#include "pconfig_mcu.h"
#include "pconfig_bmc.h"
#include "pconfig_fpga.h"
#include "pconfig_switch.h"

#endif /* PCONFIG_H */
