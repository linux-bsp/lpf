// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>

#include "osal/osal.h"
#include "generated/gen_version.h"

void osal_print_version(void)
{
	osal_log(OS_LOG_LEVEL_INFO, "OSAL",
		 "module_version=%u.%u.%u lpf_version=%s git=%s build_time=%s build_by=%s@%s compiler=%s arch=%s kernel=%s",
		 OSAL_LITE_VERSION_MAJOR, OSAL_LITE_VERSION_MINOR,
		 OSAL_LITE_VERSION_PATCH, LPF_VERSION,
		 LPF_GIT_COMMIT, LPF_COMPILE_TIME,
		 LPF_COMPILE_BY, LPF_COMPILE_HOST,
		 LPF_COMPILER, LPF_BUILD_ARCH,
		 LPF_BUILD_KERNEL);
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

MODULE_AUTHOR("LPF");
MODULE_DESCRIPTION("LPF OSAL kernel module");
MODULE_LICENSE("GPL");
