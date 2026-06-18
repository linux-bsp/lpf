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

const char *OSAL_get_version_string(void)
{
    snprintf(s_version_string_buf,
             sizeof(s_version_string_buf),
             "OSAL v%s",
             ES_MIDDLEWARE_VERSION);
    return s_version_string_buf;
}

const char *OSAL_get_version(void)
{
    return ES_MIDDLEWARE_VERSION;
}

const char *OSAL_get_version_full(void)
{
    return ES_MIDDLEWARE_VERSION_STRING;
}

const char *OSAL_get_git_commit(void)
{
    return ES_MIDDLEWARE_GIT_COMMIT;
}

const char *OSAL_get_build_time(void)
{
    return ES_MIDDLEWARE_COMPILE_TIME;
}

const char *OSAL_get_build_by(void)
{
    snprintf(s_build_by_buf,
             sizeof(s_build_by_buf),
             "%s@%s",
             ES_MIDDLEWARE_COMPILE_BY,
             ES_MIDDLEWARE_COMPILE_HOST);
    return s_build_by_buf;
}

const char *OSAL_get_compiler(void)
{
    return ES_MIDDLEWARE_COMPILER;
}

const char *OSAL_get_arch(void)
{
    return ES_MIDDLEWARE_BUILD_ARCH;
}

const char *OSAL_get_kernel(void)
{
    return ES_MIDDLEWARE_BUILD_KERNEL;
}

const char *OSAL_get_banner(void)
{
    return ES_MIDDLEWARE_BANNER;
}

void OSAL_print_version_info(void)
{
    OSAL_printf(
        "=================================================================\n");
    OSAL_printf("ES-Middleware Version Information\n");
    OSAL_printf(
        "=================================================================\n");
    OSAL_printf("Version:      %s\n", OSAL_get_version());
    OSAL_printf("Full Version: %s\n", OSAL_get_version_full());
    OSAL_printf("Git Commit:   %s\n", OSAL_get_git_commit());
    OSAL_printf("Build Time:   %s\n", OSAL_get_build_time());
    OSAL_printf("Built by:     %s\n", OSAL_get_build_by());
    OSAL_printf("Compiler:     %s\n", OSAL_get_compiler());
    OSAL_printf("Architecture: %s\n", OSAL_get_arch());
    OSAL_printf("Kernel:       %s\n", OSAL_get_kernel());
    OSAL_printf(
        "=================================================================\n");
    OSAL_printf("\nBanner:\n%s\n", OSAL_get_banner());
    OSAL_printf(
        "=================================================================\n");
}
