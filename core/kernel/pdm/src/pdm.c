// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>

#include "osal.h"
#include "pdm/pdm.h"
#include "pconfig/pconfig.h"
#include "pdm_mcu/pdm_mcu_internal.h"
#include "generated/gen_version.h"
#ifdef CONFIG_PDM_PROTOCOL
#include "pdm_protocol.h"
#endif

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
#ifdef CONFIG_PCONFIG
	pconfig_print_version();
#endif

#ifdef CONFIG_PDM_PROTOCOL
	ret = pdm_protocol_init();
	if (ret)
		return ret;
#endif

#ifdef CONFIG_PDM_MCU_SUPPORT
	ret = pdm_mcu_chrdev_register();
	if (ret) {
#ifdef CONFIG_PDM_PROTOCOL
		pdm_protocol_deinit();
#endif
		return ret;
	}
#else
	ret = 0;
#endif

	LOG_INFO("PDM", "loaded");
	return 0;
}

static void __exit pdm_exit(void)
{
#ifdef CONFIG_PDM_MCU_SUPPORT
	pdm_mcu_chrdev_unregister();
#endif
#ifdef CONFIG_PDM_PROTOCOL
	pdm_protocol_deinit();
#endif
	LOG_INFO("PDM", "unloaded");
}

module_init(pdm_init);
module_exit(pdm_exit);

MODULE_AUTHOR("ES-Middleware");
MODULE_DESCRIPTION("ES-Middleware PDM kernel module");
MODULE_LICENSE("GPL");
MODULE_SOFTDEP("pre: osal hal can can_raw");
