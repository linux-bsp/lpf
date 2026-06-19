// SPDX-License-Identifier: GPL-2.0

#include "pdm_driver.h"
#include "pdm_status.h"

#define PDM_DRIVER_TABLE_SIZE 16U

typedef struct {
	pconfig_device_type_t type;
	const pdm_driver_t *driver;
} pdm_driver_entry_t;

static pdm_driver_entry_t g_pdm_drivers[PDM_DRIVER_TABLE_SIZE];
static osal_mutex_t g_pdm_driver_lock;
static bool g_pdm_driver_lock_ready;
static bool g_pdm_drivers_ready;

int32_t pdm_driver_registry_init(void)
{
	if (g_pdm_driver_lock_ready)
		return OSAL_SUCCESS;

	if (osal_mutex_init(&g_pdm_driver_lock, NULL) != OSAL_SUCCESS)
		return OSAL_ERR_GENERIC;

	g_pdm_driver_lock_ready = true;
	return OSAL_SUCCESS;
}

void pdm_driver_registry_deinit(void)
{
	if (!g_pdm_driver_lock_ready)
		return;

	osal_memset(g_pdm_drivers, 0, sizeof(g_pdm_drivers));
	osal_mutex_destroy(&g_pdm_driver_lock);
	g_pdm_driver_lock_ready = false;
}

static const pdm_driver_t *
pdm_driver_find_locked(pconfig_device_type_t type)
{
	uint32_t i;

	for (i = 0; i < OSAL_ARRAY_SIZE(g_pdm_drivers); i++) {
		if (g_pdm_drivers[i].type == type)
			return g_pdm_drivers[i].driver;
	}

	return NULL;
}

const pdm_driver_t *pdm_driver_find(pconfig_device_type_t type)
{
	const pdm_driver_t *driver;

	if (!g_pdm_driver_lock_ready)
		return NULL;

	osal_mutex_lock(&g_pdm_driver_lock);
	driver = pdm_driver_find_locked(type);
	osal_mutex_unlock(&g_pdm_driver_lock);

	return driver;
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
	uint32_t i;

	if (!driver || driver->type == PCONFIG_DEVICE_TYPE_INVALID ||
	    !driver->probe)
		return OSAL_ERR_INVALID_PARAM;

	if (pdm_driver_registry_init() != OSAL_SUCCESS)
		return OSAL_ERR_GENERIC;

	osal_mutex_lock(&g_pdm_driver_lock);
	if (pdm_driver_find_locked(driver->type)) {
		osal_mutex_unlock(&g_pdm_driver_lock);
		return OSAL_ERR_ALREADY_EXISTS;
	}

	for (i = 0; i < OSAL_ARRAY_SIZE(g_pdm_drivers); i++) {
		if (!g_pdm_drivers[i].driver) {
			g_pdm_drivers[i].type = driver->type;
			g_pdm_drivers[i].driver = driver;
			osal_mutex_unlock(&g_pdm_driver_lock);
			LOG_INFO("PDM", "Registered driver %s type=%u",
				 driver->name ? driver->name : "unknown",
				 driver->type);
			return OSAL_SUCCESS;
		}
	}
	osal_mutex_unlock(&g_pdm_driver_lock);

	return OSAL_ERR_RESOURCE_LIMIT;
}

static void pdm_driver_del(const pdm_driver_t *driver)
{
	uint32_t i;

	if (!g_pdm_driver_lock_ready || !driver)
		return;

	osal_mutex_lock(&g_pdm_driver_lock);
	for (i = 0; i < OSAL_ARRAY_SIZE(g_pdm_drivers); i++) {
		if (g_pdm_drivers[i].driver == driver) {
			g_pdm_drivers[i].type = PCONFIG_DEVICE_TYPE_INVALID;
			g_pdm_drivers[i].driver = NULL;
			break;
		}
	}
	osal_mutex_unlock(&g_pdm_driver_lock);
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

	pdm_drivers_exit_range(pdm_driver_last());
	g_pdm_drivers_ready = false;
}

void pdm_drivers_remove_all(void)
{
	const pdm_driver_t *driver_table[PDM_DRIVER_TABLE_SIZE];
	uint32_t i;

	osal_memset(driver_table, 0, sizeof(driver_table));

	if (g_pdm_driver_lock_ready) {
		osal_mutex_lock(&g_pdm_driver_lock);
		for (i = 0; i < OSAL_ARRAY_SIZE(g_pdm_drivers); i++)
			driver_table[i] = g_pdm_drivers[i].driver;
		osal_mutex_unlock(&g_pdm_driver_lock);
	}

	for (i = 0; i < OSAL_ARRAY_SIZE(driver_table); i++) {
		if (driver_table[i] && driver_table[i]->remove_all)
			driver_table[i]->remove_all();
	}
}
