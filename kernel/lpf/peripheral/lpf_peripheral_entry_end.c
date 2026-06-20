// SPDX-License-Identifier: GPL-2.0

#include "lpf/peripheral/lpf_peripheral_internal.h"

const lpf_peripheral_entry_t lpf_peripheral_entry_end
	__attribute__((used, aligned(sizeof(void *)),
		       section(LPF_PERIPHERAL_ENTRY_SECTION))) = {};
