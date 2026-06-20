// SPDX-License-Identifier: GPL-2.0

#include "lpf/lpf_peripheral.h"

#include "lpf/lpf_core.h"
#include "lpf/lpf_led_service.h"
#include "lpf/lpf_mcu_service.h"
#include "lpf_peripheral_internal.h"

static bool g_lpf_peripheral_runtime_ready;
static bool g_lpf_peripheral_services_ready;

static int32_t lpf_peripheral_register_services(void)
{
	int32_t ret;

#ifdef CONFIG_LPF_MCU_SERVICE
	ret = lpf_mcu_service_register();
	if (ret != OSAL_SUCCESS)
		return ret;
#endif

#ifdef CONFIG_LPF_LED_SERVICE
	ret = lpf_led_service_register();
	if (ret != OSAL_SUCCESS) {
#ifdef CONFIG_LPF_MCU_SERVICE
		lpf_mcu_service_unregister();
#endif
		return ret;
	}
#endif

	return OSAL_SUCCESS;
}

static void lpf_peripheral_unregister_services(void)
{
#ifdef CONFIG_LPF_LED_SERVICE
	lpf_led_service_unregister();
#endif
#ifdef CONFIG_LPF_MCU_SERVICE
	lpf_mcu_service_unregister();
#endif
}

static int32_t lpf_peripheral_services_init(void)
{
	int32_t ret;

	if (g_lpf_peripheral_services_ready)
		return OSAL_SUCCESS;

	ret = lpf_peripheral_register_services();
	if (ret != OSAL_SUCCESS)
		return ret;

	g_lpf_peripheral_services_ready = true;
	return OSAL_SUCCESS;
}

static void lpf_peripheral_services_exit(void)
{
	if (!g_lpf_peripheral_services_ready)
		return;

	lpf_device_unregister_all();
	lpf_peripheral_unregister_services();
	g_lpf_peripheral_services_ready = false;
}

int32_t lpf_peripheral_runtime_init(void)
{
	int32_t ret;

	if (g_lpf_peripheral_runtime_ready)
		return OSAL_SUCCESS;

	ret = lpf_core_init();
	if (ret != OSAL_SUCCESS)
		return ret;

	ret = lpf_peripheral_services_init();
	if (ret != OSAL_SUCCESS)
		return ret;

	ret = lpf_peripheral_probe_devices();
	if (ret != OSAL_SUCCESS) {
		lpf_peripheral_services_exit();
		return ret;
	}

	g_lpf_peripheral_runtime_ready = true;
	return OSAL_SUCCESS;
}

void lpf_peripheral_runtime_exit(void)
{
	if (!g_lpf_peripheral_runtime_ready)
		return;

	lpf_peripheral_services_exit();
	g_lpf_peripheral_runtime_ready = false;
}
