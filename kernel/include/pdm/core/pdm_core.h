// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_CORE_H
#define PDM_CORE_H

#include "pdm/core/pdm_device.h"

/*
 * PDM Core - Minimal compatibility header
 *
 * This header provides minimal type definitions for backward compatibility.
 * The old pseudo-bus API has been removed. New code should use pdm_bus.h.
 */

/* Legacy types - kept for compatibility with existing code */
typedef struct pdm_device_handle pdm_device_handle_t;

typedef enum {
	PDM_DEVICE_EVENT_REGISTERED = 0,
	PDM_DEVICE_EVENT_BOUND,
	PDM_DEVICE_EVENT_STATE_CHANGED,
	PDM_DEVICE_EVENT_ERROR,
	PDM_DEVICE_EVENT_REMOVING,
	PDM_DEVICE_EVENT_REMOVED,
} pdm_device_event_type_t;

typedef struct {
	pdm_device_event_type_t type;
	pdm_device_info_t device;
	int32_t status;
} pdm_device_event_t;

typedef void (*pdm_device_event_callback_t)(
	const pdm_device_event_t *event, void *user_data);

/*
 * Old pseudo-bus API has been removed.
 * Use Linux bus_type API from pdm_bus.h instead:
 * - pdm_driver_register() -> use module_init() with pdm_driver
 * - pdm_device_register() -> devices created from Device Tree
 */

#endif /* PDM_CORE_H */
