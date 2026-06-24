// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>

#include "osal/osal.h"
#include "generated/gen_version.h"

static void osal_print_version(void)
{
	LOG_INFO("================ OSAL version info ================");
	LOG_INFO("osal_version=%u.%u.%u",
		 OSAL_LITE_VERSION_MAJOR, OSAL_LITE_VERSION_MINOR,
		 OSAL_LITE_VERSION_PATCH);
	LOG_INFO("git=%s", PDM_GIT_COMMIT);
	LOG_INFO("build_time=%s", PDM_COMPILE_TIME);
	LOG_INFO("build_by=%s@%s", PDM_COMPILE_BY, PDM_COMPILE_HOST);
	LOG_INFO("compiler=%s", PDM_COMPILER);
	LOG_INFO("arch=%s", PDM_BUILD_ARCH);
	LOG_INFO("kernel=%s", PDM_BUILD_KERNEL);
	LOG_INFO("===================================================");
}

static int __init osal_init(void)
{
	osal_print_version();
	LOG_INFO("loaded");
	return 0;
}

static void __exit osal_exit(void)
{
	LOG_INFO("unloaded");
}

module_init(osal_init);
module_exit(osal_exit);

MODULE_AUTHOR("PDM");
MODULE_DESCRIPTION("PDM OSAL kernel module");
MODULE_LICENSE("GPL");
