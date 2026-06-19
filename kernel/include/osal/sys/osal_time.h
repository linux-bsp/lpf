// SPDX-License-Identifier: GPL-2.0

#ifndef OSAL_TIME_H
#define OSAL_TIME_H

#include "osal_types.h"

#define OSAL_MSEC_PER_SEC 1000U
#define OSAL_USEC_PER_SEC 1000000U
#define OSAL_NSEC_PER_SEC 1000000000U
#define OSAL_USEC_PER_MSEC 1000U
#define OSAL_NSEC_PER_MSEC 1000000U
#define OSAL_NSEC_PER_USEC 1000U

int32_t osal_msleep(uint32_t msec);
int32_t osal_usleep(uint32_t usec);
int32_t osal_sleep(uint32_t sec);
int32_t osal_nanosleep(uint64_t nsec);
int64_t osal_get_monotonic_time(void);
int64_t osal_get_boot_time(void);
int64_t osal_get_highres_time(void);

#endif /* OSAL_TIME_H */
