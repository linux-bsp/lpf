// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>
#include <linux/errno.h>

#include "osal.h"
#include "pdm/pdm.h"
#include "pconfig/pconfig.h"
#include "base/pdm_driver.h"
#include "base/pdm_status.h"
#include "generated/gen_version.h"

static int pdm_probe_devices(void)
{
	const pconfig_device_config_t *device;
	int32_t ret;

	ret = pconfig_load();
	if (ret != OSAL_SUCCESS)
		return pdm_status_to_errno(ret);

	device = pconfig_get();
	if (!device) {
		pconfig_unload();
		return -ENODEV;
	}

	for (; device->device_type != PCONFIG_DEVICE_TYPE_INVALID; device++) {
		const pdm_driver_t *driver;

		driver = pdm_driver_find(device->device_type);
		if (!driver || !driver->probe) {
			LOG_ERROR("PDM", "No driver for device type=%u",
				  device->device_type);
			ret = -EOPNOTSUPP;
			goto out_error;
		}

		ret = driver->probe(device);
		if (ret != OSAL_SUCCESS) {
			ret = pdm_status_to_errno(ret);
			goto out_error;
		}
	}

	return 0;

out_error:
	pdm_drivers_remove_all();
	return ret;
}

void pdm_print_version(void)
{
	osal_log(OS_LOG_LEVEL_INFO, "PDM",
		 "module_version=%u.%u.%u middleware_version=%s git=%s build_time=%s build_by=%s@%s compiler=%s arch=%s kernel=%s",
		 PDM_VERSION_MAJOR, PDM_VERSION_MINOR, PDM_VERSION_PATCH,
		 ES_MIDDLEWARE_VERSION, ES_MIDDLEWARE_GIT_COMMIT,
		 ES_MIDDLEWARE_COMPILE_TIME, ES_MIDDLEWARE_COMPILE_BY,
		 ES_MIDDLEWARE_COMPILE_HOST, ES_MIDDLEWARE_COMPILER,
		 ES_MIDDLEWARE_BUILD_ARCH, ES_MIDDLEWARE_BUILD_KERNEL);
}
EXPORT_SYMBOL_GPL(pdm_print_version);

static int __init pdm_init(void)
{
	int ret;

	pdm_print_version();

	ret = pdm_driver_registry_init();
	if (ret != OSAL_SUCCESS)
		return pdm_status_to_errno(ret);

	ret = pdm_drivers_init();
	if (ret)
		goto out_registry_deinit;

	ret = pdm_probe_devices();
	if (ret) {
		pdm_drivers_exit();
		goto out_registry_deinit;
	}

	LOG_INFO("PDM", "loaded");
	return 0;

out_registry_deinit:
	pdm_driver_registry_deinit();
	return ret;
}

static void __exit pdm_exit(void)
{
	pdm_drivers_remove_all();
	pdm_drivers_exit();
	pdm_driver_registry_deinit();
	LOG_INFO("PDM", "unloaded");
}

module_init(pdm_init);
module_exit(pdm_exit);

MODULE_AUTHOR("ES-Middleware");
MODULE_DESCRIPTION("ES-Middleware PDM kernel module");
MODULE_LICENSE("GPL");
MODULE_SOFTDEP("pre: osal pconfig hal can can_raw");
