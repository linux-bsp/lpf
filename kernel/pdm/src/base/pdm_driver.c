// SPDX-License-Identifier: GPL-2.0

#include "pdm_bus.h"
#include "pdm_driver.h"
#include "pdm_status.h"

static bool g_pdm_drivers_ready;

int32_t pdm_driver_registry_init(void)
{
	return pdm_bus_init();
}

void pdm_driver_registry_deinit(void)
{
	pdm_bus_deinit();
}

static const pdm_driver_t *const *pdm_driver_first(void)
{
	return &pdm_driver_start + 1;
}

static const pdm_driver_t *const *pdm_driver_last(void)
{
	return &pdm_driver_end;
}

static int32_t pdm_driver_add(const pdm_driver_t *driver)
{
	return pdm_bus_register_driver(driver);
}

static void pdm_driver_del(const pdm_driver_t *driver)
{
	pdm_bus_unregister_driver(driver);
}

static void pdm_drivers_exit_range(const pdm_driver_t *const *end)
{
	const pdm_driver_t *const *first = pdm_driver_first();
	const pdm_driver_t *const *entry = end;

	while (entry > first) {
		const pdm_driver_t *driver;

		entry--;
		driver = *entry;
		if (!driver)
			continue;

		pdm_driver_del(driver);
		if (driver->exit)
			driver->exit();
	}
}

int pdm_drivers_init(void)
{
	const pdm_driver_t *const *entry;
	long err;
	int ret;

	for (entry = pdm_driver_first(); entry < pdm_driver_last(); entry++) {
		const pdm_driver_t *driver = *entry;

		if (!driver)
			continue;

		if (driver->init) {
			ret = driver->init();
			if (ret) {
				LOG_ERROR("PDM", "Driver %s init failed: %d",
					  driver->name ? driver->name :
							 "unknown",
					  ret);
				pdm_drivers_exit_range(entry);
				return ret < 0 ? ret : pdm_status_to_errno(ret);
			}
		}

		err = pdm_status_to_errno(pdm_driver_add(driver));
		if (err) {
			LOG_ERROR("PDM", "Driver %s register failed: %ld",
				  driver->name ? driver->name : "unknown",
				  err);
			if (driver->exit)
				driver->exit();
			pdm_drivers_exit_range(entry);
			return (int)err;
		}
	}

	g_pdm_drivers_ready = true;
	return 0;
}

void pdm_drivers_exit(void)
{
	if (!g_pdm_drivers_ready)
		return;

	pdm_bus_remove_devices();
	pdm_drivers_exit_range(pdm_driver_last());
	g_pdm_drivers_ready = false;
}
