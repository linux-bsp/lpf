/*
 * Example program demonstrating ES-Middleware version information usage
 * Shows how to use OSAL version APIs and convenience macros
 */

#include "osal.h"

int main(void)
{
    /* Method 1: Use the high-level print function */
    OSAL_Printf("\n=== Method 1: Using OSAL_PrintVersionInfo() ===\n\n");
    OSAL_PrintVersionInfo();

    /* Method 2: Query individual version fields */
    OSAL_Printf("\n=== Method 2: Using individual query APIs ===\n\n");
    OSAL_Printf("Version:      %s\n", OSAL_GetVersion());
    OSAL_Printf("Full Version: %s\n", OSAL_GetVersionFull());
    OSAL_Printf("Git Commit:   %s\n", OSAL_GetGitCommit());
    OSAL_Printf("Build Time:   %s\n", OSAL_GetBuildTime());
    OSAL_Printf("Built by:     %s\n", OSAL_GetBuildBy());
    OSAL_Printf("Compiler:     %s\n", OSAL_GetCompiler());
    OSAL_Printf("Architecture: %s\n", OSAL_GetArch());
    OSAL_Printf("Kernel:       %s\n", OSAL_GetKernel());

    /* Method 3: Use convenience macros (kernel-style) */
    OSAL_Printf("\n=== Method 3: Using convenience macros ===\n\n");
    OSAL_Printf("OSAL_VERSION:      %s\n", OSAL_VERSION);
    OSAL_Printf("OSAL_VERSION_FULL: %s\n", OSAL_VERSION_FULL);
    OSAL_Printf("OSAL_GIT_COMMIT:   %s\n", OSAL_GIT_COMMIT);
    OSAL_Printf("OSAL_BUILD_TIME:   %s\n", OSAL_BUILD_TIME);

    /* Method 4: Use location macros (useful for logging) */
    OSAL_Printf("\n=== Method 4: Location and debug macros ===\n\n");
    OSAL_Printf("Current file:     %s\n", OSAL_FILE);
    OSAL_Printf("Current line:     %d\n", OSAL_LINE);
    OSAL_Printf("Current function: %s\n", OSAL_FUNC);
    OSAL_Printf("Location string:  %s\n", OSAL_LOCATION);
    OSAL_Printf("Version+Location: %s\n", OSAL_VERSION_LOCATION);

    OSAL_Printf("\n");
    return 0;
}
