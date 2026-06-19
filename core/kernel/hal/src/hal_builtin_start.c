// SPDX-License-Identifier: GPL-2.0

#include "hal_internal.h"

const hal_builtin_driver_t hal_builtin_driver_start
	__attribute__((used, aligned(sizeof(void *)),
		       section(HAL_BUILTIN_DRIVER_SECTION))) = {};
