#include "test_framework.h"
/**
 * @file test_osal_semaphore.c
 * @brief OSAL信号量单元测试
 *
 * 使用新的libtest框架，测试自动注册
 */

#include "osal.h"

static int32_t shared_counter = 0;

/* 测试用例1: 信号量创建成功 */
static void test_semaphore_create_success(void)
{
    osal_semaphore_t *sem = NULL;

    int32_t ret = OSAL_SemaphoreCreate(&sem, 1);

    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
    TEST_ASSERT_NOT_EQUAL(NULL, sem);

    OSAL_SemaphoreDelete(sem);
}

/* 测试用例2: 信号量创建失败 - 空指针 */
static void test_semaphore_create_nullpointer(void)
{
    int32_t ret = OSAL_SemaphoreCreate(NULL, 1);
    TEST_ASSERT_EQUAL(OSAL_ERR_INVALID_POINTER, ret);
}

/* 测试用例3: 信号量创建失败 - 初始值过大 */
static void test_semaphore_create_invalid_value(void)
{
    osal_semaphore_t *sem = NULL;
    int32_t ret = OSAL_SemaphoreCreate(&sem, (uint32_t)INT32_MAX + 1);
    TEST_ASSERT_EQUAL(OSAL_ERR_INVALID_SEM_VALUE, ret);
}

/* 测试用例4: 信号量等待和释放 */
static void test_semaphore_wait_post_success(void)
{
    osal_semaphore_t *sem = NULL;
    OSAL_SemaphoreCreate(&sem, 1);

    int32_t ret = OSAL_SemaphoreWait(sem);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    ret = OSAL_SemaphorePost(sem);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    OSAL_SemaphoreDelete(sem);
}

/* 测试用例5: 信号量等待失败 - 空指针 */
static void test_semaphore_wait_nullpointer(void)
{
    int32_t ret = OSAL_SemaphoreWait(NULL);
    TEST_ASSERT_EQUAL(OSAL_ERR_INVALID_POINTER, ret);
}

/* 测试用例6: 信号量释放失败 - 空指针 */
static void test_semaphore_post_nullpointer(void)
{
    int32_t ret = OSAL_SemaphorePost(NULL);
    TEST_ASSERT_EQUAL(OSAL_ERR_INVALID_POINTER, ret);
}

/* 测试用例7: 信号量超时等待 - 超时 */
static void test_semaphore_timedwait_timeout(void)
{
    osal_semaphore_t *sem = NULL;
    OSAL_SemaphoreCreate(&sem, 0);

    int32_t ret = OSAL_SemaphoreTimedWait(sem, 100);
    TEST_ASSERT_EQUAL(OSAL_ERR_TIMEOUT, ret);

    OSAL_SemaphoreDelete(sem);
}

/* 测试用例8: 信号量超时等待 - 成功 */
static void test_semaphore_timedwait_success(void)
{
    osal_semaphore_t *sem = NULL;
    OSAL_SemaphoreCreate(&sem, 1);

    int32_t ret = OSAL_SemaphoreTimedWait(sem, 100);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    OSAL_SemaphoreDelete(sem);
}

/* 测试用例9: 信号量非阻塞等待 - 超时 */
static void test_semaphore_trywait_timeout(void)
{
    osal_semaphore_t *sem = NULL;
    OSAL_SemaphoreCreate(&sem, 0);

    int32_t ret = OSAL_SemaphoreTimedWait(sem, 0);
    TEST_ASSERT_EQUAL(OSAL_ERR_TIMEOUT, ret);

    OSAL_SemaphoreDelete(sem);
}

/* 测试用例10: 信号量非阻塞等待 - 成功 */
static void test_semaphore_trywait_success(void)
{
    osal_semaphore_t *sem = NULL;
    OSAL_SemaphoreCreate(&sem, 1);

    int32_t ret = OSAL_SemaphoreTimedWait(sem, 0);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    OSAL_SemaphoreDelete(sem);
}

/* 测试用例11: 信号量删除 */
static void test_semaphore_delete_success(void)
{
    osal_semaphore_t *sem = NULL;
    OSAL_SemaphoreCreate(&sem, 1);

    int32_t ret = OSAL_SemaphoreDelete(sem);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
}

/* 测试用例12: 信号量删除失败 - 空指针 */
static void test_semaphore_delete_nullpointer(void)
{
    int32_t ret = OSAL_SemaphoreDelete(NULL);
    TEST_ASSERT_EQUAL(OSAL_ERR_INVALID_POINTER, ret);
}

/* 生产者线程 */
static void* producer_thread(void *arg)
{
    osal_semaphore_t *sem = (osal_semaphore_t *)arg;

    int32_t i;

    for (i = 0; i < 10; i++) {
        OSAL_msleep(10);
        shared_counter++;
        OSAL_SemaphorePost(sem);
    }

    return NULL;
}

/* 消费者线程 */
static void* consumer_thread(void *arg)
{
    osal_semaphore_t *sem = (osal_semaphore_t *)arg;

    int32_t i;

    for (i = 0; i < 10; i++) {
        OSAL_SemaphoreWait(sem);
        shared_counter--;
    }

    return NULL;
}

/* 测试用例13: 信号量生产者-消费者模式 */
static void test_semaphore_producer_consumer(void)
{
    shared_counter = 0;
    osal_semaphore_t *sem = NULL;
    OSAL_SemaphoreCreate(&sem, 0);

    osal_thread_t producer, consumer;

    /* 创建生产者和消费者线程 */
    OSAL_ThreadCreate(&producer, producer_thread, sem);
    OSAL_ThreadCreate(&consumer, consumer_thread, sem);

    /* 等待线程完成 */
    OSAL_ThreadJoin(producer);
    OSAL_ThreadJoin(consumer);

    /* 验证计数器归零 */
    TEST_ASSERT_EQUAL(0, shared_counter);

    OSAL_SemaphoreDelete(sem);
}

/* 注册测试套件 - 自动注册 */

/* 测试用例数组 - 使用函数指针数组 */
static const test_case_t test_cases[] = {
	{
		.name = "test_semaphore_create_success",
		.func = test_semaphore_create_success,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_semaphore_create_nullpointer",
		.func = test_semaphore_create_nullpointer,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_semaphore_create_invalid_value",
		.func = test_semaphore_create_invalid_value,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_semaphore_wait_post_success",
		.func = test_semaphore_wait_post_success,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_semaphore_wait_nullpointer",
		.func = test_semaphore_wait_nullpointer,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_semaphore_post_nullpointer",
		.func = test_semaphore_post_nullpointer,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_semaphore_timedwait_timeout",
		.func = test_semaphore_timedwait_timeout,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_semaphore_timedwait_success",
		.func = test_semaphore_timedwait_success,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_semaphore_trywait_timeout",
		.func = test_semaphore_trywait_timeout,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_semaphore_trywait_success",
		.func = test_semaphore_trywait_success,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_semaphore_delete_success",
		.func = test_semaphore_delete_success,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_semaphore_delete_nullpointer",
		.func = test_semaphore_delete_nullpointer,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_semaphore_producer_consumer",
		.func = test_semaphore_producer_consumer,
		.setup = NULL,
		.teardown = NULL
	},
};

/* 测试套件定义 */
static const test_suite_t test_suite = {
	.suite_name = "osal_semaphore",
	.module_name = "osal_semaphore",
	.layer_name = "OSAL",
	.cases = test_cases,
	.case_count = OSAL_sizeof(test_cases) / OSAL_sizeof(test_case_t),
	.suite_setup = NULL,
	.suite_teardown = NULL,
	.metadata = {
		.category = TEST_CATEGORY_UNIT,
		.tags = TEST_TAG_FAST,
		.timeout_ms = 100,
		.description = "OSAL osal_semaphore tests"
	}
};

/* 测试套件注册函数 */
__attribute__((constructor))
static void register_osal_semaphore_tests(void)
{
	libutest_register_suite(&test_suite);
}
