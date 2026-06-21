// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>

#include "osal/osal.h"
#include "generated/gen_version.h"

void osal_print_version(void)
{
	osal_log(OS_LOG_LEVEL_INFO, "OSAL",
		 "================ OSAL version info ================");
	osal_log(OS_LOG_LEVEL_INFO, "OSAL", "module_version=%u.%u.%u",
		 OSAL_LITE_VERSION_MAJOR, OSAL_LITE_VERSION_MINOR,
		 OSAL_LITE_VERSION_PATCH);
	osal_log(OS_LOG_LEVEL_INFO, "OSAL", "lpf_version=%s", LPF_VERSION);
	osal_log(OS_LOG_LEVEL_INFO, "OSAL", "git=%s", LPF_GIT_COMMIT);
	osal_log(OS_LOG_LEVEL_INFO, "OSAL", "build_time=%s", LPF_COMPILE_TIME);
	osal_log(OS_LOG_LEVEL_INFO, "OSAL", "build_by=%s@%s",
		 LPF_COMPILE_BY, LPF_COMPILE_HOST);
	osal_log(OS_LOG_LEVEL_INFO, "OSAL", "compiler=%s", LPF_COMPILER);
	osal_log(OS_LOG_LEVEL_INFO, "OSAL", "arch=%s", LPF_BUILD_ARCH);
	osal_log(OS_LOG_LEVEL_INFO, "OSAL", "kernel=%s", LPF_BUILD_KERNEL);
	osal_log(OS_LOG_LEVEL_INFO, "OSAL",
		 "===================================================");
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
