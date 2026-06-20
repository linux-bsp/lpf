// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>

#include "osal.h"
#include "lpf/lpf_errno.h"
#include "lpf/runtime/lpf_runtime.h"

static int __init lpf_runtime_module_init(void)
{
	int ret;

	lpf_runtime_print_version();

	ret = lpf_runtime_init();
	if (ret != OSAL_SUCCESS)
		return lpf_status_to_errno(ret);

	LOG_INFO("LPF-RUNTIME", "loaded");
	return 0;
}

static void __exit lpf_runtime_module_exit(void)
{
	lpf_runtime_exit();
	LOG_INFO("LPF-RUNTIME", "unloaded");
}

module_init(lpf_runtime_module_init);
module_exit(lpf_runtime_module_exit);

MODULE_AUTHOR("LPF");
MODULE_DESCRIPTION("LPF runtime integration module");
MODULE_LICENSE("GPL");
MODULE_SOFTDEP("pre: osal lpf_core can can_raw");
