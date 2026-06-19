// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>

#include "osal.h"
#include "hal/hal.h"
#include "hal_internal.h"
#include "generated/gen_version.h"

void hal_print_version(void)
{
	osal_log(OS_LOG_LEVEL_INFO, "HAL",
		 "module_version=%u.%u.%u middleware_version=%s git=%s build_time=%s build_by=%s@%s compiler=%s arch=%s kernel=%s",
		 HAL_VERSION_MAJOR, HAL_VERSION_MINOR, HAL_VERSION_PATCH,
		 ES_MIDDLEWARE_VERSION, ES_MIDDLEWARE_GIT_COMMIT,
		 ES_MIDDLEWARE_COMPILE_TIME, ES_MIDDLEWARE_COMPILE_BY,
		 ES_MIDDLEWARE_COMPILE_HOST, ES_MIDDLEWARE_COMPILER,
		 ES_MIDDLEWARE_BUILD_ARCH, ES_MIDDLEWARE_BUILD_KERNEL);
}
EXPORT_SYMBOL_GPL(hal_print_version);

static int __init hal_init(void)
{
	hal_print_version();
#ifdef CONFIG_HAL_GPIO
	int ret;

	ret = hal_gpio_module_init();
	if (ret)
		return ret;
#endif

	LOG_INFO("HAL", "loaded");
	return 0;
}

static void __exit hal_exit(void)
{
#ifdef CONFIG_HAL_GPIO
	hal_gpio_module_deinit();
#endif
	LOG_INFO("HAL", "unloaded");
}

module_init(hal_init);
module_exit(hal_exit);

MODULE_AUTHOR("ES-Middleware");
MODULE_DESCRIPTION("ES-Middleware HAL kernel module");
MODULE_LICENSE("GPL");
MODULE_SOFTDEP("pre: osal can can_raw");
