// SPDX-License-Identifier: GPL-2.0

#include "pdm_hw_internal.h"

const pdm_hw_builtin_driver_t pdm_hw_builtin_driver_end
	__attribute__((used, aligned(sizeof(void *)),
		       section(PDM_HW_BUILTIN_DRIVER_SECTION))) = {};
