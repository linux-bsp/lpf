/*
 * Example program demonstrating ES-Middleware version information usage
 * Similar to Linux kernel's version display
 */

#include "osal.h"
#include "version.h"
#include "autoconf.h"

int main(void)
{
    OSAL_printf("=====================================================\n");
    OSAL_printf("ES-Middleware Version Information\n");
    OSAL_printf("=====================================================\n\n");

    /* Display full version banner (like Linux kernel) */
    OSAL_printf("Banner:\n");
    OSAL_printf("  %s\n\n", ES_MIDDLEWARE_BANNER);

    /* Display detailed version information */
    OSAL_printf("Version Details:\n");
    OSAL_printf("  Version:           %s\n", ES_MIDDLEWARE_VERSION);
    OSAL_printf("  Version String:    %s\n", ES_MIDDLEWARE_VERSION_STRING);
    OSAL_printf("  Git Commit:        %s\n\n", ES_MIDDLEWARE_GIT_COMMIT);

    /* Display build information */
    OSAL_printf("Build Information:\n");
    OSAL_printf("  Built by:          %s@%s\n",
           ES_MIDDLEWARE_COMPILE_BY, ES_MIDDLEWARE_COMPILE_HOST);
    OSAL_printf("  Compiler:          %s\n", ES_MIDDLEWARE_COMPILER);
    OSAL_printf("  Build time:        %s\n", ES_MIDDLEWARE_COMPILE_TIME);
    OSAL_printf("  Build timestamp:   %ld\n\n", ES_MIDDLEWARE_COMPILE_TIMESTAMP);

    /* Display platform information */
    OSAL_printf("Platform Information:\n");
    OSAL_printf("  Architecture:      %s\n", ES_MIDDLEWARE_BUILD_ARCH);
    OSAL_printf("  Kernel:            %s\n\n", ES_MIDDLEWARE_BUILD_KERNEL);

    /* Display some configuration from autoconf.h */
    OSAL_printf("Configuration:\n");
#ifdef CONFIG_OSAL_POSIX
    OSAL_printf("  OSAL:              POSIX\n");
#endif
#ifdef CONFIG_HAL_X86
    OSAL_printf("  HAL:               x86\n");
#endif
#ifdef CONFIG_DEBUG
    OSAL_printf("  Debug:             Enabled\n");
#else
    OSAL_printf("  Debug:             Disabled\n");
#endif

    OSAL_printf("\n=====================================================\n");

    return 0;
}
