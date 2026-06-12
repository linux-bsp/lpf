#include "test_framework.h"
/**
 * @file test_osal_cond.c
 * @brief OSAL条件变量单元测试
 */

#include "osal.h"

static int32_t shared_data = 0;
static bool data_ready = false;

/* 测试用例1: 条件变量创建成功 */
static void test_cond_create_success(void)
{
    osal_cond_t *cond = NULL;
    int32_t ret = OSAL_CondCreate(&cond);

    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
    TEST_ASSERT_NOT_EQUAL(NULL, cond);

    OSAL_CondDelete(cond);
}

/* 测试用例2: 条件变量创建失败 - 空指针 */
static void test_cond_create_nullpointer(void)
{
    int32_t ret = OSAL_CondCreate(NULL);
    TEST_ASSERT_EQUAL(OSAL_ERR_INVALID_POINTER, ret);
}

/* 测试用例3: 条件变量删除成功 */
static void test_cond_delete_success(void)
{
    osal_cond_t *cond = NULL;
    OSAL_CondCreate(&cond);

    int32_t ret = OSAL_CondDelete(cond);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
}

/* 测试用例4: 条件变量删除失败 - 空指针 */
static void test_cond_delete_nullpointer(void)
{
    int32_t ret = OSAL_CondDelete(NULL);
    TEST_ASSERT_EQUAL(OSAL_ERR_INVALID_POINTER, ret);
}

/* 测试用例5: 条件变量Signal失败 - 空指针 */
static void test_cond_signal_nullpointer(void)
{
    int32_t ret = OSAL_CondSignal(NULL);
    TEST_ASSERT_EQUAL(OSAL_ERR_INVALID_POINTER, ret);
}

/* 测试用例6: 条件变量Broadcast失败 - 空指针 */
static void test_cond_broadcast_nullpointer(void)
{
    int32_t ret = OSAL_CondBroadcast(NULL);
    TEST_ASSERT_EQUAL(OSAL_ERR_INVALID_POINTER, ret);
}

/* 测试用例7: 条件变量Wait失败 - 空指针 */
static void test_cond_wait_nullpointer(void)
{
    osal_cond_t *cond = NULL;
    osal_mutex_t *mutex = NULL;

    OSAL_CondCreate(&cond);
    OSAL_MutexCreate(&mutex);

    /* cond 为 NULL */
    int32_t ret = OSAL_CondWait(NULL, mutex);
    TEST_ASSERT_EQUAL(OSAL_ERR_INVALID_POINTER, ret);

    /* mutex 为 NULL */
    ret = OSAL_CondWait(cond, NULL);
    TEST_ASSERT_EQUAL(OSAL_ERR_INVALID_POINTER, ret);

    OSAL_CondDelete(cond);
    OSAL_MutexDelete(mutex);
}

/* 测试用例8: 条件变量超时等待 - 超时 */
static void test_cond_timedwait_timeout(void)
{
    osal_cond_t *cond = NULL;
    osal_mutex_t *mutex = NULL;

    OSAL_CondCreate(&cond);
    OSAL_MutexCreate(&mutex);

    OSAL_MutexLock(mutex);
    int32_t ret = OSAL_CondTimedWait(cond, mutex, 100);
    OSAL_MutexUnlock(mutex);

    TEST_ASSERT_EQUAL(OSAL_ERR_TIMEOUT, ret);

    OSAL_CondDelete(cond);
    OSAL_MutexDelete(mutex);
}

/* 生产者线程 - 使用Signal */
static void* producer_signal_thread(void *arg)
{
    osal_cond_t *cond = ((osal_cond_t **)arg)[0];
    osal_mutex_t *mutex = ((osal_mutex_t **)arg)[1];

    OSAL_msleep(50);

    OSAL_MutexLock(mutex);
    shared_data = 42;
    data_ready = true;
    OSAL_CondSignal(cond);
    OSAL_MutexUnlock(mutex);

    return NULL;
}

/* 消费者线程 */
static void* consumer_thread(void *arg)
{
    osal_cond_t *cond = ((osal_cond_t **)arg)[0];
    osal_mutex_t *mutex = ((osal_mutex_t **)arg)[1];

    OSAL_MutexLock(mutex);
    while (!data_ready) {
        OSAL_CondWait(cond, mutex);
    }
    int32_t data = shared_data;
    OSAL_MutexUnlock(mutex);

    /* 验证数据 */
    if (data == 42) {
        shared_data = 100;  /* 标记成功 */
    }

    return NULL;
}

/* 测试用例9: 条件变量Signal唤醒 */
static void test_cond_signal_wakeup(void)
{
    shared_data = 0;
    data_ready = false;

    osal_cond_t *cond = NULL;
    osal_mutex_t *mutex = NULL;

    OSAL_CondCreate(&cond);
    OSAL_MutexCreate(&mutex);

    void *args[2] = {cond, mutex};
    osal_thread_t producer, consumer;

    OSAL_ThreadCreate(&consumer, consumer_thread, args);
    OSAL_ThreadCreate(&producer, producer_signal_thread, args);

    OSAL_ThreadJoin(producer);
    OSAL_ThreadJoin(consumer);

    TEST_ASSERT_EQUAL(100, shared_data);

    OSAL_CondDelete(cond);
    OSAL_MutexDelete(mutex);
}

/* 多个消费者线程 */
static int32_t consumer_count = 0;
static osal_mutex_t *count_mutex = NULL;

static void* multi_consumer_thread(void *arg)
{
    osal_cond_t *cond = ((osal_cond_t **)arg)[0];
    osal_mutex_t *mutex = ((osal_mutex_t **)arg)[1];

    OSAL_MutexLock(mutex);
    while (!data_ready) {
        OSAL_CondWait(cond, mutex);
    }
    OSAL_MutexUnlock(mutex);

    /* 增加计数器 */
    OSAL_MutexLock(count_mutex);
    consumer_count++;
    OSAL_MutexUnlock(count_mutex);

    return NULL;
}

/* 生产者线程 - 使用Broadcast */
static void* producer_broadcast_thread(void *arg)
{
    osal_cond_t *cond = ((osal_cond_t **)arg)[0];
    osal_mutex_t *mutex = ((osal_mutex_t **)arg)[1];

    OSAL_msleep(100);

    OSAL_MutexLock(mutex);
    data_ready = true;
    OSAL_CondBroadcast(cond);
    OSAL_MutexUnlock(mutex);

    return NULL;
}

/* 测试用例10: 条件变量Broadcast唤醒多个线程 */
static void test_cond_broadcast_wakeup(void)
{
    consumer_count = 0;
    data_ready = false;

    osal_cond_t *cond = NULL;
    osal_mutex_t *mutex = NULL;
    OSAL_MutexCreate(&count_mutex);

    OSAL_CondCreate(&cond);
    OSAL_MutexCreate(&mutex);

    void *args[2] = {cond, mutex};
    osal_thread_t producer, consumers[3];

    /* 创建3个消费者线程 */
    int32_t i;

    for (i = 0; i < 3; i++) {
        OSAL_ThreadCreate(&consumers[i], multi_consumer_thread, args);
    }

    /* 创建生产者线程 */
    OSAL_ThreadCreate(&producer, producer_broadcast_thread, args);

    /* 等待所有线程完成 */
    OSAL_ThreadJoin(producer);

    for (i = 0; i < 3; i++) {
        OSAL_ThreadJoin(consumers[i]);
    }

    /* 验证所有消费者都被唤醒 */
    TEST_ASSERT_EQUAL(3, consumer_count);

    OSAL_CondDelete(cond);
    OSAL_MutexDelete(mutex);
    OSAL_MutexDelete(count_mutex);
}

/* 注册测试套件 */

/* 测试用例数组 - 使用函数指针数组 */
static const test_case_t test_cases[] = {
	{
		.name = "test_cond_create_success",
		.func = test_cond_create_success,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_cond_create_nullpointer",
		.func = test_cond_create_nullpointer,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_cond_delete_success",
		.func = test_cond_delete_success,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_cond_delete_nullpointer",
		.func = test_cond_delete_nullpointer,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_cond_signal_nullpointer",
		.func = test_cond_signal_nullpointer,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_cond_broadcast_nullpointer",
		.func = test_cond_broadcast_nullpointer,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_cond_wait_nullpointer",
		.func = test_cond_wait_nullpointer,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_cond_timedwait_timeout",
		.func = test_cond_timedwait_timeout,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_cond_signal_wakeup",
		.func = test_cond_signal_wakeup,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_cond_broadcast_wakeup",
		.func = test_cond_broadcast_wakeup,
		.setup = NULL,
		.teardown = NULL
	},
};

/* 测试套件定义 */
static const test_suite_t test_suite = {
	.suite_name = "osal_cond",
	.module_name = "osal_cond",
	.layer_name = "OSAL",
	.cases = test_cases,
	.case_count = OSAL_sizeof(test_cases) / OSAL_sizeof(test_case_t),
	.suite_setup = NULL,
	.suite_teardown = NULL,
	.metadata = {
		.category = TEST_CATEGORY_UNIT,
		.tags = TEST_TAG_FAST,
		.timeout_ms = 100,
		.description = "OSAL osal_cond tests"
	}
};

/* 测试套件注册函数 */
__attribute__((constructor))
static void register_osal_cond_tests(void)
{
	libutest_register_suite(&test_suite);
}
