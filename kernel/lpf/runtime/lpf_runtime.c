// SPDX-License-Identifier: GPL-2.0

#include "lpf/runtime/lpf_runtime.h"

#include "lpf/core/lpf_core.h"
#include "lpf/hw/lpf_hw.h"
#include "lpf/config/lpf_config.h"
#include "lpf/runtime/lpf_runtime_internal.h"
#include "generated/gen_version.h"

static bool g_lpf_runtime_ready;
static bool g_lpf_runtime_entries_ready;

void lpf_runtime_print_version(void)
{
	osal_log(OS_LOG_LEVEL_INFO, "LPF-RUNTIME",
		 "module_version=%u.%u.%u lpf_version=%s git=%s build_time=%s build_by=%s@%s compiler=%s arch=%s kernel=%s",
		 LPF_RUNTIME_VERSION_MAJOR,
		 LPF_RUNTIME_VERSION_MINOR,
		 LPF_RUNTIME_VERSION_PATCH,
		 LPF_VERSION, LPF_GIT_COMMIT,
		 LPF_COMPILE_TIME, LPF_COMPILE_BY,
		 LPF_COMPILE_HOST, LPF_COMPILER,
		 LPF_BUILD_ARCH, LPF_BUILD_KERNEL);
}

static const lpf_runtime_entry_t *lpf_runtime_entry_first(void)
{
	return &lpf_runtime_entry_start + 1;
}

static const lpf_runtime_entry_t *lpf_runtime_entry_last(void)
{
	return &lpf_runtime_entry_end;
}

static void
lpf_runtime_entries_exit_range(const lpf_runtime_entry_t *end)
{
	const lpf_runtime_entry_t *entry;

	entry = end;
	while (entry > lpf_runtime_entry_first()) {
		entry--;
		if (entry->exit)
			entry->exit();
	}
}

static int32_t lpf_runtime_entries_init(void)
{
	const lpf_runtime_entry_t *entry;
	int32_t ret;

	if (g_lpf_runtime_entries_ready)
		return OSAL_SUCCESS;

	for (entry = lpf_runtime_entry_first();
	     entry < lpf_runtime_entry_last(); entry++) {
		if (!entry->init)
			continue;

		ret = entry->init();
		if (ret != OSAL_SUCCESS) {
			LOG_ERROR("LPF-RUNTIME",
				  "peripheral entry %s init failed: %d",
				  entry->name ? entry->name : "unknown",
				  ret);
			lpf_runtime_entries_exit_range(entry);
			return ret;
		}

		LOG_INFO("LPF-RUNTIME", "peripheral entry %s initialized",
			 entry->name ? entry->name : "unknown");
	}

	g_lpf_runtime_entries_ready = true;
	return OSAL_SUCCESS;
}

static void lpf_runtime_entries_exit(void)
{
	if (!g_lpf_runtime_entries_ready)
		return;

	lpf_device_unregister_all();
	lpf_runtime_entries_exit_range(lpf_runtime_entry_last());
	g_lpf_runtime_entries_ready = false;
}

int32_t lpf_runtime_init(void)
{
	int32_t ret;

	if (g_lpf_runtime_ready)
		return OSAL_SUCCESS;

	ret = lpf_core_init();
	if (ret != OSAL_SUCCESS)
		return ret;

	ret = lpf_hw_runtime_init();
	if (ret != OSAL_SUCCESS)
		return ret;

	ret = lpf_runtime_entries_init();
	if (ret != OSAL_SUCCESS) {
		lpf_hw_runtime_exit();
		return ret;
	}

	ret = lpf_runtime_probe_devices();
	if (ret != OSAL_SUCCESS) {
		lpf_runtime_entries_exit();
		lpf_config_unload();
		lpf_hw_runtime_exit();
		return ret;
	}

	g_lpf_runtime_ready = true;
	return OSAL_SUCCESS;
}

void lpf_runtime_exit(void)
{
	if (!g_lpf_runtime_ready)
		return;

	lpf_runtime_entries_exit();
	lpf_config_unload();
	lpf_hw_runtime_exit();
	g_lpf_runtime_ready = false;
}
