/**
 * @file test_pconfig_api.c
 * @brief PCONFIG API unit tests
 */

#include <test_framework/test_framework.h>
#include "osal.h"
#include "pconfig/pconfig.h"

static void test_pconfig_get_board(void)
{
    const pconfig_platform_config_t *cfg = PCONFIG_GetBoard();
    TEST_ASSERT_NOT_NULL(cfg);
    TEST_ASSERT_EQUAL(0, OSAL_strcmp(cfg->platform_name, "generic"));
    TEST_ASSERT_EQUAL(0, OSAL_strcmp(cfg->product_name, "middleware"));
}

static void test_pconfig_find_by_name(void)
{
    const pconfig_platform_config_t *cfg =
        PCONFIG_Find("ti", "framework", NULL);
    TEST_ASSERT_NOT_NULL(cfg);
    TEST_ASSERT_EQUAL(0, OSAL_strcmp(cfg->chip_name, "am625"));
}

static void test_pconfig_find_alt_config(void)
{
    const pconfig_platform_config_t *cfg =
        PCONFIG_Find("ti", "framework_alt", NULL);
    TEST_ASSERT_NOT_NULL(cfg);
    TEST_ASSERT_EQUAL(0, OSAL_strcmp(cfg->product_name, "framework_alt"));
}

static void test_pconfig_find_missing_config(void)
{
    const pconfig_platform_config_t *cfg =
        PCONFIG_Find("missing", "framework", NULL);
    TEST_ASSERT_NULL(cfg);
}

static void test_pconfig_hw_get_mcu_by_id(void)
{
    const pconfig_platform_config_t *platform =
        PCONFIG_Find("ti", "framework", NULL);
    const pconfig_mcu_entry_t *mcu = PCONFIG_HW_GetMCU(platform, 0);
    TEST_ASSERT_NOT_NULL(mcu);
    TEST_ASSERT_EQUAL(0, OSAL_strcmp(mcu->config.name, "mcu0"));
}

static void test_pconfig_hw_get_mcu_second_entry(void)
{
    const pconfig_platform_config_t *platform =
        PCONFIG_Find("ti", "framework", NULL);
    const pconfig_mcu_entry_t *mcu = PCONFIG_HW_GetMCU(platform, 1);
    TEST_ASSERT_NOT_NULL(mcu);
    TEST_ASSERT_EQUAL(0, OSAL_strcmp(mcu->config.name, "mcu1"));
}

static void test_pconfig_validate_success(void)
{
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, PCONFIG_validate(PCONFIG_GetBoard()));
}

static void test_pconfig_list(void)
{
    const pconfig_platform_config_t *configs[10];
    uint32_t count = 10;

    TEST_ASSERT_EQUAL(OSAL_SUCCESS, PCONFIG_list(configs, &count));
    TEST_ASSERT_EQUAL(3u, count);
    TEST_ASSERT_NOT_NULL(configs[0]);
    TEST_ASSERT_NOT_NULL(configs[1]);
    TEST_ASSERT_NOT_NULL(configs[2]);
}

static const test_case_t test_cases[] = {
    { .name = "test_pconfig_get_board",
      .func = test_pconfig_get_board,
      .setup = NULL,
      .teardown = NULL },
    { .name = "test_pconfig_find_by_name",
      .func = test_pconfig_find_by_name,
      .setup = NULL,
      .teardown = NULL },
    { .name = "test_pconfig_find_alt_config",
      .func = test_pconfig_find_alt_config,
      .setup = NULL,
      .teardown = NULL },
    { .name = "test_pconfig_find_missing_config",
      .func = test_pconfig_find_missing_config,
      .setup = NULL,
      .teardown = NULL },
    { .name = "test_pconfig_hw_get_mcu_by_id",
      .func = test_pconfig_hw_get_mcu_by_id,
      .setup = NULL,
      .teardown = NULL },
    { .name = "test_pconfig_hw_get_mcu_second_entry",
      .func = test_pconfig_hw_get_mcu_second_entry,
      .setup = NULL,
      .teardown = NULL },
    { .name = "test_pconfig_validate_success",
      .func = test_pconfig_validate_success,
      .setup = NULL,
      .teardown = NULL },
    { .name = "test_pconfig_list",
      .func = test_pconfig_list,
      .setup = NULL,
      .teardown = NULL },
};

static const test_suite_t test_suite = {
    .suite_name = "pconfig_api",
    .module_name = "pconfig",
    .layer_name = "UNIT",
    .cases = test_cases,
    .case_count = OSAL_sizeof(test_cases) / OSAL_sizeof(test_case_t),
    .suite_setup = NULL,
    .suite_teardown = NULL,
    .metadata = { .category = TEST_CATEGORY_UNIT,
                  .tags = TEST_TAG_FAST,
                  .timeout_ms = 100,
                  .description = "PCONFIG API unit tests" }
};

__attribute__((constructor)) static void register_pconfig_api_tests(void)
{
    libutest_register_suite(&test_suite);
}
