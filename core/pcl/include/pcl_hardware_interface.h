/************************************************************************
 * PCL硬件接口定义
 *
 * 功能：
 * - 硬件通信接口类型枚举（CAN/UART/I2C/SPI/Ethernet/USB/SpaceWire/1553B）
 *
 * 说明：
 * - 本文件定义物理层硬件通信接口类型枚举
 * - 实际硬件配置结构由 HAL 层定义（hal_can.h, hal_serial.h 等）
 * - PDL 层直接嵌入 HAL 配置，不需要 PCL 层的配置结构
 *
 * 注意：
 * - 本文件中的配置结构体已废弃，仅保留接口类型枚举
 * - 请直接使用 HAL 层的配置结构体（hal_can_config_t, hal_serial_config_t 等）
 ************************************************************************/

#ifndef PCL_HARDWARE_INTERFACE_H
#define PCL_HARDWARE_INTERFACE_H

#include "osal_types.h"

/*===========================================================================
 * 硬件接口类型枚举
 *===========================================================================*/

/**
 * @brief 硬件接口类型枚举
 */
typedef enum {
    PCL_HW_INTERFACE_NONE = 0,
    PCL_HW_INTERFACE_CAN,
    PCL_HW_INTERFACE_UART,
    PCL_HW_INTERFACE_I2C,
    PCL_HW_INTERFACE_SPI,
    PCL_HW_INTERFACE_ETHERNET,
    PCL_HW_INTERFACE_USB,
    PCL_HW_INTERFACE_SPACEWIRE,
    PCL_HW_INTERFACE_1553B,
    PCL_HW_INTERFACE_MAX
} pcl_hw_interface_type_t;

/*===========================================================================
 * 注意：硬件接口配置结构已废弃
 *===========================================================================*/

/*
 * 以下配置结构已废弃，请直接使用 HAL 层的配置结构：
 *
 * - CAN:      使用 hal_can_config_t     (hal_can.h)
 * - UART:     使用 hal_serial_config_t  (hal_serial.h)
 * - I2C:      使用 hal_i2c_config_t     (hal_i2c.h)
 * - SPI:      使用 hal_spi_config_t     (hal_spi.h)
 * - GPIO:     使用 hal_gpio_config_t    (hal_gpio.h)
 *
 * PDL 层配置直接嵌入 HAL 配置，避免重复定义和转换开销。
 */

#endif /* PCL_HARDWARE_INTERFACE_H */
