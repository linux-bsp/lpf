/**
 * @file osal_version.c
 * @brief OSAL版本信息和构建信息实现
 */

#include "osal.h"
#include <stdio.h>

/* ========================================================================
 * Internal Static Variables
 * ======================================================================== */

static char s_version_string_buf[64];
static char s_build_by_buf[128];

/* ========================================================================
 * Public API Implementation
 * ======================================================================== */

const char *osal_get_version_string(void)
{
	snprintf(s_version_string_buf, sizeof(s_version_string_buf), "OSAL v%s",
			 LPF_VERSION);
	return s_version_string_buf;
}

const char *osal_get_version(void)
{
	return LPF_VERSION;
}

const char *osal_get_version_full(void)
{
	return LPF_VERSION_STRING;
}

const char *osal_get_git_commit(void)
{
	return LPF_GIT_COMMIT;
}

const char *osal_get_build_time(void)
{
	return LPF_COMPILE_TIME;
}

const char *osal_get_build_by(void)
{
	snprintf(s_build_by_buf, sizeof(s_build_by_buf), "%s@%s",
			 LPF_COMPILE_BY, LPF_COMPILE_HOST);
	return s_build_by_buf;
}

const char *osal_get_compiler(void)
{
	return LPF_COMPILER;
}

const char *osal_get_arch(void)
{
	return LPF_BUILD_ARCH;
}

const char *osal_get_kernel(void)
{
	return LPF_BUILD_KERNEL;
}

const char *osal_get_banner(void)
{
	return LPF_BANNER;
}

void osal_print_version_info(void)
{
	osal_printf(
		"=================================================================\n");
	osal_printf("LPF Version Information\n");
	osal_printf(
		"=================================================================\n");
	osal_printf("Version:      %s\n", osal_get_version());
	osal_printf("Full Version: %s\n", osal_get_version_full());
	osal_printf("Git Commit:   %s\n", osal_get_git_commit());
	osal_printf("Build Time:   %s\n", osal_get_build_time());
	osal_printf("Built by:     %s\n", osal_get_build_by());
	osal_printf("Compiler:     %s\n", osal_get_compiler());
	osal_printf("Architecture: %s\n", osal_get_arch());
	osal_printf("Kernel:       %s\n", osal_get_kernel());
	osal_printf(
		"=================================================================\n");
	osal_printf("\nBanner:\n%s\n", osal_get_banner());
	osal_printf(
		"=================================================================\n");
}
