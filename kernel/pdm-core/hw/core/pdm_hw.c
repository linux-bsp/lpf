// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>

#include "osal.h"
#include "pdm/hw/pdm_hw.h"
#include "pdm_hw_internal.h"
static bool g_lpf_hw_builtin_drivers_ready;

static const pdm_hw_builtin_driver_t *pdm_hw_builtin_driver_first(void)
{
	return &pdm_hw_builtin_driver_start + 1;
}

static const pdm_hw_builtin_driver_t *pdm_hw_builtin_driver_last(void)
{
	return &pdm_hw_builtin_driver_end;
}

static void
pdm_hw_builtin_drivers_exit_range(const pdm_hw_builtin_driver_t *end)
{
	const pdm_hw_builtin_driver_t *first = pdm_hw_builtin_driver_first();
	const pdm_hw_builtin_driver_t *driver = end;

	while (driver > first) {
		driver--;
		if (driver->exit)
			driver->exit();
	}
}

static int pdm_hw_builtin_drivers_init(void)
{
	const pdm_hw_builtin_driver_t *driver;
	int ret;

	for (driver = pdm_hw_builtin_driver_first();
	     driver < pdm_hw_builtin_driver_last(); driver++) {
		if (!driver->init)
			continue;

		ret = driver->init();
		if (ret) {
			LOG_ERROR("PDM_HW", "Builtin driver %s init failed: %d",
				  driver->name ? driver->name : "unknown", ret);
			pdm_hw_builtin_drivers_exit_range(driver);
			return ret;
		}

		LOG_INFO("PDM_HW", "Builtin driver %s initialized",
			 driver->name ? driver->name : "unknown");
	}

	g_lpf_hw_builtin_drivers_ready = true;
	return 0;
}

static void pdm_hw_builtin_drivers_exit(void)
{
	if (!g_lpf_hw_builtin_drivers_ready)
		return;

	pdm_hw_builtin_drivers_exit_range(pdm_hw_builtin_driver_last());
	g_lpf_hw_builtin_drivers_ready = false;
}

int32_t pdm_hw_runtime_init(void)
{
	int ret;

	ret = pdm_hw_builtin_drivers_init();
	if (ret)
		return ret;

	LOG_INFO("PDM_HW", "runtime initialized");
	return OSAL_SUCCESS;
}

void pdm_hw_runtime_exit(void)
{
	pdm_hw_builtin_drivers_exit();
	LOG_INFO("PDM_HW", "runtime exited");
}
