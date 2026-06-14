/************************************************************************
 * OSAL线程管理单元测试
 ************************************************************************/

#include "test_framework.h"
#include "osal.h"

/*===========================================================================
 * 测试辅助变量
 *===========================================================================*/

static volatile int32_t thread_counter = 0;

/*===========================================================================
 * 测试线程函数
 *===========================================================================*/

static void* simple_thread_func(void *arg)
{
    int32_t *value = (int32_t *)arg;
    if (value) {
        *value = 42;
    }
    return (void *)123;
}

static void* counter_thread_func(void *arg)
{
    (void)arg;
    thread_counter++;
    OSAL_msleep(100);
    thread_counter++;
    return NULL;
}

static void* sleep_thread_func(void *arg)
{
    int32_t sleep_ms = *(int32_t *)arg;
    OSAL_msleep(sleep_ms);
    return NULL;
}

/*===========================================================================
 * 测试用例
 *===========================================================================*/

static void test_thread_create_join(void)
{
    osal_thread_t thread;
    int32_t value = 0;
    void *retval = NULL;

    int32_t ret = OSAL_pthread_create(&thread, NULL, simple_thread_func, &value);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    ret = OSAL_pthread_join(thread, NULL);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
    TEST_ASSERT_EQUAL(42, value);
    TEST_ASSERT_EQUAL(123, (int32_t)(intptr_t)retval);
}

static void test_thread_create_simplified(void)
{
    osal_thread_t thread;
    int32_t value = 0;

    int32_t ret = OSAL_pthread_create(&thread, NULL, simple_thread_func, &value);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    ret = OSAL_pthread_join(thread, NULL);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
    TEST_ASSERT_EQUAL(42, value);
}

static void test_thread_multiple_threads(void)
{
    uint32_t i;
    osal_thread_t threads[5];
    int32_t values[5] = {0};

    /* 创建多个线程 */

    for (i = 0; i < 5; i++) {
        int32_t ret = OSAL_pthread_create(&threads[i], NULL, simple_thread_func, &values[i]);
        TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
    }

    /* 等待所有线程完成 */

    for (i = 0; i < 5; i++) {
        int32_t ret = OSAL_pthread_join(threads[i], NULL);
        TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
        TEST_ASSERT_EQUAL(42, values[i]);
    }
}

static void test_thread_counter(void)
{
    osal_thread_t thread;
    thread_counter = 0;

    int32_t ret = OSAL_pthread_create(&thread, NULL, counter_thread_func, NULL);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    /* 等待线程完成 */
    ret = OSAL_pthread_join(thread, NULL);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
    TEST_ASSERT_EQUAL(2, thread_counter);
}

static void test_thread_concurrent_counter(void)
{
    uint32_t i;
    osal_thread_t threads[10];
    thread_counter = 0;

    /* 创建10个线程，每个增加计数器2次 */

    for (i = 0; i < 10; i++) {
        int32_t ret = OSAL_pthread_create(&threads[i], NULL, counter_thread_func, NULL);
        TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
    }

    /* 等待所有线程完成 */

    for (i = 0; i < 10; i++) {
        int32_t ret = OSAL_pthread_join(threads[i], NULL);
        TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
    }

    /* 验证计数器值（注意：没有同步，可能有竞态） */
    TEST_ASSERT_EQUAL(20, thread_counter);
}

static void test_thread_null_params(void)
{
    osal_thread_t thread;

    /* NULL线程指针 */
    int32_t ret = OSAL_pthread_create(NULL, NULL, simple_thread_func, NULL);
    TEST_ASSERT_NOT_EQUAL(OSAL_SUCCESS, ret);

    /* NULL函数指针 */
    ret = OSAL_pthread_create(&thread, NULL, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(OSAL_SUCCESS, ret);
}

static void test_thread_with_null_arg(void)
{
    osal_thread_t thread;

    /* 线程函数可以接受NULL参数 */
    int32_t ret = OSAL_pthread_create(&thread, NULL, simple_thread_func, NULL);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    ret = OSAL_pthread_join(thread, NULL);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
}

static void test_thread_timing(void)
{
    uint32_t i;
    osal_thread_t threads[3];
    int32_t sleep_times[3] = {50, 100, 150};
    uint64_t start_time, end_time;

    start_time = OSAL_get_tick_count();

    /* 创建3个不同睡眠时间的线程 */

    for (i = 0; i < 3; i++) {
        int32_t ret = OSAL_pthread_create(&threads[i], NULL, sleep_thread_func, &sleep_times[i]);
        TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
    }

    /* 等待所有线程完成 */

    for (i = 0; i < 3; i++) {
        int32_t ret = OSAL_pthread_join(threads[i], NULL);
        TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
    }

    end_time = OSAL_get_tick_count();
    uint64_t elapsed = end_time - start_time;

    /* 总时间应该接近最长的睡眠时间（150ms），因为线程并发执行 */
    TEST_ASSERT_TRUE(elapsed >= 150 && elapsed < 250);
}

static void test_thread_sequential_execution(void)
{
    osal_thread_t thread1, thread2;
    int32_t value1 = 0, value2 = 0;

    /* 顺序创建和等待线程 */
    int32_t ret = OSAL_pthread_create(&thread1, NULL, simple_thread_func, &value1);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    ret = OSAL_pthread_join(thread1, NULL);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
    TEST_ASSERT_EQUAL(42, value1);

    ret = OSAL_pthread_create(&thread2, NULL, simple_thread_func, &value2);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    ret = OSAL_pthread_join(thread2, NULL);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
    TEST_ASSERT_EQUAL(42, value2);
}

/*===========================================================================
 * 测试模块注册
 *===========================================================================*/

/* 测试用例数组 - 使用函数指针数组 */
static const test_case_t test_cases[] = {
	{
		.name = "test_thread_create_join",
		.func = test_thread_create_join,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_thread_create_simplified",
		.func = test_thread_create_simplified,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_thread_multiple_threads",
		.func = test_thread_multiple_threads,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_thread_counter",
		.func = test_thread_counter,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_thread_concurrent_counter",
		.func = test_thread_concurrent_counter,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_thread_null_params",
		.func = test_thread_null_params,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_thread_with_null_arg",
		.func = test_thread_with_null_arg,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_thread_timing",
		.func = test_thread_timing,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_thread_sequential_execution",
		.func = test_thread_sequential_execution,
		.setup = NULL,
		.teardown = NULL
	},
};

/* 测试套件定义 */
static const test_suite_t test_suite = {
	.suite_name = "osal_thread",
	.module_name = "osal_thread",
	.layer_name = "OSAL",
	.cases = test_cases,
	.case_count = OSAL_sizeof(test_cases) / OSAL_sizeof(test_case_t),
	.suite_setup = NULL,
	.suite_teardown = NULL,
	.metadata = {
		.category = TEST_CATEGORY_UNIT,
		.tags = TEST_TAG_FAST,
		.timeout_ms = 100,
		.description = "OSAL osal_thread tests"
	}
};

/* 测试套件注册函数 */
__attribute__((constructor))
static void register_osal_thread_tests(void)
{
	libutest_register_suite(&test_suite);
}
