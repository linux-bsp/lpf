/************************************************************************
 * PDM LED public API
 ************************************************************************/

#ifndef PDM_LED_H
#define PDM_LED_H

#include "osal.h"

#define PDM_LED_VERSION_MAJOR 0x01
#define PDM_LED_VERSION_MINOR 0x00
#define PDM_LED_VERSION_PATCH 0x00

typedef void *pdm_led_handle_t;

typedef struct {
	uint32_t brightness;
	uint32_t max_brightness;
	bool enabled;
} pdm_led_state_t;

int32_t pdm_led_set_brightness(pdm_led_handle_t handle, uint32_t brightness);
int32_t pdm_led_get_state(pdm_led_handle_t handle, pdm_led_state_t *state);
int32_t pdm_led_enable(pdm_led_handle_t handle);
int32_t pdm_led_disable(pdm_led_handle_t handle);

#endif /* PDM_LED_H */
