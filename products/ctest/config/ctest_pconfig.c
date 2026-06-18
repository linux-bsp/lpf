/**
 * @file ctest_pconfig.c
 * @brief Static PCONFIG data for the ctest product.
 */

#include "osal.h"
#include "pconfig/pconfig.h"

#define CTEST_SERIAL_DEVICE "/tmp/es_middleware_ctest_mcu0"

static const pconfig_mcu_entry_t ctest_system_mcu_entries[] = {
    {
        .description = "PTY-backed MCU",
        .enabled = true,
        .config = {
            .name = "mcu0",
            .interface = PCONFIG_MCU_INTERFACE_SERIAL,
            .hw.serial = {
                .device = CTEST_SERIAL_DEVICE,
                .baudrate = 115200,
                .data_bits = 8,
                .stop_bits = 1,
                .parity = PCONFIG_MCU_PARITY_NONE,
                .flow_control = PCONFIG_MCU_FLOW_NONE
            },
            .cmd_timeout_ms = 2000,
            .retry_count = 0
        },
        .reset_gpio = NULL,
        .irq_gpio = NULL
    }
};

static const pconfig_mcu_entry_t ctest_unit_mcu_entries[] = {
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

static const pconfig_platform_config_t ctest_system_platform_config = {
    .platform_name = "generic",
    .chip_name = "framework",
    .project_name = "ctest",
    .product_name = "middleware",
    .mcu_count = 1,
    .mcu_array = ctest_system_mcu_entries
};

static const pconfig_platform_config_t ctest_unit_platform_config = {
    .platform_name = "ti",
    .chip_name = "am625",
    .project_name = "test_project",
    .product_name = "framework",
    .mcu_count = 2,
    .mcu_array = ctest_unit_mcu_entries
};

static const pconfig_platform_config_t ctest_unit_alt_platform_config = {
    .platform_name = "ti",
    .chip_name = "am625",
    .project_name = "test_project",
    .product_name = "framework_alt",
    .mcu_count = 0,
    .mcu_array = NULL
};

static const pconfig_platform_config_t *const ctest_platform_configs[] = {
    &ctest_system_platform_config,
    &ctest_unit_platform_config,
    &ctest_unit_alt_platform_config
};

const pconfig_platform_table_t g_pconfig_platform_table = {
    .configs = ctest_platform_configs,
    .count = OSAL_sizeof(ctest_platform_configs) / OSAL_sizeof(ctest_platform_configs[0]),
    .current_index = 0
};
