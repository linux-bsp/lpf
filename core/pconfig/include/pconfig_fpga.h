/************************************************************************
 * PCL - FPGA外设配置
 *
 * 功能：
 * - FPGA外设配置类型定义
 * - 支持多种通信接口（SPI、I2C、PCIe等）
 *
 * 注意：
 * - 当前为简化版本，仅包含基本配置
 * - 如需使用 PDL 层 FPGA 驱动，请参考 MCU/BMC 的设计模式
 ************************************************************************/

#ifndef PCONFIG_FPGA_H
#define PCONFIG_FPGA_H

#include "pconfig_common.h"

/**
 * @brief FPGA外设配置
 */
typedef struct {
    /* 外设基本信息 */
    const char *name;                    /* 外设名称 */
    const char *description;             /* 描述信息 */
    bool enabled;                        /* 是否启用 */

    /* FPGA特定配置 */
    const char *device;                  /* 设备路径 */
    uint32_t cmd_timeout_ms;             /* 命令超时时间 */
    uint32_t retry_count;                /* 重试次数 */
} pconfig_fpga_cfg_t;

#endif /* PCONFIG_FPGA_H */
