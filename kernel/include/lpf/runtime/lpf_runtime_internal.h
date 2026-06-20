// SPDX-License-Identifier: GPL-2.0

#ifndef LPF_RUNTIME_INTERNAL_H
#define LPF_RUNTIME_INTERNAL_H

#include "osal.h"

#define LPF_RUNTIME_ENTRY_SECTION ".lpf_runtime_entry"

typedef struct {
	const char *name;
	int32_t (*init)(void);
	void (*exit)(void);
} lpf_runtime_entry_t;

#define lpf_runtime_register(_id, _init, _exit)                       \
	static const lpf_runtime_entry_t __lpf_runtime_entry_##_id \
		__attribute__((used, aligned(sizeof(void *)),             \
			       section(LPF_RUNTIME_ENTRY_SECTION))) = { \
			.name = #_id,                                    \
			.init = _init,                                   \
			.exit = _exit,                                   \
		}

extern const lpf_runtime_entry_t lpf_runtime_entry_start;
extern const lpf_runtime_entry_t lpf_runtime_entry_end;
int32_t lpf_runtime_probe_devices(void);

#endif /* LPF_RUNTIME_INTERNAL_H */
