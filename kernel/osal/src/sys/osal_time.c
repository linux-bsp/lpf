// SPDX-License-Identifier: GPL-2.0

#include "osal/osal.h"

#include <linux/delay.h>
#include <linux/math64.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>

int32_t osal_msleep(uint32_t msec)
{
	msleep(msec);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_msleep);

int32_t osal_usleep(uint32_t usec)
{
	if (usec == 0) {
		return OSAL_SUCCESS;
	}

	if (usec < 10) {
		udelay(usec);
	}
	else {
		usleep_range(usec, usec + min(usec / 8U, 1000U));
	}

	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_usleep);

int32_t osal_sleep(uint32_t sec)
{
	uint64_t msec;

	msec = (uint64_t)sec * OSAL_MSEC_PER_SEC;
	if (msec > U32_MAX) {
		return OSAL_ERR_INVALID_SIZE;
	}

	return osal_msleep((uint32_t)msec);
}
EXPORT_SYMBOL_GPL(osal_sleep);

int32_t osal_nanosleep(uint64_t nsec)
{
	uint64_t usec;

	usec = DIV_ROUND_UP_ULL(nsec, OSAL_NSEC_PER_USEC);
	if (usec > U32_MAX) {
		return OSAL_ERR_INVALID_SIZE;
	}

	return osal_usleep((uint32_t)usec);
}
EXPORT_SYMBOL_GPL(osal_nanosleep);

int64_t osal_get_monotonic_time(void)
{
	return (int64_t)div_u64(ktime_get_ns(), OSAL_NSEC_PER_USEC);
}
EXPORT_SYMBOL_GPL(osal_get_monotonic_time);

int64_t osal_get_boot_time(void)
{
	return (int64_t)div_u64(ktime_get_boottime_ns(), OSAL_NSEC_PER_USEC);
}
EXPORT_SYMBOL_GPL(osal_get_boot_time);

int64_t osal_get_highres_time(void)
{
	return (int64_t)ktime_get_ns();
}
EXPORT_SYMBOL_GPL(osal_get_highres_time);
