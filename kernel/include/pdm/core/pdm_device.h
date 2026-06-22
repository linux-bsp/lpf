// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_DEVICE_H
#define PDM_DEVICE_H

#include "osal.h"

#define PDM_DEVICE_NAME_LEN 64U

#define PDM_DEVICE_CAP_NONE          0ULL
#define PDM_DEVICE_CAP_TRANSPORT_CAN (1ULL << 0)
#define PDM_DEVICE_CAP_TRANSPORT_UART (1ULL << 1)
#define PDM_DEVICE_CAP_CONTROL_GPIO  (1ULL << 8)
#define PDM_DEVICE_CAP_CONTROL_PWM   (1ULL << 9)
#define PDM_DEVICE_CAP_USER_IOCTL    (1ULL << 16)
#define PDM_DEVICE_CAP_DEBUGFS       (1ULL << 17)

typedef enum {
	PDM_DEVICE_TYPE_INVALID = 0x00,
	PDM_DEVICE_TYPE_MCU = 0x01,
	PDM_DEVICE_TYPE_LED = 0x02,
	PDM_DEVICE_TYPE_DUMMY = 0x7F,
} pdm_device_type_t;

typedef enum {
	PDM_DEVICE_STATE_REGISTERED = 0,
	PDM_DEVICE_STATE_BOUND,
	PDM_DEVICE_STATE_ERROR,
} pdm_device_state_t;

typedef uint64_t pdm_capability_t;

typedef struct {
	/* Match key used to find the registered PDM driver. */
	pdm_device_type_t type;
	/* Stable instance index within one PDM device type. */
	uint32_t index;
	/* Typed configuration payload owned by the runtime config driver. */
	const void *entry;
	/* Optional configured name; Core falls back to driver name + index. */
	const char *name;
	pdm_capability_t capabilities;
} pdm_device_config_t;

struct pdm_driver;

typedef struct pdm_device {
	pdm_device_config_t config;
	const struct pdm_driver *driver;
	pdm_device_state_t state;
	int32_t last_error;
	uint32_t error_count;
	pdm_capability_t capabilities;
	char name[PDM_DEVICE_NAME_LEN];
} pdm_device_t;

typedef struct {
	pdm_device_type_t type;
	uint32_t index;
	pdm_device_state_t state;
	int32_t last_error;
	uint32_t error_count;
	pdm_capability_t capabilities;
	char name[PDM_DEVICE_NAME_LEN];
	char driver_name[PDM_DEVICE_NAME_LEN];
} pdm_device_info_t;

#endif /* PDM_DEVICE_H */
