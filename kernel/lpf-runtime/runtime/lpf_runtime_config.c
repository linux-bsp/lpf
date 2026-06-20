// SPDX-License-Identifier: GPL-2.0

#include "lpf/runtime/lpf_runtime.h"

#include "lpf/core/lpf_core.h"
#include "lpf_runtime_internal.h"

static const lpf_runtime_config_driver_t *
lpf_runtime_config_driver_first(void)
{
	return &lpf_runtime_config_driver_start + 1;
}

static const lpf_runtime_config_driver_t *
lpf_runtime_config_driver_last(void)
{
	return &lpf_runtime_config_driver_end;
}

int32_t lpf_runtime_probe_devices(void)
{
	const lpf_config_platform_config_t *platform;
	const lpf_runtime_config_driver_t *driver;
	int32_t ret;

	ret = lpf_config_load();
	if (ret != OSAL_SUCCESS)
		return ret;

	platform = lpf_config_get_board();
	if (!platform)
		return OSAL_ENODEV;

	for (driver = lpf_runtime_config_driver_first();
	     driver < lpf_runtime_config_driver_last(); driver++) {
		if (!driver->probe)
			continue;

		ret = driver->probe(platform);
		if (ret != OSAL_SUCCESS)
			goto out_error;
	}

	return OSAL_SUCCESS;

out_error:
	lpf_device_unregister_all();
	return ret;
}
