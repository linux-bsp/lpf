// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>

#include "osal/osal.h"
#include "generated/gen_version.h"

void osal_print_version(void)
{
	osal_log(OS_LOG_LEVEL_INFO, "OSAL",
		 "module_version=%u.%u.%u middleware_version=%s git=%s build_time=%s build_by=%s@%s compiler=%s arch=%s kernel=%s",
		 OSAL_LITE_VERSION_MAJOR, OSAL_LITE_VERSION_MINOR,
		 OSAL_LITE_VERSION_PATCH, ES_MIDDLEWARE_VERSION,
		 ES_MIDDLEWARE_GIT_COMMIT, ES_MIDDLEWARE_COMPILE_TIME,
		 ES_MIDDLEWARE_COMPILE_BY, ES_MIDDLEWARE_COMPILE_HOST,
		 ES_MIDDLEWARE_COMPILER, ES_MIDDLEWARE_BUILD_ARCH,
		 ES_MIDDLEWARE_BUILD_KERNEL);
}
EXPORT_SYMBOL_GPL(osal_print_version);

static int __init osal_init(void)
{
	osal_print_version();
	LOG_INFO("OSAL", "loaded");
	return 0;
}

static void __exit osal_exit(void)
{
	LOG_INFO("OSAL", "unloaded");
}

module_init(osal_init);
module_exit(osal_exit);

MODULE_AUTHOR("ES-Middleware");
MODULE_DESCRIPTION("ES-Middleware OSAL kernel module");
MODULE_LICENSE("GPL");
