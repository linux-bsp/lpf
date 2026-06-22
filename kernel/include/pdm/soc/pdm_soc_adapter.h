// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_SOC_ADAPTER_H
#define PDM_SOC_ADAPTER_H

#include "pdm/types/pdm_can_types.h"
#include "pdm/types/pdm_gpio_types.h"
#include "pdm/types/pdm_i2c_types.h"
#include "pdm/types/pdm_pwm_types.h"
#include "pdm/types/pdm_serial_types.h"
#include "pdm/types/pdm_spi_types.h"

typedef struct {
	int32_t (*request)(uint32_t gpio_num, const char *label);
	void (*free)(uint32_t gpio_num);
	int32_t (*set_direction)(uint32_t gpio_num,
				 pdm_gpio_direction_t direction,
				 pdm_gpio_level_t initial_level);
	int32_t (*set_level)(uint32_t gpio_num, pdm_gpio_level_t level);
	int32_t (*get_level)(uint32_t gpio_num, pdm_gpio_level_t *level);
	int32_t (*request_interrupt)(uint32_t gpio_num, pdm_gpio_edge_t edge,
				     pdm_gpio_irq_callback_t callback,
				     void *user_data, void **irq_handle);
	void (*free_interrupt)(void *irq_handle);
	int32_t (*enable_interrupt)(void *irq_handle);
	int32_t (*disable_interrupt)(void *irq_handle);
} pdm_soc_gpio_ops_t;

typedef struct {
	int32_t (*get)(const char *consumer, pdm_pwm_handle_t *handle);
	void (*put)(pdm_pwm_handle_t handle);
	int32_t (*apply)(pdm_pwm_handle_t handle,
			 const pdm_pwm_state_t *state);
	int32_t (*get_state)(pdm_pwm_handle_t handle,
			     pdm_pwm_state_t *state);
	int32_t (*enable)(pdm_pwm_handle_t handle);
	int32_t (*disable)(pdm_pwm_handle_t handle);
} pdm_soc_pwm_ops_t;

typedef struct {
	int32_t (*open)(const char *device, pdm_i2c_handle_t *handle);
	void (*close)(pdm_i2c_handle_t handle);
	int32_t (*transfer)(pdm_i2c_handle_t handle, pdm_i2c_msg_t *msgs,
			    uint32_t num);
} pdm_soc_i2c_ops_t;

typedef struct {
	int32_t (*open)(const pdm_spi_config_t *config,
			pdm_spi_handle_t *handle);
	void (*close)(pdm_spi_handle_t handle);
	int32_t (*transfer)(pdm_spi_handle_t handle,
			    const pdm_spi_transfer_t *transfers,
			    uint32_t num);
	int32_t (*set_config)(pdm_spi_handle_t handle,
			      const pdm_spi_config_t *config);
} pdm_soc_spi_ops_t;

typedef struct {
	int32_t (*init)(const pdm_can_config_t *config,
			pdm_can_handle_t *handle);
	int32_t (*deinit)(pdm_can_handle_t handle);
	int32_t (*send)(pdm_can_handle_t handle,
			const pdm_can_frame_t *frame);
	int32_t (*recv)(pdm_can_handle_t handle, pdm_can_frame_t *frame,
			int32_t timeout);
	int32_t (*set_filter)(pdm_can_handle_t handle, uint32_t filter_id,
			      uint32_t filter_mask);
} pdm_soc_can_ops_t;

typedef struct {
	int32_t (*open)(const char *device, const pdm_serial_config_t *config,
			pdm_serial_handle_t *handle);
	int32_t (*close)(pdm_serial_handle_t handle);
	int32_t (*write)(pdm_serial_handle_t handle, const void *buffer,
			 uint32_t size, int32_t timeout);
	int32_t (*read)(pdm_serial_handle_t handle, void *buffer,
			uint32_t size, int32_t timeout);
	int32_t (*flush)(pdm_serial_handle_t handle);
	int32_t (*set_config)(pdm_serial_handle_t handle,
			      const pdm_serial_config_t *config);
} pdm_soc_serial_ops_t;

typedef struct {
	const char *name;
	pdm_soc_gpio_ops_t gpio;
	pdm_soc_pwm_ops_t pwm;
	pdm_soc_i2c_ops_t i2c;
	pdm_soc_spi_ops_t spi;
	pdm_soc_can_ops_t can;
	pdm_soc_serial_ops_t serial;
} pdm_soc_adapter_t;

int32_t pdm_soc_adapter_init(void);
void pdm_soc_adapter_deinit(void);
int32_t pdm_soc_adapter_register(const pdm_soc_adapter_t *adapter);
void pdm_soc_adapter_unregister(const pdm_soc_adapter_t *adapter);
const pdm_soc_adapter_t *pdm_soc_adapter_current(void);
const char *pdm_soc_adapter_name(void);

int32_t pdm_soc_gpio_request(uint32_t gpio_num, const char *label);
void pdm_soc_gpio_free(uint32_t gpio_num);
int32_t pdm_soc_gpio_set_direction(uint32_t gpio_num,
				   pdm_gpio_direction_t direction,
				   pdm_gpio_level_t initial_level);
int32_t pdm_soc_gpio_set_level(uint32_t gpio_num, pdm_gpio_level_t level);
int32_t pdm_soc_gpio_get_level(uint32_t gpio_num, pdm_gpio_level_t *level);
int32_t pdm_soc_gpio_request_interrupt(uint32_t gpio_num,
				       pdm_gpio_edge_t edge,
				       pdm_gpio_irq_callback_t callback,
				       void *user_data, void **irq_handle);
void pdm_soc_gpio_free_interrupt(void *irq_handle);
int32_t pdm_soc_gpio_enable_interrupt(void *irq_handle);
int32_t pdm_soc_gpio_disable_interrupt(void *irq_handle);

int32_t pdm_soc_pwm_get(const char *consumer, pdm_pwm_handle_t *handle);
void pdm_soc_pwm_put(pdm_pwm_handle_t handle);
int32_t pdm_soc_pwm_apply(pdm_pwm_handle_t handle,
			  const pdm_pwm_state_t *state);
int32_t pdm_soc_pwm_get_state(pdm_pwm_handle_t handle,
			      pdm_pwm_state_t *state);
int32_t pdm_soc_pwm_enable(pdm_pwm_handle_t handle);
int32_t pdm_soc_pwm_disable(pdm_pwm_handle_t handle);

int32_t pdm_soc_i2c_open(const char *device, pdm_i2c_handle_t *handle);
void pdm_soc_i2c_close(pdm_i2c_handle_t handle);
int32_t pdm_soc_i2c_transfer(pdm_i2c_handle_t handle,
			     pdm_i2c_msg_t *msgs, uint32_t num);

int32_t pdm_soc_spi_open(const pdm_spi_config_t *config,
			 pdm_spi_handle_t *handle);
void pdm_soc_spi_close(pdm_spi_handle_t handle);
int32_t pdm_soc_spi_transfer(pdm_spi_handle_t handle,
			     const pdm_spi_transfer_t *transfers,
			     uint32_t num);
int32_t pdm_soc_spi_set_config(pdm_spi_handle_t handle,
			       const pdm_spi_config_t *config);

int32_t pdm_soc_can_init(const pdm_can_config_t *config,
			 pdm_can_handle_t *handle);
int32_t pdm_soc_can_deinit(pdm_can_handle_t handle);
int32_t pdm_soc_can_send(pdm_can_handle_t handle,
			 const pdm_can_frame_t *frame);
int32_t pdm_soc_can_recv(pdm_can_handle_t handle, pdm_can_frame_t *frame,
			 int32_t timeout);
int32_t pdm_soc_can_set_filter(pdm_can_handle_t handle, uint32_t filter_id,
			       uint32_t filter_mask);

int32_t pdm_soc_serial_open(const char *device,
			    const pdm_serial_config_t *config,
			    pdm_serial_handle_t *handle);
int32_t pdm_soc_serial_close(pdm_serial_handle_t handle);
int32_t pdm_soc_serial_write(pdm_serial_handle_t handle, const void *buffer,
			     uint32_t size, int32_t timeout);
int32_t pdm_soc_serial_read(pdm_serial_handle_t handle, void *buffer,
			    uint32_t size, int32_t timeout);
int32_t pdm_soc_serial_flush(pdm_serial_handle_t handle);
int32_t pdm_soc_serial_set_config(pdm_serial_handle_t handle,
				  const pdm_serial_config_t *config);

#endif /* PDM_SOC_ADAPTER_H */
