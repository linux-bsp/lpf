// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_COMPAT_SYSFS_H
#define PDM_COMPAT_SYSFS_H

#include "pdm/compat/pdm_compat_features.h"

#include <linux/kernel.h>
#include <linux/sysfs.h>

#if PDM_KERNEL_HAS_SYSFS_EMIT
#define pdm_compat_sysfs_emit(buf, fmt, ...) \
	sysfs_emit((buf), (fmt), ##__VA_ARGS__)
#else
#define pdm_compat_sysfs_emit(buf, fmt, ...) \
	scnprintf((buf), PAGE_SIZE, (fmt), ##__VA_ARGS__)
#endif

#endif /* PDM_COMPAT_SYSFS_H */
