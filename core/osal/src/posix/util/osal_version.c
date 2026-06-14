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

const char *OSAL_GetVersionString(void)
{
    snprintf(s_version_string_buf, sizeof(s_version_string_buf),
             "OSAL v%s", ES_MIDDLEWARE_VERSION);
    return s_version_string_buf;
}

const char *OSAL_GetVersion(void)
{
    return ES_MIDDLEWARE_VERSION;
}

const char *OSAL_GetVersionFull(void)
{
    return ES_MIDDLEWARE_VERSION_STRING;
}

const char *OSAL_GetGitCommit(void)
{
    return ES_MIDDLEWARE_GIT_COMMIT;
}

const char *OSAL_GetBuildTime(void)
{
    return ES_MIDDLEWARE_COMPILE_TIME;
}

const char *OSAL_GetBuildBy(void)
{
    snprintf(s_build_by_buf, sizeof(s_build_by_buf),
             "%s@%s", ES_MIDDLEWARE_COMPILE_BY, ES_MIDDLEWARE_COMPILE_HOST);
    return s_build_by_buf;
}

const char *OSAL_GetCompiler(void)
{
    return ES_MIDDLEWARE_COMPILER;
}

const char *OSAL_GetArch(void)
{
    return ES_MIDDLEWARE_BUILD_ARCH;
}

const char *OSAL_GetKernel(void)
{
    return ES_MIDDLEWARE_BUILD_KERNEL;
}

const char *OSAL_GetBanner(void)
{
    return ES_MIDDLEWARE_BANNER;
}

void OSAL_PrintVersionInfo(void)
{
    OSAL_Printf("=================================================================\n");
    OSAL_Printf("ES-Middleware Version Information\n");
    OSAL_Printf("=================================================================\n");
    OSAL_Printf("Version:      %s\n", OSAL_GetVersion());
    OSAL_Printf("Full Version: %s\n", OSAL_GetVersionFull());
    OSAL_Printf("Git Commit:   %s\n", OSAL_GetGitCommit());
    OSAL_Printf("Build Time:   %s\n", OSAL_GetBuildTime());
    OSAL_Printf("Built by:     %s\n", OSAL_GetBuildBy());
    OSAL_Printf("Compiler:     %s\n", OSAL_GetCompiler());
    OSAL_Printf("Architecture: %s\n", OSAL_GetArch());
    OSAL_Printf("Kernel:       %s\n", OSAL_GetKernel());
    OSAL_Printf("=================================================================\n");
    OSAL_Printf("\nBanner:\n%s\n", OSAL_GetBanner());
    OSAL_Printf("=================================================================\n");
}
