// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_CORE_H
#define PDM_CORE_H

#include "pdm/core/pdm_device.h"
#include "pdm/core/pdm_driver.h"

/*
 * PDM Core is the PDM-owned device model. Runtime/peripheral code registers
 * PDM drivers and configured PDM devices here.
 * Core matches devices to drivers by PDM device type and calls probe/remove.
 */

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

int32_t pdm_driver_register(const pdm_driver_t *driver);
void pdm_driver_unregister(const pdm_driver_t *driver);
void pdm_driver_unregister_all(void);

/* Registers one configured PDM device and binds it to the matching driver. */
int32_t pdm_device_register(const pdm_device_config_t *config);
void pdm_device_unregister_all(void);
const pdm_device_t *pdm_device_find(pdm_device_type_t type, uint32_t index);
pdm_device_handle_t *pdm_device_get(pdm_device_type_t type, uint32_t index);
pdm_device_handle_t *pdm_device_get_by_name(const char *name);
pdm_device_handle_t *
pdm_device_get_by_capability(pdm_capability_t required, uint32_t match_index);
const pdm_device_t *pdm_device_from_handle(
	const pdm_device_handle_t *handle);
int32_t pdm_device_handle_get_info(const pdm_device_handle_t *handle,
				   pdm_device_info_t *info);
void pdm_device_put(pdm_device_handle_t *handle);
int32_t pdm_device_get_info(pdm_device_type_t type, uint32_t index,
			    pdm_device_info_t *info);
int32_t pdm_device_get_info_by_name(const char *name,
				    pdm_device_info_t *info);
int32_t pdm_device_get_info_by_capability(pdm_capability_t required,
					  uint32_t match_index,
					  pdm_device_info_t *info);
int32_t pdm_device_list(pdm_device_info_t *infos, uint32_t *count);
int32_t pdm_device_set_state(pdm_device_type_t type, uint32_t index,
			     pdm_device_state_t state, int32_t status);
void pdm_device_record_error(pdm_device_type_t type, uint32_t index,
			     int32_t error);
int32_t pdm_device_record_recovery(pdm_device_type_t type, uint32_t index);
int32_t pdm_device_event_subscribe(pdm_device_event_callback_t callback,
				   void *user_data);
void pdm_device_event_unsubscribe(pdm_device_event_callback_t callback,
				  void *user_data);

#endif /* PDM_CORE_H */
