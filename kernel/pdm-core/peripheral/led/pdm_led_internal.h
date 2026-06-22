// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_LED_INTERNAL_H
#define PDM_LED_INTERNAL_H

#include "osal.h"
#include "pdm/config/pdm_config.h"
#include "pdm/core/pdm_core.h"
#include "pdm/peripheral/led/pdm_led_service.h"

#ifndef CONFIG_PDM_LED_MAX_DEVICES
#define CONFIG_PDM_LED_MAX_DEVICES 8
#endif

#define PDM_LED_MAX_DEVICES CONFIG_PDM_LED_MAX_DEVICES

typedef struct pdm_led_context {
	const pdm_config_led_config_t *config;
	void *hw_handle;
	osal_mutex_t lock;
	uint32_t brightness;
	bool enabled;
	bool lock_ready;
	uint32_t index;
	struct pdm_led_context *next;
} pdm_led_context_t;

typedef struct {
	pdm_config_led_control_t control;
	int32_t (*init)(pdm_led_context_t *ctx);
	int32_t (*apply)(pdm_led_context_t *ctx);
	void (*deinit)(pdm_led_context_t *ctx);
} pdm_led_control_ops_t;

typedef struct {
	bool present;
	char name[64];
	pdm_config_led_control_t control;
	bool enabled;
	uint32_t brightness;
	uint32_t max_brightness;
} pdm_led_debug_info_t;

int32_t pdm_led_probe(const pdm_device_t *device);
void pdm_led_remove(const pdm_device_t *device);
pdm_led_handle_t pdm_led_get(uint32_t index);
int32_t pdm_led_debug_get(uint32_t index, pdm_led_debug_info_t *info);
const pdm_led_control_ops_t *
pdm_led_control_get(pdm_config_led_control_t control);
extern const pdm_led_control_ops_t pdm_led_gpio_ops;
extern const pdm_led_control_ops_t pdm_led_pwm_ops;

int pdm_led_chrdev_register(void);
void pdm_led_chrdev_unregister(void);
int pdm_led_chrdev_register_device(const pdm_device_t *device);
void pdm_led_chrdev_unregister_device(const pdm_device_t *device);
void pdm_led_chrdev_record_error(uint32_t index, int error);
void pdm_led_chrdev_record_recovery(uint32_t index);
int pdm_led_proc_register(void);
void pdm_led_proc_unregister(void);
int pdm_led_debugfs_register(void);
void pdm_led_debugfs_unregister(void);

#endif /* PDM_LED_INTERNAL_H */
