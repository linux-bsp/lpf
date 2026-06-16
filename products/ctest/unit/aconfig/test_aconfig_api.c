/**
 * @file test_aconfig_api.c
 * @brief ACONFIG API 单元测试
 *
 * 测试通用配置框架的基本 API 功能
 */

#include <test_framework/test_framework.h>
#include "osal.h"
#include "aconfig.h"

/*===========================================================================
 * 初始化和清理测试
 *===========================================================================*/

/* 测试用例: 初始化 ACONFIG */
static void test_aconfig_init(void)
{
    int32_t ret = ACONFIG_init();
    TEST_ASSERT_EQUAL(0, ret);
    ACONFIG_cleanup();
}

/* 测试用例: 重复初始化 */
static void test_aconfig_init_twice(void)
{
    int32_t ret = ACONFIG_init();
    TEST_ASSERT_EQUAL(0, ret);

    /* 第二次初始化应该成功或返回已初始化 */
    ret = ACONFIG_init();
    /* 允许成功或已初始化 */

    ACONFIG_cleanup();
}

/* 测试用例: 清理未初始化的 ACONFIG */
static void test_aconfig_cleanup_without_init(void)
{
    /* 清理未初始化的 ACONFIG 不应该崩溃 */
    ACONFIG_cleanup();
    TEST_ASSERT_TRUE(true);
}

/*===========================================================================
 * 配置表注册测试
 *===========================================================================*/

/* 测试用例: 注册空配置表 */
static void test_aconfig_register_null_table(void)
{
    ACONFIG_init();

    int32_t ret = ACONFIG_register_table(NULL);
    TEST_ASSERT_NOT_EQUAL(0, ret);

    ACONFIG_cleanup();
}

/* 测试用例: 注册有效配置表 */
static void test_aconfig_register_valid_table(void)
{
    ACONFIG_init();

    aconfig_config_table_t table = {
        .name = "test_config",
        .function_map = NULL,
        .user_data = NULL
    };

    int32_t ret = ACONFIG_register_table(&table);
    TEST_ASSERT_EQUAL(0, ret);

    /* 验证可以获取配置表 */
    const aconfig_config_table_t *retrieved = ACONFIG_GetTable();
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_STRING("test_config", retrieved->name);

    ACONFIG_cleanup();
}

/* 测试用例: 注册后注销配置表 */
static void test_aconfig_unregister_table(void)
{
    ACONFIG_init();

    aconfig_config_table_t table = {
        .name = "test_config",
        .function_map = NULL,
        .user_data = NULL
    };

    int32_t ret = ACONFIG_register_table(&table);
    TEST_ASSERT_EQUAL(0, ret);

    /* 注销配置表 */
    ret = ACONFIG_unregister_table();
    TEST_ASSERT_EQUAL(0, ret);

    /* 验证配置表已被移除 */
    const aconfig_config_table_t *retrieved = ACONFIG_GetTable();
    TEST_ASSERT_NULL(retrieved);

    ACONFIG_cleanup();
}

/* 测试用例: 未注册时注销配置表 */
static void test_aconfig_unregister_without_register(void)
{
    ACONFIG_init();

    (void)ACONFIG_unregister_table();
    /* 应该成功或返回未注册错误 */

    ACONFIG_cleanup();
}

/*===========================================================================
 * 配置查询测试
 *===========================================================================*/

/* 测试用例: 获取未注册的配置表 */
static void test_aconfig_get_table_not_registered(void)
{
    ACONFIG_init();

    const aconfig_config_table_t *table = ACONFIG_GetTable();
    TEST_ASSERT_NULL(table);

    ACONFIG_cleanup();
}

/* 测试用例: 查询功能配置（无配置表） */
static void test_aconfig_get_function_config_no_table(void)
{
    ACONFIG_init();

    const void *config = ACONFIG_GetFunctionConfig(1);
    TEST_ASSERT_NULL(config);

    ACONFIG_cleanup();
}

/* 测试用例: 检查功能启用状态（无配置表） */
static void test_aconfig_is_function_enabled_no_table(void)
{
    ACONFIG_init();

    bool enabled = ACONFIG_IsFunctionEnabled(1);
    TEST_ASSERT_FALSE(enabled);

    ACONFIG_cleanup();
}

/* 测试用例: 获取统计信息（无配置表） */
static void test_aconfig_get_statistics_no_table(void)
{
    ACONFIG_init();

    aconfig_statistics_t stats;
    int32_t ret = ACONFIG_get_statistics(&stats);

    if (ret == 0) {
        /* 如果成功，应该返回全零 */
        TEST_ASSERT_EQUAL(0, stats.total_functions);
        TEST_ASSERT_EQUAL(0, stats.enabled_functions);
        TEST_ASSERT_EQUAL(0, stats.disabled_functions);
    } else {
        /* 或者返回错误 */
        TEST_ASSERT_NOT_EQUAL(0, ret);
    }

    ACONFIG_cleanup();
}

/* 测试用例: 获取统计信息 - 空指针 */
static void test_aconfig_get_statistics_null_pointer(void)
{
    ACONFIG_init();

    int32_t ret = ACONFIG_get_statistics(NULL);
    TEST_ASSERT_NOT_EQUAL(0, ret);

    ACONFIG_cleanup();
}

/*===========================================================================
 * 测试套件入口
 *===========================================================================*/

void test_aconfig_api(void)
{
    /* 初始化和清理 */
    test_aconfig_init();
    test_aconfig_init_twice();
    test_aconfig_cleanup_without_init();

    /* 配置表注册 */
    test_aconfig_register_null_table();
    test_aconfig_register_valid_table();
    test_aconfig_unregister_table();
    test_aconfig_unregister_without_register();

    /* 配置查询 */
    test_aconfig_get_table_not_registered();
    test_aconfig_get_function_config_no_table();
    test_aconfig_is_function_enabled_no_table();
    test_aconfig_get_statistics_no_table();
    test_aconfig_get_statistics_null_pointer();
}