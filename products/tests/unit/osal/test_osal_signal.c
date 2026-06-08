#include "test_framework.h"
/**
 * @file test_signal.c
 * @brief OSAL信号处理单元测试
 *
 * 使用新的libtest框架，测试自动注册
 */

#include "osal.h"

static volatile int32_t g_signal_received = 0;
static volatile int32_t g_signal_number = 0;

/* 信号处理函数 */
static void test_signal_handler(int32_t signum)
{
    g_signal_received = 1;
    g_signal_number = signum;
}

/* 测试用例1: 注册信号处理函数 */
static void test_signal_register_success(void)
{
    g_signal_received = 0;
    g_signal_number = 0;

    /* 注册SIGUSR1处理函数（使用用户信号避免中断程序） */
    int32_t ret = OSAL_SignalRegister(OS_SIGNAL_USR1, test_signal_handler);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    /* 发送信号给自己 */
    OSAL_Kill(OSAL_Getpid(), SIGUSR1);

    /* 等待信号处理 */
    OSAL_msleep(100);

    TEST_ASSERT_EQUAL(1, g_signal_received);
    TEST_ASSERT_EQUAL(SIGUSR1, g_signal_number);

    /* 恢复默认 */
    OSAL_SignalIgnore(OS_SIGNAL_USR1);
}

/* 测试用例2: 忽略信号 */
static void test_signal_ignore_success(void)
{
    g_signal_received = 0;
    g_signal_number = 0;

    /* 先注册处理函数 */
    int32_t ret = OSAL_SignalRegister(OS_SIGNAL_USR2, test_signal_handler);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    /* 然后忽略信号 */
    ret = OSAL_SignalIgnore(OS_SIGNAL_USR2);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    /* 发送信号 */
    OSAL_Kill(OSAL_Getpid(), SIGUSR2);
    OSAL_usleep(100000);

    /* 信号应该被忽略，处理函数不应该被调用 */
    TEST_ASSERT_EQUAL(0, g_signal_received);
}

/* 测试用例3: 阻塞信号 */
static void test_signal_block_success(void)
{
    g_signal_received = 0;
    g_signal_number = 0;

    /* 阻塞SIGUSR1 */
    int32_t ret = OSAL_SignalBlock(OS_SIGNAL_USR1);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    /* 注册处理函数 */
    ret = OSAL_SignalRegister(OS_SIGNAL_USR1, test_signal_handler);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    /* 发送信号 */
    OSAL_Kill(OSAL_Getpid(), SIGUSR1);
    OSAL_usleep(100000);

    /* 信号被阻塞，处理函数不应该被调用 */
    TEST_ASSERT_EQUAL(0, g_signal_received);

    /* 解除阻塞 */
    ret = OSAL_SignalUnblock(OS_SIGNAL_USR1);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    /* 等待信号处理 */
    OSAL_usleep(100000);

    /* 现在信号应该被处理 */
    TEST_ASSERT_EQUAL(1, g_signal_received);

    /* 恢复默认 */
    OSAL_SignalIgnore(OS_SIGNAL_USR1);
}

/* 测试用例4: 恢复默认信号处理 */
static void test_signal_default_success(void)
{
    g_signal_received = 0;
    g_signal_number = 0;

    /* 先注册处理函数 */
    int32_t ret = OSAL_SignalRegister(OS_SIGNAL_USR1, test_signal_handler);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    /* 恢复默认处理 */
    ret = OSAL_SignalDefault(OS_SIGNAL_USR1);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    /* SIGUSR1的默认行为是终止进程，但我们只验证API调用成功 */
    /* 不实际发送信号以避免终止测试进程 */
}

/* 测试用例5: 多个信号处理 */
static void test_signal_register_multiple(void)
{
    g_signal_received = 0;
    g_signal_number = 0;

    /* 注册多个信号（都使用用户信号） */
    int32_t ret = OSAL_SignalRegister(OS_SIGNAL_USR1, test_signal_handler);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    ret = OSAL_SignalRegister(OS_SIGNAL_USR2, test_signal_handler);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    /* 发送SIGUSR1 */
    OSAL_Kill(OSAL_Getpid(), SIGUSR1);
    OSAL_usleep(100000);

    TEST_ASSERT_EQUAL(1, g_signal_received);
    TEST_ASSERT_EQUAL(SIGUSR1, g_signal_number);

    /* 恢复默认 */
    OSAL_SignalIgnore(OS_SIGNAL_USR1);
    OSAL_SignalIgnore(OS_SIGNAL_USR2);
}

/* 测试用例6: 无效参数测试 */
static void test_signal_register_invalidparams(void)
{
    /* NULL处理函数 */
    int32_t ret = OSAL_SignalRegister(OS_SIGNAL_USR1, NULL);
    TEST_ASSERT_EQUAL(OSAL_ERR_INVALID_POINTER, ret);
}

/* 注册测试套件 - 自动注册 */

/* 测试用例数组 - 使用函数指针数组 */
static const test_case_t test_cases[] = {
	{
		.name = "test_signal_register_success",
		.func = test_signal_register_success,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_signal_ignore_success",
		.func = test_signal_ignore_success,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_signal_block_success",
		.func = test_signal_block_success,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_signal_default_success",
		.func = test_signal_default_success,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_signal_register_multiple",
		.func = test_signal_register_multiple,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_signal_register_invalidparams",
		.func = test_signal_register_invalidparams,
		.setup = NULL,
		.teardown = NULL
	},
};

/* 测试套件定义 */
static const test_suite_t test_suite = {
	.suite_name = "osal_signal",
	.module_name = "osal_signal",
	.layer_name = "OSAL",
	.cases = test_cases,
	.case_count = sizeof(test_cases) / sizeof(test_case_t),
	.suite_setup = NULL,
	.suite_teardown = NULL,
	.metadata = {
		.category = TEST_CATEGORY_UNIT,
		.tags = TEST_TAG_FAST,
		.timeout_ms = 100,
		.description = "OSAL osal_signal tests"
	}
};

/* 测试套件注册函数 */
__attribute__((constructor))
static void register_osal_signal_tests(void)
{
	libutest_register_suite(&test_suite);
}
