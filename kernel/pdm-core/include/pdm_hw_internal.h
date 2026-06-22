// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_HW_INTERNAL_H
#define PDM_HW_INTERNAL_H

#define PDM_HW_BUILTIN_DRIVER_SECTION ".pdm_hw_builtin_driver"

typedef struct {
	const char *name;
	int (*init)(void);
	void (*exit)(void);
} pdm_hw_builtin_driver_t;

#define PDM_HW_BUILTIN_DRIVER(_id, _init, _exit)                         \
	static const pdm_hw_builtin_driver_t __lpf_hw_builtin_driver_##_id \
		__attribute__((used, aligned(sizeof(void *)),           \
			       section(PDM_HW_BUILTIN_DRIVER_SECTION))) = { \
			.name = #_id,                                  \
			.init = _init,                                 \
			.exit = _exit,                                 \
		}

extern const pdm_hw_builtin_driver_t pdm_hw_builtin_driver_start;
extern const pdm_hw_builtin_driver_t pdm_hw_builtin_driver_end;

#endif /* PDM_HW_INTERNAL_H */
