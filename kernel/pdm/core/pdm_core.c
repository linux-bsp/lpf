// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>

#include "../bus/pdm_bus_controller.h"
#include "pdm_ctl.h"
#include "../peripheral/pdm_peripheral.h"
#include "../mock/pdm_mock_devices.h"
#include "pdm/core/pdm_client.h"
#include "pdm/core/pdm_bus.h"
#include "pdm/pdm_errno.h"

/**
 * @brief PDM Core 模块初始化
 */
static int __init pdm_core_module_init(void)
{
	int ret;

	LOG_INFO("PDM_CORE", "Initializing PDM Core with Linux bus_type");

	/* 初始化 Linux bus_type */
	ret = pdm_bus_init();
	if (ret) {
		LOG_ERROR("PDM_CORE", "Failed to initialize PDM bus: %d", ret);
		return ret;
	}

	ret = pdm_ctl_init();
	if (ret) {
		LOG_ERROR("PDM_CORE", "Failed to initialize PDM control node: %d", ret);
		pdm_bus_exit();
		return ret;
	}

	ret = pdm_client_init();
	if (ret) {
		LOG_ERROR("PDM_CORE", "Failed to initialize PDM client nodes: %d", ret);
		pdm_ctl_exit();
		pdm_bus_exit();
		return ret;
	}

	ret = pdm_mcu_driver_init();
	if (ret) {
		LOG_ERROR("PDM_CORE", "Failed to register MCU driver: %d", ret);
		pdm_client_exit();
		pdm_ctl_exit();
		pdm_bus_exit();
		return ret;
	}

	ret = pdm_led_driver_init();
	if (ret) {
		LOG_ERROR("PDM_CORE", "Failed to register LED driver: %d", ret);
		pdm_mcu_driver_exit();
		pdm_client_exit();
		pdm_ctl_exit();
		pdm_bus_exit();
		return ret;
	}

	ret = pdm_mock_devices_init();
	if (ret) {
		LOG_ERROR("PDM_CORE", "Failed to register mock PDM devices: %d", ret);
		pdm_led_driver_exit();
		pdm_mcu_driver_exit();
		pdm_client_exit();
		pdm_ctl_exit();
		pdm_bus_exit();
		return ret;
	}

	/* 注册 PDM bus controller platform driver */
	ret = pdm_bus_controller_init();
	if (ret) {
		LOG_ERROR("PDM_CORE", "Failed to register bus controller: %d", ret);
		pdm_mock_devices_exit();
		pdm_led_driver_exit();
		pdm_mcu_driver_exit();
		pdm_client_exit();
		pdm_ctl_exit();
		pdm_bus_exit();
		return ret;
	}

	LOG_INFO("PDM_CORE", "PDM Core initialized successfully");
	return 0;
}

/**
 * @brief PDM Core 模块退出
 */
static void __exit pdm_core_module_exit(void)
{
	pdm_bus_controller_exit();
	pdm_mock_devices_exit();
	pdm_led_driver_exit();
	pdm_mcu_driver_exit();
	pdm_client_exit();
	pdm_ctl_exit();
	pdm_bus_exit();

	LOG_INFO("PDM_CORE", "PDM Core exited");
}

module_init(pdm_core_module_init);
module_exit(pdm_core_module_exit);

MODULE_AUTHOR("PDM");
MODULE_DESCRIPTION("PDM core device model with Linux bus_type");
MODULE_LICENSE("GPL");
MODULE_SOFTDEP("pre: osal can can_raw");
