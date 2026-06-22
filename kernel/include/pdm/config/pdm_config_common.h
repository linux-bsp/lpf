/************************************************************************
 * PDM_CONFIG 通用类型定义
 *
 * 功能：
 * - GPIO配置（PDM_CONFIG 层扩展功能）
 *
 * 说明：
 * - 本文件包含 PDM_CONFIG 层特有的扩展配置类型
 * - 硬件接口类型由 PDM_CONFIG 层定义，外设服务按需消费
 ************************************************************************/

#ifndef PDM_CONFIG_COMMON_H
#define PDM_CONFIG_COMMON_H

#include "osal.h"

/*===========================================================================
 * Device list
 *===========================================================================*/

typedef enum {
	PDM_CONFIG_DEVICE_TYPE_INVALID = 0x00,
	PDM_CONFIG_DEVICE_TYPE_MCU = 0x01,
	PDM_CONFIG_DEVICE_TYPE_LED = 0x02,
} pdm_config_device_type_t;

typedef enum {
	PDM_CONFIG_NODE_STATUS_DISABLED = 0,
	PDM_CONFIG_NODE_STATUS_OKAY = 1,
} pdm_config_node_status_t;

typedef struct {
	pdm_config_device_type_t device_type;
	uint32_t index;
	const char *name;
	const char *compatible;
	pdm_config_node_status_t status;
	const void *payload;
	const void *entry;
	uint32_t payload_size;
} pdm_config_device_node_t;

typedef pdm_config_device_node_t pdm_config_device_config_t;

/*===========================================================================
 * GPIO配置（PDM_CONFIG 层扩展）
 *===========================================================================*/

/**
 * @brief GPIO配置
 *
 * 说明：
 * - 用于 PDM_CONFIG 层的设备扩展（复位 GPIO、中断 GPIO 等）
 * - 实际 GPIO 操作通过 PDM HW 层实现
 */
typedef struct {
	uint32_t gpio_num; /* GPIO编号 */
	uint32_t pin_mux; /* 引脚复用配置 */
	bool active_low; /* 低电平有效 */
	bool pull_up; /* 上拉使能 */
	bool pull_down; /* 下拉使能 */
} pdm_config_gpio_config_t;

#endif /* PDM_CONFIG_COMMON_H */
