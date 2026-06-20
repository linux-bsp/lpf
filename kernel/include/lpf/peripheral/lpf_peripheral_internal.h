// SPDX-License-Identifier: GPL-2.0

#ifndef LPF_PERIPHERAL_INTERNAL_H
#define LPF_PERIPHERAL_INTERNAL_H

#include "osal.h"

#define LPF_PERIPHERAL_ENTRY_SECTION ".lpf_peripheral_entry"

typedef struct {
	const char *name;
	int32_t (*init)(void);
	void (*exit)(void);
} lpf_peripheral_entry_t;

#define lpf_peripheral_register(_id, _init, _exit)                       \
	static const lpf_peripheral_entry_t __lpf_peripheral_entry_##_id \
		__attribute__((used, aligned(sizeof(void *)),             \
			       section(LPF_PERIPHERAL_ENTRY_SECTION))) = { \
			.name = #_id,                                    \
			.init = _init,                                   \
			.exit = _exit,                                   \
		}

extern const lpf_peripheral_entry_t lpf_peripheral_entry_start;
extern const lpf_peripheral_entry_t lpf_peripheral_entry_end;
int32_t lpf_peripheral_probe_devices(void);

#endif /* LPF_PERIPHERAL_INTERNAL_H */
