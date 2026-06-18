/**
 * @file ctest_aconfig.c
 * @brief Static ACONFIG data for the ctest product.
 */

#include "osal.h"
#include "aconfig.h"

const aconfig_config_table_t g_aconfig_table = { .name = "ctest_aconfig",
                                                 .function_map = NULL,
                                                 .user_data = NULL };
