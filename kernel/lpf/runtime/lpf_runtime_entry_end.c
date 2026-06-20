// SPDX-License-Identifier: GPL-2.0

#include "lpf/runtime/lpf_runtime_internal.h"

const lpf_runtime_entry_t lpf_runtime_entry_end
	__attribute__((used, aligned(sizeof(void *)),
		       section(LPF_RUNTIME_ENTRY_SECTION))) = {};
