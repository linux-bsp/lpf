/**
 * @file test_pdi_api.c
 * @brief PDI userspace API tests
 */

#include <errno.h>
#include <stdint.h>
#include <unistd.h>

#include <test_framework/test_framework.h>
#include "pdi.h"

#define TEST_PDI_MISSING_DEVICE "/tmp/es_middleware_pdi_missing_device"

static void _test_pdi_open_null_context(void)
{
	errno = 0;
	TEST_ASSERT_EQUAL(-1, pdi_open(NULL, NULL));
	TEST_ASSERT_EQUAL(EINVAL, errno);
}

static void _test_pdi_open_missing_device(void)
{
	pdi_context_t ctx = { .fd = -1 };

	(void)unlink(TEST_PDI_MISSING_DEVICE);
	errno = 0;
	TEST_ASSERT_EQUAL(-1, pdi_open(&ctx, TEST_PDI_MISSING_DEVICE));
	TEST_ASSERT_EQUAL(ENOENT, errno);
	TEST_ASSERT_EQUAL(-1, ctx.fd);
}

static void _test_pdi_close_invalid_context(void)
{
	pdi_context_t ctx = { .fd = -1 };

	errno = 0;
	TEST_ASSERT_EQUAL(-1, pdi_close(NULL));
	TEST_ASSERT_EQUAL(EINVAL, errno);

	errno = 0;
	TEST_ASSERT_EQUAL(-1, pdi_close(&ctx));
	TEST_ASSERT_EQUAL(EINVAL, errno);
}

static void _test_pdi_ioctl_invalid_context(void)
{
	pdi_context_t ctx = { .fd = -1 };
	struct pdi_info info;
	uint32_t echo_enabled = 0;

	errno = 0;
	TEST_ASSERT_EQUAL(-1, pdi_ping(&ctx));
	TEST_ASSERT_EQUAL(EINVAL, errno);

	errno = 0;
	TEST_ASSERT_EQUAL(-1, pdi_get_info(&ctx, &info));
	TEST_ASSERT_EQUAL(EINVAL, errno);

	errno = 0;
	TEST_ASSERT_EQUAL(-1, pdi_get_echo(&ctx, &echo_enabled));
	TEST_ASSERT_EQUAL(EINVAL, errno);

	errno = 0;
	TEST_ASSERT_EQUAL(-1, pdi_set_echo(&ctx, 1U));
	TEST_ASSERT_EQUAL(EINVAL, errno);
}

static void _test_pdi_null_outputs(void)
{
	pdi_context_t ctx = { .fd = -1 };

	errno = 0;
	TEST_ASSERT_EQUAL(-1, pdi_get_info(&ctx, NULL));
	TEST_ASSERT_EQUAL(EINVAL, errno);

	errno = 0;
	TEST_ASSERT_EQUAL(-1, pdi_get_echo(&ctx, NULL));
	TEST_ASSERT_EQUAL(EINVAL, errno);
}

static const test_case_t test_cases[] = {
	{ .name = "test_pdi_open_null_context",
	  .func = _test_pdi_open_null_context,
	  .setup = NULL,
	  .teardown = NULL },
	{ .name = "test_pdi_open_missing_device",
	  .func = _test_pdi_open_missing_device,
	  .setup = NULL,
	  .teardown = NULL },
	{ .name = "test_pdi_close_invalid_context",
	  .func = _test_pdi_close_invalid_context,
	  .setup = NULL,
	  .teardown = NULL },
	{ .name = "test_pdi_ioctl_invalid_context",
	  .func = _test_pdi_ioctl_invalid_context,
	  .setup = NULL,
	  .teardown = NULL },
	{ .name = "test_pdi_null_outputs",
	  .func = _test_pdi_null_outputs,
	  .setup = NULL,
	  .teardown = NULL },
};

static const test_suite_t test_suite = {
	.suite_name = "pdi_api",
	.module_name = "pdi",
	.layer_name = "PDI",
	.cases = test_cases,
	.case_count = OSAL_sizeof(test_cases) / OSAL_sizeof(test_case_t),
	.suite_setup = NULL,
	.suite_teardown = NULL,
	.metadata = { .category = TEST_CATEGORY_UNIT,
				  .tags = TEST_TAG_FAST,
				  .timeout_ms = 1000,
				  .description = "PDI userspace API validation tests" }
};

void register_pdi_api_tests(void)
{
	libutest_register_suite(&test_suite);
}
