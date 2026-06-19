// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>
#include <linux/errno.h>

#include "osal.h"
#include "pdm/pdm.h"
#include "pconfig/pconfig.h"
#include "pdm_bus.h"
#include "pdm_driver.h"
#include "pdm_status.h"
#include "generated/gen_version.h"

static int pdm_probe_devices(void)
{
	const pconfig_device_config_t *device;
	int32_t ret;

	ret = pconfig_load();
	if (ret != OSAL_SUCCESS)
		return pdm_status_to_errno(ret);

	device = pconfig_get();
	if (!device)
		return -ENODEV;

	for (; device->device_type != PCONFIG_DEVICE_TYPE_INVALID; device++) {
		ret = pdm_bus_register_device(device);
		if (ret != OSAL_SUCCESS) {
			ret = pdm_status_to_errno(ret);
			goto out_error;
		}
	}

	return 0;

out_error:
	pdm_bus_remove_devices();
	return ret;
}

void pdm_print_version(void)
{
	osal_log(OS_LOG_LEVEL_INFO, "PDM",
		 "module_version=%u.%u.%u lpf_version=%s git=%s build_time=%s build_by=%s@%s compiler=%s arch=%s kernel=%s",
		 PDM_VERSION_MAJOR, PDM_VERSION_MINOR, PDM_VERSION_PATCH,
		 LPF_VERSION, LPF_GIT_COMMIT,
		 LPF_COMPILE_TIME, LPF_COMPILE_BY,
		 LPF_COMPILE_HOST, LPF_COMPILER,
		 LPF_BUILD_ARCH, LPF_BUILD_KERNEL);
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
	pdm_bus_remove_devices();
	pdm_drivers_exit();
	pdm_driver_registry_deinit();
	LOG_INFO("PDM", "unloaded");
}

module_init(pdm_init);
module_exit(pdm_exit);

MODULE_AUTHOR("LPF");
MODULE_DESCRIPTION("LPF PDM kernel module");
MODULE_LICENSE("GPL");
MODULE_SOFTDEP("pre: osal pconfig hal can can_raw");
