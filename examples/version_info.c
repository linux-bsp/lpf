/*
 * Example program demonstrating LPF version information usage
 * Shows how to use OSAL version APIs and convenience macros
 */

#include "osal.h"

int main(void)
{
    /* Method 1: Use the high-level print function */
    OSAL_printf("\n=== Method 1: Using OSAL_print_version_info() ===\n\n");
    OSAL_print_version_info();

    /* Method 2: Query individual version fields */
    OSAL_printf("\n=== Method 2: Using individual query APIs ===\n\n");
    OSAL_printf("Version:      %s\n", OSAL_get_version());
    OSAL_printf("Full Version: %s\n", OSAL_get_version_full());
    OSAL_printf("Git Commit:   %s\n", OSAL_get_git_commit());
    OSAL_printf("Build Time:   %s\n", OSAL_get_build_time());
    OSAL_printf("Built by:     %s\n", OSAL_get_build_by());
    OSAL_printf("Compiler:     %s\n", OSAL_get_compiler());
    OSAL_printf("Architecture: %s\n", OSAL_get_arch());
    OSAL_printf("Kernel:       %s\n", OSAL_get_kernel());

    /* Method 3: Use convenience macros (kernel-style) */
    OSAL_printf("\n=== Method 3: Using convenience macros ===\n\n");
    OSAL_printf("OSAL_VERSION:      %s\n", OSAL_VERSION);
    OSAL_printf("OSAL_VERSION_FULL: %s\n", OSAL_VERSION_FULL);
    OSAL_printf("OSAL_GIT_COMMIT:   %s\n", OSAL_GIT_COMMIT);
    OSAL_printf("OSAL_BUILD_TIME:   %s\n", OSAL_BUILD_TIME);

    /* Method 4: Use location macros (useful for logging) */
    OSAL_printf("\n=== Method 4: Location and debug macros ===\n\n");
    OSAL_printf("Current file:     %s\n", OSAL_FILE);
    OSAL_printf("Current line:     %d\n", OSAL_LINE);
    OSAL_printf("Current function: %s\n", OSAL_FUNC);
    OSAL_printf("Location string:  %s\n", OSAL_LOCATION);
    OSAL_printf("Version+Location: %s\n", OSAL_VERSION_LOCATION);

    OSAL_printf("\n");
    return 0;
}
