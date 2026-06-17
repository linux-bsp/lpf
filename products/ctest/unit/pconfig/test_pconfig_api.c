/**
 * @file test_pconfig_api.c
 * @brief PCONFIG API unit tests
 */

#include <test_framework/test_framework.h>
#include "pconfig/pconfig.h"

static pconfig_mcu_entry_t test_mcu_entries[] = {
    {
        .description = "Primary MCU",
        .enabled = true,
        .config = {
            .name = "mcu0",
            .interface = PCONFIG_MCU_INTERFACE_CAN,
            .hw.can = {
                .device = "can0",
                .bitrate = 500000,
                .rx_timeout = 100,
                .tx_timeout = 100,
                .tx_id = 0x100,
                .rx_id = 0x101
            },
            .cmd_timeout_ms = 1000,
            .retry_count = 3
        },
        .reset_gpio = NULL,
        .irq_gpio = NULL
    },
    {
        .description = "Secondary MCU",
        .enabled = true,
        .config = {
            .name = "mcu1",
            .interface = PCONFIG_MCU_INTERFACE_SERIAL,
            .hw.serial = {
                .device = "/dev/ttyS1",
                .baudrate = 115200,
                .data_bits = 8,
                .stop_bits = 1,
                .parity = PCONFIG_MCU_PARITY_NONE,
                .flow_control = PCONFIG_MCU_FLOW_NONE
            },
            .cmd_timeout_ms = 1000,
            .retry_count = 3
        },
        .reset_gpio = NULL,
        .irq_gpio = NULL
    }
};

static pconfig_platform_config_t test_platform_config = {
    .platform_name = "ti",
    .chip_name = "am625",
    .project_name = "test_project",
    .product_name = "framework",
    .mcu_count = 2,
    .mcu_array = test_mcu_entries
};

static pconfig_platform_config_t test_platform_config2 = {
    .platform_name = "ti",
    .chip_name = "am625",
    .project_name = "test_project",
    .product_name = "framework_alt",
    .mcu_count = 0,
    .mcu_array = NULL
};

static void suite_setup(void)
{
    PCONFIG_init();
    PCONFIG_register(&test_platform_config);
}

static void suite_teardown(void)
{
    PCONFIG_cleanup();
}

static void test_pconfig_init_success(void)
{
    PCONFIG_cleanup();
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, PCONFIG_init());
}

static void test_pconfig_register_success(void)
{
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, PCONFIG_register(&test_platform_config2));
}

static void test_pconfig_get_board(void)
{
    const pconfig_platform_config_t *cfg = PCONFIG_GetBoard();
    TEST_ASSERT_NOT_NULL(cfg);
    TEST_ASSERT_EQUAL(0, OSAL_strcmp(cfg->product_name, "framework"));
}

static void test_pconfig_find_by_name(void)
{
    const pconfig_platform_config_t *cfg = PCONFIG_Find("ti", "framework", NULL);
    TEST_ASSERT_NOT_NULL(cfg);
    TEST_ASSERT_EQUAL(0, OSAL_strcmp(cfg->chip_name, "am625"));
}

static void test_pconfig_hw_get_mcu_by_id(void)
{
    const pconfig_platform_config_t *platform = PCONFIG_GetBoard();
    const pconfig_mcu_entry_t *mcu = PCONFIG_HW_GetMCU(platform, 0);
    TEST_ASSERT_NOT_NULL(mcu);
    TEST_ASSERT_EQUAL(0, OSAL_strcmp(mcu->config.name, "mcu0"));
}

static void test_pconfig_hw_get_mcu_second_entry(void)
{
    const pconfig_platform_config_t *platform = PCONFIG_GetBoard();
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
    TEST_ASSERT_TRUE(count >= 1);
}

static const test_case_t test_cases[] = {
    { .name = "test_pconfig_init_success", .func = test_pconfig_init_success, .setup = NULL, .teardown = NULL },
    { .name = "test_pconfig_register_success", .func = test_pconfig_register_success, .setup = NULL, .teardown = NULL },
    { .name = "test_pconfig_get_board", .func = test_pconfig_get_board, .setup = NULL, .teardown = NULL },
    { .name = "test_pconfig_find_by_name", .func = test_pconfig_find_by_name, .setup = NULL, .teardown = NULL },
    { .name = "test_pconfig_hw_get_mcu_by_id", .func = test_pconfig_hw_get_mcu_by_id, .setup = NULL, .teardown = NULL },
    { .name = "test_pconfig_hw_get_mcu_second_entry", .func = test_pconfig_hw_get_mcu_second_entry, .setup = NULL, .teardown = NULL },
    { .name = "test_pconfig_validate_success", .func = test_pconfig_validate_success, .setup = NULL, .teardown = NULL },
    { .name = "test_pconfig_list", .func = test_pconfig_list, .setup = NULL, .teardown = NULL },
};

static const test_suite_t test_suite = {
    .suite_name = "pconfig_api",
    .module_name = "pconfig",
    .layer_name = "UNIT",
    .cases = test_cases,
    .case_count = OSAL_sizeof(test_cases) / OSAL_sizeof(test_case_t),
    .suite_setup = suite_setup,
    .suite_teardown = suite_teardown,
    .metadata = {
        .category = TEST_CATEGORY_UNIT,
        .tags = TEST_TAG_FAST,
        .timeout_ms = 100,
        .description = "PCONFIG API unit tests"
    }
};

__attribute__((constructor))
static void register_pconfig_api_tests(void)
{
    libutest_register_suite(&test_suite);
}
