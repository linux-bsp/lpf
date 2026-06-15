/**
 * @file test_aconfig_api.c
 * @brief ACONFIG API 单元测试（优化版）
 * @note 适配优化后的 ACONFIG 数据结构
 */

#include "test_framework.h"
#include "aconfig.h"
#include "ccm_tc_functions.h"
#include "ccm_tm_functions.h"

/* 失效映射数据 */
static const uint32_t power_on_affected[] = {CCM_TM_POWER_STATUS};
static const uint32_t power_off_affected[] = {CCM_TM_POWER_STATUS};

/* 测试用的遥控配置（稀疏数组） */
static const aconfig_tc_entry_t test_tc_entries[] = {
    {
        .function_id = CCM_TC_POWER_ON,
        .config = {
            .function_id = CCM_TC_POWER_ON,
            .device = {.type = PCONFIG_DEVICE_MCU, .index = 0},
            .invalidated_tm_ids = power_on_affected,
            .invalidated_tm_count = 1,
            .enabled = true,
            .user_context = NULL
        }
    },
    {
        .function_id = CCM_TC_POWER_OFF,
        .config = {
            .function_id = CCM_TC_POWER_OFF,
            .device = {.type = PCONFIG_DEVICE_MCU, .index = 0},
            .invalidated_tm_ids = power_off_affected,
            .invalidated_tm_count = 1,
            .enabled = true,
            .user_context = NULL
        }
    },
    {
        .function_id = CCM_TC_MCU_RESET,
        .config = {
            .function_id = CCM_TC_MCU_RESET,
            .device = {.type = PCONFIG_DEVICE_MCU, .index = 0},
            .invalidated_tm_ids = NULL,
            .invalidated_tm_count = 0,
            .enabled = false,  /* 禁用 */
            .user_context = NULL
        }
    }
};

/* 测试用的遥测配置（稀疏数组） */
static const aconfig_tm_entry_t test_tm_entries[] = {
    {
        .function_id = CCM_TM_VOLTAGE_5V,
        .config = {
            .function_id = CCM_TM_VOLTAGE_5V,
            .device = {.type = PCONFIG_DEVICE_MCU, .index = 0},
            .poll_period_ms = 500,
            .validity_period_ms = 1000,
            .enabled = true,
            .user_context = NULL
        }
    },
    {
        .function_id = CCM_TM_CURRENT,
        .config = {
            .function_id = CCM_TM_CURRENT,
            .device = {.type = PCONFIG_DEVICE_MCU, .index = 0},
            .poll_period_ms = 500,
            .validity_period_ms = 1000,
            .enabled = true,
            .user_context = NULL
        }
    },
    {
        .function_id = CCM_TM_CPU_TEMP,
        .config = {
            .function_id = CCM_TM_CPU_TEMP,
            .device = {.type = PCONFIG_DEVICE_SENSOR, .index = 0},
            .poll_period_ms = 1000,
            .validity_period_ms = 2000,
            .enabled = false,  /* 禁用 */
            .user_context = NULL
        }
    }
};

/* 测试配置表 */
static const aconfig_config_table_t test_config_table = {
    .name = "test_config",
    .hwid_count = 0,
    .hwid_list = NULL,
    .tc_entries = test_tc_entries,
    .tc_count = sizeof(test_tc_entries) / sizeof(test_tc_entries[0]),
    .tm_entries = test_tm_entries,
    .tm_count = sizeof(test_tm_entries) / sizeof(test_tm_entries[0])
};

/**
 * @brief 测试初始化和注册
 */
static void test_aconfig_init_register(void)
{
    int32_t ret;

    /* 初始化 */
    ret = ACONFIG_Init();
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    /* 注册配置表 */
    ret = ACONFIG_RegisterTable(&test_config_table);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    /* 重复注册应该给出警告但成功 */
    ret = ACONFIG_RegisterTable(&test_config_table);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    /* NULL 指针应该失败 */
    ret = ACONFIG_RegisterTable(NULL);
    TEST_ASSERT_EQUAL(OSAL_ERR_INVALID_POINTER, ret);
}

/**
 * @brief 测试遥控配置查询
 */
static void test_aconfig_tc_query(void)
{
    const aconfig_tc_config_t *config;

    /* 查询启用的遥控功能 */
    config = ACONFIG_GetTcConfig(CCM_TC_POWER_ON);
    TEST_ASSERT_NOT_NULL(config);
    TEST_ASSERT_EQUAL(CCM_TC_POWER_ON, config->function_id);
    TEST_ASSERT_EQUAL(PCONFIG_DEVICE_MCU, config->device.type);
    TEST_ASSERT_EQUAL(0, config->device.index);
    TEST_ASSERT_TRUE(config->enabled);

    /* 查询禁用的遥控功能 */
    config = ACONFIG_GetTcConfig(CCM_TC_MCU_RESET);
    TEST_ASSERT_NOT_NULL(config);
    TEST_ASSERT_FALSE(config->enabled);

    /* 查询不存在的功能 */
    config = ACONFIG_GetTcConfig(0xFFFF);
    TEST_ASSERT_NULL(config);
}

/**
 * @brief 测试遥测配置查询
 */
static void test_aconfig_tm_query(void)
{
    const aconfig_tm_config_t *config;

    /* 查询启用的遥测功能 */
    config = ACONFIG_GetTmConfig(CCM_TM_VOLTAGE_5V);
    TEST_ASSERT_NOT_NULL(config);
    TEST_ASSERT_EQUAL(CCM_TM_VOLTAGE_5V, config->function_id);
    TEST_ASSERT_EQUAL(PCONFIG_DEVICE_MCU, config->device.type);
    TEST_ASSERT_EQUAL(0, config->device.index);
    TEST_ASSERT_EQUAL(500, config->poll_period_ms);
    TEST_ASSERT_EQUAL(1000, config->validity_period_ms);
    TEST_ASSERT_TRUE(config->enabled);

    /* 查询禁用的遥测功能 */
    config = ACONFIG_GetTmConfig(CCM_TM_CPU_TEMP);
    TEST_ASSERT_NOT_NULL(config);
    TEST_ASSERT_FALSE(config->enabled);

    /* 查询不存在的功能 */
    config = ACONFIG_GetTmConfig(0xFFFF);
    TEST_ASSERT_NULL(config);
}

/**
 * @brief 测试功能使能检查
 */
static void test_aconfig_enabled_check(void)
{
    /* 遥控功能使能检查 */
    TEST_ASSERT_TRUE(ACONFIG_IsTcEnabled(CCM_TC_POWER_ON));
    TEST_ASSERT_TRUE(ACONFIG_IsTcEnabled(CCM_TC_POWER_OFF));
    TEST_ASSERT_FALSE(ACONFIG_IsTcEnabled(CCM_TC_MCU_RESET));
    TEST_ASSERT_FALSE(ACONFIG_IsTcEnabled(0xFFFF));

    /* 遥测功能使能检查 */
    TEST_ASSERT_TRUE(ACONFIG_IsTmEnabled(CCM_TM_VOLTAGE_5V));
    TEST_ASSERT_TRUE(ACONFIG_IsTmEnabled(CCM_TM_CURRENT));
    TEST_ASSERT_FALSE(ACONFIG_IsTmEnabled(CCM_TM_CPU_TEMP));
    TEST_ASSERT_FALSE(ACONFIG_IsTmEnabled(0xFFFF));
}

/**
 * @brief 测试失效映射（内嵌版）
 */
static void test_aconfig_invalidation_map(void)
{
    uint32_t affected_ids[16];
    uint32_t actual_count;
    int32_t ret;

    /* 查询 POWER_ON 的失效映射 */
    ret = ACONFIG_GetInvalidationMap(CCM_TC_POWER_ON, affected_ids, 16, &actual_count);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
    TEST_ASSERT_EQUAL(1, actual_count);
    TEST_ASSERT_EQUAL(CCM_TM_POWER_STATUS, affected_ids[0]);

    /* 查询没有失效映射的遥控功能 */
    ret = ACONFIG_GetInvalidationMap(CCM_TC_MCU_RESET, affected_ids, 16, &actual_count);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
    TEST_ASSERT_EQUAL(0, actual_count);

    /* 查询不存在的功能 */
    ret = ACONFIG_GetInvalidationMap(0xFFFF, affected_ids, 16, &actual_count);
    TEST_ASSERT_EQUAL(OSAL_ERR_NAME_NOT_FOUND, ret);

    /* NULL 指针测试 */
    ret = ACONFIG_GetInvalidationMap(CCM_TC_POWER_ON, NULL, 16, &actual_count);
    TEST_ASSERT_EQUAL(OSAL_ERR_INVALID_POINTER, ret);

    ret = ACONFIG_GetInvalidationMap(CCM_TC_POWER_ON, affected_ids, 16, NULL);
    TEST_ASSERT_EQUAL(OSAL_ERR_INVALID_POINTER, ret);
}

/**
 * @brief 测试统计信息
 */
static void test_aconfig_statistics(void)
{
    aconfig_statistics_t stats;
    int32_t ret;

    ret = ACONFIG_GetStatistics(&stats);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    /* 验证统计信息 */
    TEST_ASSERT_EQUAL(2, stats.tc_enabled_count);   /* POWER_ON, POWER_OFF */
    TEST_ASSERT_EQUAL(1, stats.tc_disabled_count);  /* MCU_RESET */
    TEST_ASSERT_EQUAL(2, stats.tm_enabled_count);   /* VOLTAGE_5V, CURRENT */
    TEST_ASSERT_EQUAL(1, stats.tm_disabled_count);  /* CPU_TEMP */
    TEST_ASSERT_EQUAL(2, stats.total_invalidation_maps);  /* POWER_ON, POWER_OFF */

    /* NULL 指针测试 */
    ret = ACONFIG_GetStatistics(NULL);
    TEST_ASSERT_EQUAL(OSAL_ERR_INVALID_POINTER, ret);
}

/**
 * @brief 测试配置打印
 */
static void test_aconfig_print(void)
{
    /* 只测试不崩溃 */
    ACONFIG_PrintConfig();
}

/**
 * @brief ACONFIG 测试套件
 */
void run_aconfig_tests(void)
{
    TEST_RUN(test_aconfig_init_register);
    TEST_RUN(test_aconfig_tc_query);
    TEST_RUN(test_aconfig_tm_query);
    TEST_RUN(test_aconfig_enabled_check);
    TEST_RUN(test_aconfig_invalidation_map);
    TEST_RUN(test_aconfig_statistics);
    TEST_RUN(test_aconfig_print);
}
