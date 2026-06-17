/**
 * @file test_pconfig_api.c
 * @brief PCONFIG API comprehensive unit tests
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
    TEST_ASSERT_NOT_EQUAL(NULL, cfg);
    TEST_ASSERT_EQUAL(0, OSAL_strcmp(cfg->product_name, "framework"));
}

static void test_pconfig_find_by_name(void)
{
    const pconfig_platform_config_t *cfg = PCONFIG_Find("ti", "framework", NULL);
    TEST_ASSERT_NOT_EQUAL(NULL, cfg);
    TEST_ASSERT_EQUAL(0, OSAL_strcmp(cfg->chip_name, "am625"));
}

static void test_pconfig_hw_get_mcu_by_id(void)
{
    const pconfig_platform_config_t *platform = PCONFIG_GetBoard();
    const pconfig_mcu_entry_t *mcu = PCONFIG_HW_GetMCU(platform, 0);
    TEST_ASSERT_NOT_EQUAL(NULL, mcu);
    TEST_ASSERT_EQUAL(0, OSAL_strcmp(mcu->config.name, "mcu0"));
}

static void test_pconfig_hw_get_mcu_second_entry(void)
{
    const pconfig_platform_config_t *platform = PCONFIG_GetBoard();
    const pconfig_mcu_entry_t *mcu = PCONFIG_HW_GetMCU(platform, 1);
    TEST_ASSERT_NOT_EQUAL(NULL, mcu);
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

int main(void)
{
    test_framework_init();
    TEST_SUITE("pconfig_api", suite_setup, suite_teardown);
    TEST_CASE(test_pconfig_init_success);
    TEST_CASE(test_pconfig_register_success);
    TEST_CASE(test_pconfig_get_board);
    TEST_CASE(test_pconfig_find_by_name);
    TEST_CASE(test_pconfig_hw_get_mcu_by_id);
    TEST_CASE(test_pconfig_hw_get_mcu_second_entry);
    TEST_CASE(test_pconfig_validate_success);
    TEST_CASE(test_pconfig_list);
    return test_framework_run();
}
