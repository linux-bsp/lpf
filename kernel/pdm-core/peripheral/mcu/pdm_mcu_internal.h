/************************************************************************
 * MCU外设驱动内部头文件
 *
 * 职责：
 * - 定义内部数据结构
 * - 声明内部接口（CAN/串口通信层）
 ************************************************************************/

#ifndef PDM_MCU_INTERNAL_H
#define PDM_MCU_INTERNAL_H

#include "osal.h"
#include "pdm/core/pdm_core.h"
#include "pdm/config/pdm_config.h"
#include "pdm/peripheral/mcu/pdm_mcu_service.h"
#include "pdm_mcu_transport.h"

#ifndef CONFIG_PDM_MCU_MAX_DEVICES
#define CONFIG_PDM_MCU_MAX_DEVICES 4
#endif

#define PDM_MCU_MAX_DEVICES CONFIG_PDM_MCU_MAX_DEVICES

typedef struct {
	bool present;
	char name[64];
	pdm_config_mcu_interface_t interface;
	uint32_t cmd_timeout_ms;
} pdm_mcu_debug_info_t;

int32_t pdm_mcu_probe(const pdm_device_t *device);
void pdm_mcu_remove(const pdm_device_t *device);
pdm_mcu_handle_t pdm_mcu_get(uint32_t index);
int32_t pdm_mcu_debug_get(uint32_t index, pdm_mcu_debug_info_t *info);

int pdm_mcu_chrdev_register(void);
void pdm_mcu_chrdev_unregister(void);
int pdm_mcu_chrdev_register_device(const pdm_device_t *device);
void pdm_mcu_chrdev_unregister_device(const pdm_device_t *device);
void pdm_mcu_chrdev_record_error(uint32_t index, int error);
void pdm_mcu_chrdev_record_recovery(uint32_t index);
void pdm_mcu_chrdev_record_status(uint32_t index,
				  const pdm_mcu_status_t *status);
int pdm_mcu_proc_register(void);
void pdm_mcu_proc_unregister(void);
int pdm_mcu_debugfs_register(void);
void pdm_mcu_debugfs_unregister(void);

#endif /* PDM_MCU_INTERNAL_H */
