// SPDX-License-Identifier: GPL-2.0

#ifndef HAL_INTERNAL_H
#define HAL_INTERNAL_H

#define HAL_BUILTIN_DRIVER_SECTION ".hal_builtin_driver"

typedef struct {
	const char *name;
	int (*init)(void);
	void (*exit)(void);
} hal_builtin_driver_t;

#define HAL_BUILTIN_DRIVER(_id, _init, _exit)                         \
	static const hal_builtin_driver_t __hal_builtin_driver_##_id   \
		__attribute__((used, aligned(sizeof(void *)),           \
			       section(HAL_BUILTIN_DRIVER_SECTION))) = { \
			.name = #_id,                                  \
			.init = _init,                                 \
			.exit = _exit,                                 \
		}

extern const hal_builtin_driver_t hal_builtin_driver_start;
extern const hal_builtin_driver_t hal_builtin_driver_end;

#endif /* HAL_INTERNAL_H */
