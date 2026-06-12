#include "test_framework.h"
/**
 * @file test_osal_errno.c
 * @brief OSAL错误码操作单元测试
 */

#include "osal.h"

/*===========================================================================
 * 测试用例
 *===========================================================================*/

static void test_osal_get_errno(void)
{
    /* Set errno to a known value */
    OSAL_SetErrno(OSAL_EINVAL);

    /* Get errno */
    int32_t err = OSAL_GetErrno();
    TEST_ASSERT_EQUAL(OSAL_EINVAL, err);
}

static void test_osal_set_errno(void)
{
    /* Set errno to different values */
    OSAL_SetErrno(OSAL_ENOENT);
    TEST_ASSERT_EQUAL(OSAL_ENOENT, OSAL_GetErrno());

    OSAL_SetErrno(OSAL_ENOMEM);
    TEST_ASSERT_EQUAL(OSAL_ENOMEM, OSAL_GetErrno());

    OSAL_SetErrno(0);
    TEST_ASSERT_EQUAL(0, OSAL_GetErrno());
}

static void test_osal_strerror(void)
{
    /* Test common error codes */
    const char *msg;

    msg = OSAL_StrError(OSAL_EINVAL);
    TEST_ASSERT_NOT_NULL(msg);
    TEST_ASSERT_TRUE(OSAL_strlen(msg) > 0);

    msg = OSAL_StrError(OSAL_ENOENT);
    TEST_ASSERT_NOT_NULL(msg);
    TEST_ASSERT_TRUE(OSAL_strlen(msg) > 0);

    msg = OSAL_StrError(OSAL_ENOMEM);
    TEST_ASSERT_NOT_NULL(msg);
    TEST_ASSERT_TRUE(OSAL_strlen(msg) > 0);
}

static void test_osal_get_status_name(void)
{
    /* Test OSAL status code names */
    const char *name;

    name = OSAL_GetStatusName(OSAL_SUCCESS);
    TEST_ASSERT_NOT_NULL(name);
    TEST_ASSERT_STRING_EQUAL("OSAL_SUCCESS", name);

    name = OSAL_GetStatusName(OSAL_ERR_INVALID_POINTER);
    TEST_ASSERT_NOT_NULL(name);
    TEST_ASSERT_STRING_EQUAL("OSAL_ERR_INVALID_POINTER", name);

    name = OSAL_GetStatusName(OSAL_ERR_INVALID_SIZE);
    TEST_ASSERT_NOT_NULL(name);
    TEST_ASSERT_STRING_EQUAL("OSAL_ERR_INVALID_SIZE", name);
}

/*===========================================================================
 * 测试套件注册
 *===========================================================================*/

/* 测试用例数组 - 使用函数指针数组 */
static const test_case_t test_cases[] = {
	{
		.name = "test_osal_get_errno",
		.func = test_osal_get_errno,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_osal_set_errno",
		.func = test_osal_set_errno,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_osal_strerror",
		.func = test_osal_strerror,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_osal_get_status_name",
		.func = test_osal_get_status_name,
		.setup = NULL,
		.teardown = NULL
	},
};

/* 测试套件定义 */
static const test_suite_t test_suite = {
	.suite_name = "osal_errno",
	.module_name = "osal_errno",
	.layer_name = "OSAL",
	.cases = test_cases,
	.case_count = OSAL_SIZEOF(test_cases) / OSAL_SIZEOF(test_case_t),
	.suite_setup = NULL,
	.suite_teardown = NULL,
	.metadata = {
		.category = TEST_CATEGORY_UNIT,
		.tags = TEST_TAG_FAST,
		.timeout_ms = 100,
		.description = "OSAL osal_errno tests"
	}
};

/* 测试套件注册函数 */
__attribute__((constructor))
static void register_osal_errno_tests(void)
{
	libutest_register_suite(&test_suite);
}
