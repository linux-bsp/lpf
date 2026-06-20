// SPDX-License-Identifier: MIT

#include "pdi/pdi.h"

#include <errno.h>
#include <stdint.h>
#include <string.h>

#define MISSING_DEVICE_PATH "/tmp/lpf_missing_device_for_pdi_test"

static int expect_failure_errno(int32_t ret, int expected_errno)
{
	return ret == -1 && errno == expected_errno ? 0 : -1;
}

static int test_ctl_validation(void)
{
	pdi_ctl_context_t ctx = { .fd = -1 };
	struct lpf_ctl_info info;
	struct lpf_ctl_device_info device;
	uint32_t count = 0;

	errno = 0;
	if (expect_failure_errno(pdi_ctl_open(NULL, NULL), EINVAL))
		return 1;

	errno = 0;
	ctx.fd = 123;
	if (pdi_ctl_open(&ctx, MISSING_DEVICE_PATH) != -1)
		return 2;
	if (errno != ENOENT || ctx.fd != -1)
		return 3;

	errno = 0;
	if (expect_failure_errno(pdi_ctl_close(NULL), EINVAL))
		return 4;

	errno = 0;
	if (expect_failure_errno(pdi_ctl_close(&ctx), EBADF))
		return 5;

	errno = 0;
	if (expect_failure_errno(pdi_ctl_get_info(NULL, &info), EINVAL))
		return 6;

	errno = 0;
	if (expect_failure_errno(pdi_ctl_get_info(&ctx, NULL), EINVAL))
		return 7;

	errno = 0;
	if (expect_failure_errno(pdi_ctl_get_info(&ctx, &info), EBADF))
		return 8;

	errno = 0;
	if (expect_failure_errno(pdi_list_devices(&ctx, NULL, NULL), EINVAL))
		return 9;

	errno = 0;
	count = 1;
	if (expect_failure_errno(pdi_list_devices(&ctx, NULL, &count), EBADF))
		return 10;

	errno = 0;
	if (expect_failure_errno(pdi_get_device_by_name(NULL, "mcu0", &device),
				 EINVAL))
		return 11;

	errno = 0;
	if (expect_failure_errno(pdi_get_device_by_name(&ctx, NULL, &device),
				 EINVAL))
		return 12;

	errno = 0;
	if (expect_failure_errno(pdi_get_device_by_name(&ctx, "mcu0", NULL),
				 EINVAL))
		return 13;

	errno = 0;
	if (expect_failure_errno(
		    pdi_get_device_by_capability(&ctx, LPF_CTL_DEVICE_CAP_NONE,
						 0, &device),
		    EINVAL))
		return 14;

	errno = 0;
	if (expect_failure_errno(
		    pdi_get_device_by_capability(&ctx,
						 LPF_CTL_DEVICE_CAP_USER_IOCTL,
						 0, NULL),
		    EINVAL))
		return 15;

	return 0;
}

static int test_mcu_validation(void)
{
	pdi_mcu_context_t ctx = { .fd = -1 };
	struct lpf_mcu_info info;
	struct lpf_mcu_version version;
	struct lpf_mcu_status status;
	struct lpf_mcu_command command;
	struct lpf_mcu_data data;

	memset(&command, 0, sizeof(command));
	memset(&data, 0, sizeof(data));

	errno = 0;
	if (expect_failure_errno(pdi_mcu_open(NULL, NULL), EINVAL))
		return 101;

	errno = 0;
	ctx.fd = 123;
	if (pdi_mcu_open(&ctx, MISSING_DEVICE_PATH) != -1)
		return 102;
	if (errno != ENOENT || ctx.fd != -1)
		return 103;

	errno = 0;
	if (expect_failure_errno(pdi_mcu_open_by_name(NULL, "mcu0"), EINVAL))
		return 104;

	errno = 0;
	ctx.fd = 123;
	if (expect_failure_errno(pdi_mcu_open_by_name(&ctx, NULL), EINVAL))
		return 105;
	if (ctx.fd != -1)
		return 122;

	errno = 0;
	if (expect_failure_errno(pdi_mcu_close(NULL), EINVAL))
		return 106;

	errno = 0;
	if (expect_failure_errno(pdi_mcu_close(&ctx), EBADF))
		return 107;

	errno = 0;
	if (expect_failure_errno(pdi_mcu_get_info(NULL, &info), EINVAL))
		return 108;

	errno = 0;
	if (expect_failure_errno(pdi_mcu_get_info(&ctx, NULL), EINVAL))
		return 109;

	errno = 0;
	if (expect_failure_errno(pdi_mcu_get_info(&ctx, &info), EBADF))
		return 110;

	errno = 0;
	if (expect_failure_errno(pdi_mcu_get_version(&ctx, NULL), EINVAL))
		return 111;

	errno = 0;
	if (expect_failure_errno(pdi_mcu_get_version(&ctx, &version), EBADF))
		return 112;

	errno = 0;
	if (expect_failure_errno(pdi_mcu_get_status(&ctx, NULL), EINVAL))
		return 113;

	errno = 0;
	if (expect_failure_errno(pdi_mcu_get_status(&ctx, &status), EBADF))
		return 114;

	errno = 0;
	if (expect_failure_errno(pdi_mcu_reset(&ctx, 0), EBADF))
		return 115;

	errno = 0;
	if (expect_failure_errno(pdi_mcu_command(&ctx, NULL), EINVAL))
		return 116;

	errno = 0;
	if (expect_failure_errno(pdi_mcu_command(&ctx, &command), EBADF))
		return 117;

	errno = 0;
	if (expect_failure_errno(pdi_mcu_read_data(&ctx, NULL), EINVAL))
		return 118;

	errno = 0;
	if (expect_failure_errno(pdi_mcu_read_data(&ctx, &data), EBADF))
		return 119;

	errno = 0;
	if (expect_failure_errno(pdi_mcu_write_data(&ctx, NULL), EINVAL))
		return 120;

	errno = 0;
	if (expect_failure_errno(pdi_mcu_write_data(&ctx, &data), EBADF))
		return 121;

	return 0;
}

static int test_led_validation(void)
{
	pdi_led_context_t ctx = { .fd = -1 };
	struct lpf_led_info info;
	struct lpf_led_state state;

	errno = 0;
	if (expect_failure_errno(pdi_led_open(NULL, NULL), EINVAL))
		return 201;

	errno = 0;
	ctx.fd = 123;
	if (pdi_led_open(&ctx, MISSING_DEVICE_PATH) != -1)
		return 202;
	if (errno != ENOENT || ctx.fd != -1)
		return 203;

	errno = 0;
	if (expect_failure_errno(pdi_led_open_by_name(NULL, "led0"), EINVAL))
		return 204;

	errno = 0;
	ctx.fd = 123;
	if (expect_failure_errno(pdi_led_open_by_name(&ctx, NULL), EINVAL))
		return 205;
	if (ctx.fd != -1)
		return 216;

	errno = 0;
	if (expect_failure_errno(pdi_led_close(NULL), EINVAL))
		return 206;

	errno = 0;
	if (expect_failure_errno(pdi_led_close(&ctx), EBADF))
		return 207;

	errno = 0;
	if (expect_failure_errno(pdi_led_get_info(NULL, &info), EINVAL))
		return 208;

	errno = 0;
	if (expect_failure_errno(pdi_led_get_info(&ctx, NULL), EINVAL))
		return 209;

	errno = 0;
	if (expect_failure_errno(pdi_led_get_info(&ctx, &info), EBADF))
		return 210;

	errno = 0;
	if (expect_failure_errno(pdi_led_get_state(&ctx, NULL), EINVAL))
		return 211;

	errno = 0;
	if (expect_failure_errno(pdi_led_get_state(&ctx, &state), EBADF))
		return 212;

	errno = 0;
	if (expect_failure_errno(pdi_led_set_brightness(&ctx, 0, 1), EBADF))
		return 213;

	errno = 0;
	if (expect_failure_errno(pdi_led_enable(&ctx, 0), EBADF))
		return 214;

	errno = 0;
	if (expect_failure_errno(pdi_led_disable(&ctx, 0), EBADF))
		return 215;

	return 0;
}

int main(void)
{
	int ret;

	ret = test_ctl_validation();
	if (ret)
		return ret;

	ret = test_mcu_validation();
	if (ret)
		return ret;

	return test_led_validation();
}
