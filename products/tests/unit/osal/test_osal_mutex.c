/**
 * @file test_osal_mutex.c
 * @brief OSAL Mutex Unit Tests
 *
 * Tests OSAL mutex operations using function pointer array registration.
 *
 * Test Coverage:
 * - Mutex creation and destruction
 * - Mutex locking and unlocking
 * - Null pointer handling
 * - Multi-threaded synchronization
 */

#include "test_framework.h"
#include "osal.h"

static int32_t shared_counter = 0;

/* 测试用例1: 互斥锁创建成功 */
static void test_mutex_create_success(void)
{
	osal_mutex_t *mutex = NULL;

	int32_t ret = OSAL_MutexCreate(&mutex);

	TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
	TEST_ASSERT_NOT_EQUAL(NULL, mutex);

	OSAL_MutexDelete(mutex);
}

/* 测试用例2: 互斥锁创建失败 - 空指针 */
static void test_mutex_create_nullpointer(void)
{
	int32_t ret = OSAL_MutexCreate(NULL);
	TEST_ASSERT_EQUAL(OSAL_ERR_INVALID_POINTER, ret);
}

/* 测试用例3: 互斥锁加锁解锁 */
static void test_mutex_lockunlock_success(void)
{
	osal_mutex_t *mutex = NULL;
	OSAL_MutexCreate(&mutex);

	int32_t ret = OSAL_MutexLock(mutex);
	TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

	ret = OSAL_MutexUnlock(mutex);
	TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

	OSAL_MutexDelete(mutex);
}

/* 测试用例4: 互斥锁加锁失败 - 空指针 */
static void test_mutex_lock_nullpointer(void)
{
	int32_t ret = OSAL_MutexLock(NULL);
	TEST_ASSERT_EQUAL(OSAL_ERR_INVALID_POINTER, ret);
}

/* 测试用例5: 互斥锁解锁失败 - 空指针 */
static void test_mutex_unlock_nullpointer(void)
{
	int32_t ret = OSAL_MutexUnlock(NULL);
	TEST_ASSERT_EQUAL(OSAL_ERR_INVALID_POINTER, ret);
}

/* 测试用例6: 互斥锁删除 */
static void test_mutex_delete_success(void)
{
	osal_mutex_t *mutex = NULL;
	OSAL_MutexCreate(&mutex);

	int32_t ret = OSAL_MutexDelete(mutex);
	TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
}

/* 测试用例7: 互斥锁删除失败 - 空指针 */
static void test_mutex_delete_nullpointer(void)
{
	int32_t ret = OSAL_MutexDelete(NULL);
	TEST_ASSERT_EQUAL(OSAL_ERR_INVALID_POINTER, ret);
}

/* 线程函数 - 用于测试互斥锁保护 */
static void* increment_thread(void *arg)
{
	osal_mutex_t *mutex = (osal_mutex_t *)arg;
	int32_t i;

	for (i = 0; i < 1000; i++) {
		OSAL_MutexLock(mutex);
		shared_counter++;
		OSAL_MutexUnlock(mutex);
	}

	return NULL;
}

/* 测试用例8: 互斥锁保护共享资源 */
static void test_mutex_protect_shared_resource(void)
{
	shared_counter = 0;
	osal_mutex_t *mutex = NULL;
	OSAL_MutexCreate(&mutex);

	osal_thread_t thread1, thread2;

	/* 创建两个线程同时增加计数器 */
	OSAL_ThreadCreate(&thread1, increment_thread, mutex);
	OSAL_ThreadCreate(&thread2, increment_thread, mutex);

	/* 等待线程完成 */
	OSAL_ThreadJoin(thread1);
	OSAL_ThreadJoin(thread2);

	/* 验证计数器正确 */
	TEST_ASSERT_EQUAL(2000, shared_counter);

	OSAL_MutexDelete(mutex);
}

/* 测试用例数组 - 使用函数指针数组 */
static const test_case_t test_cases[] = {
	{
		.name = "test_mutex_create_success",
		.func = test_mutex_create_success,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_mutex_create_nullpointer",
		.func = test_mutex_create_nullpointer,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_mutex_lockunlock_success",
		.func = test_mutex_lockunlock_success,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_mutex_lock_nullpointer",
		.func = test_mutex_lock_nullpointer,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_mutex_unlock_nullpointer",
		.func = test_mutex_unlock_nullpointer,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_mutex_delete_success",
		.func = test_mutex_delete_success,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_mutex_delete_nullpointer",
		.func = test_mutex_delete_nullpointer,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_mutex_protect_shared_resource",
		.func = test_mutex_protect_shared_resource,
		.setup = NULL,
		.teardown = NULL
	},
};

/* 测试套件定义 */
static const test_suite_t test_suite = {
	.suite_name = "osal_mutex",
	.module_name = "osal_mutex",
	.layer_name = "OSAL",
	.cases = test_cases,
	.case_count = OSAL_sizeof(test_cases) / OSAL_sizeof(test_case_t),
	.suite_setup = NULL,
	.suite_teardown = NULL,
	.metadata = {
		.category = TEST_CATEGORY_UNIT,
		.tags = TEST_TAG_FAST,
		.timeout_ms = 100,
		.description = "OSAL osal_mutex tests"
	}
};

/* 测试套件注册函数 */
__attribute__((constructor))
static void register_osal_mutex_tests(void)
{
	libutest_register_suite(&test_suite);
}
