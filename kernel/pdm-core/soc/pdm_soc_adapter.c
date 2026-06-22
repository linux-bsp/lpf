// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>

#include "pdm/soc/pdm_soc_adapter.h"

#if defined(CONFIG_PDM_SOC_ADAPTER_MOCK)
extern const pdm_soc_adapter_t g_lpf_soc_mock_adapter;
#define PDM_SOC_DEFAULT_ADAPTER g_lpf_soc_mock_adapter
#else
extern const pdm_soc_adapter_t g_lpf_soc_generic_linux_adapter;
#define PDM_SOC_DEFAULT_ADAPTER g_lpf_soc_generic_linux_adapter
#endif

static osal_mutex_t g_lpf_soc_lock;
static bool g_lpf_soc_ready;
static const pdm_soc_adapter_t *g_lpf_soc_adapter;

static const pdm_soc_adapter_t *pdm_soc_get_adapter(void)
{
	const pdm_soc_adapter_t *adapter;

	if (!g_lpf_soc_ready)
		return NULL;

	osal_mutex_lock(&g_lpf_soc_lock);
	adapter = g_lpf_soc_adapter;
	osal_mutex_unlock(&g_lpf_soc_lock);
	return adapter;
}

int32_t pdm_soc_adapter_init(void)
{
	int32_t ret;

	if (g_lpf_soc_ready)
		return OSAL_SUCCESS;

	ret = osal_mutex_init(&g_lpf_soc_lock, NULL);
	if (ret != OSAL_SUCCESS)
		return ret;

	g_lpf_soc_ready = true;
	ret = pdm_soc_adapter_register(&PDM_SOC_DEFAULT_ADAPTER);
	if (ret != OSAL_SUCCESS) {
		g_lpf_soc_ready = false;
		osal_mutex_destroy(&g_lpf_soc_lock);
		return ret;
	}

	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(pdm_soc_adapter_init);

void pdm_soc_adapter_deinit(void)
{
	if (!g_lpf_soc_ready)
		return;

	osal_mutex_lock(&g_lpf_soc_lock);
	g_lpf_soc_adapter = NULL;
	osal_mutex_unlock(&g_lpf_soc_lock);
	osal_mutex_destroy(&g_lpf_soc_lock);
	g_lpf_soc_ready = false;
}
EXPORT_SYMBOL_GPL(pdm_soc_adapter_deinit);

int32_t pdm_soc_adapter_register(const pdm_soc_adapter_t *adapter)
{
	if (!g_lpf_soc_ready || !adapter || !adapter->name)
		return OSAL_ERR_INVALID_PARAM;

	osal_mutex_lock(&g_lpf_soc_lock);
	if (g_lpf_soc_adapter) {
		osal_mutex_unlock(&g_lpf_soc_lock);
		return OSAL_ERR_ALREADY_EXISTS;
	}

	g_lpf_soc_adapter = adapter;
	osal_mutex_unlock(&g_lpf_soc_lock);

	LOG_INFO("PDM", "registered SoC adapter %s", adapter->name);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(pdm_soc_adapter_register);

void pdm_soc_adapter_unregister(const pdm_soc_adapter_t *adapter)
{
	if (!g_lpf_soc_ready || !adapter)
		return;

	osal_mutex_lock(&g_lpf_soc_lock);
	if (g_lpf_soc_adapter == adapter)
		g_lpf_soc_adapter = NULL;
	osal_mutex_unlock(&g_lpf_soc_lock);
}
EXPORT_SYMBOL_GPL(pdm_soc_adapter_unregister);

const pdm_soc_adapter_t *pdm_soc_adapter_current(void)
{
	return pdm_soc_get_adapter();
}
EXPORT_SYMBOL_GPL(pdm_soc_adapter_current);

const char *pdm_soc_adapter_name(void)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	return adapter ? adapter->name : NULL;
}
EXPORT_SYMBOL_GPL(pdm_soc_adapter_name);

int32_t pdm_soc_gpio_request(uint32_t gpio_num, const char *label)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->gpio.request)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->gpio.request(gpio_num, label);
}
EXPORT_SYMBOL_GPL(pdm_soc_gpio_request);

void pdm_soc_gpio_free(uint32_t gpio_num)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (adapter && adapter->gpio.free)
		adapter->gpio.free(gpio_num);
}
EXPORT_SYMBOL_GPL(pdm_soc_gpio_free);

int32_t pdm_soc_gpio_set_direction(uint32_t gpio_num,
				   pdm_gpio_direction_t direction,
				   pdm_gpio_level_t initial_level)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->gpio.set_direction)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->gpio.set_direction(gpio_num, direction, initial_level);
}
EXPORT_SYMBOL_GPL(pdm_soc_gpio_set_direction);

int32_t pdm_soc_gpio_set_level(uint32_t gpio_num, pdm_gpio_level_t level)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->gpio.set_level)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->gpio.set_level(gpio_num, level);
}
EXPORT_SYMBOL_GPL(pdm_soc_gpio_set_level);

int32_t pdm_soc_gpio_get_level(uint32_t gpio_num, pdm_gpio_level_t *level)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->gpio.get_level)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->gpio.get_level(gpio_num, level);
}
EXPORT_SYMBOL_GPL(pdm_soc_gpio_get_level);

int32_t pdm_soc_gpio_request_interrupt(uint32_t gpio_num,
				       pdm_gpio_edge_t edge,
				       pdm_gpio_irq_callback_t callback,
				       void *user_data, void **irq_handle)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->gpio.request_interrupt)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->gpio.request_interrupt(gpio_num, edge, callback,
					       user_data, irq_handle);
}
EXPORT_SYMBOL_GPL(pdm_soc_gpio_request_interrupt);

void pdm_soc_gpio_free_interrupt(void *irq_handle)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (adapter && adapter->gpio.free_interrupt)
		adapter->gpio.free_interrupt(irq_handle);
}
EXPORT_SYMBOL_GPL(pdm_soc_gpio_free_interrupt);

int32_t pdm_soc_gpio_enable_interrupt(void *irq_handle)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->gpio.enable_interrupt)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->gpio.enable_interrupt(irq_handle);
}
EXPORT_SYMBOL_GPL(pdm_soc_gpio_enable_interrupt);

int32_t pdm_soc_gpio_disable_interrupt(void *irq_handle)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->gpio.disable_interrupt)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->gpio.disable_interrupt(irq_handle);
}
EXPORT_SYMBOL_GPL(pdm_soc_gpio_disable_interrupt);

int32_t pdm_soc_pwm_get(const char *consumer, pdm_pwm_handle_t *handle)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->pwm.get)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->pwm.get(consumer, handle);
}
EXPORT_SYMBOL_GPL(pdm_soc_pwm_get);

void pdm_soc_pwm_put(pdm_pwm_handle_t handle)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (adapter && adapter->pwm.put)
		adapter->pwm.put(handle);
}
EXPORT_SYMBOL_GPL(pdm_soc_pwm_put);

int32_t pdm_soc_pwm_apply(pdm_pwm_handle_t handle,
			  const pdm_pwm_state_t *state)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->pwm.apply)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->pwm.apply(handle, state);
}
EXPORT_SYMBOL_GPL(pdm_soc_pwm_apply);

int32_t pdm_soc_pwm_get_state(pdm_pwm_handle_t handle,
			      pdm_pwm_state_t *state)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->pwm.get_state)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->pwm.get_state(handle, state);
}
EXPORT_SYMBOL_GPL(pdm_soc_pwm_get_state);

int32_t pdm_soc_pwm_enable(pdm_pwm_handle_t handle)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->pwm.enable)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->pwm.enable(handle);
}
EXPORT_SYMBOL_GPL(pdm_soc_pwm_enable);

int32_t pdm_soc_pwm_disable(pdm_pwm_handle_t handle)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->pwm.disable)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->pwm.disable(handle);
}
EXPORT_SYMBOL_GPL(pdm_soc_pwm_disable);

int32_t pdm_soc_i2c_open(const char *device, pdm_i2c_handle_t *handle)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->i2c.open)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->i2c.open(device, handle);
}
EXPORT_SYMBOL_GPL(pdm_soc_i2c_open);

void pdm_soc_i2c_close(pdm_i2c_handle_t handle)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (adapter && adapter->i2c.close)
		adapter->i2c.close(handle);
}
EXPORT_SYMBOL_GPL(pdm_soc_i2c_close);

int32_t pdm_soc_i2c_transfer(pdm_i2c_handle_t handle,
			     pdm_i2c_msg_t *msgs, uint32_t num)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->i2c.transfer)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->i2c.transfer(handle, msgs, num);
}
EXPORT_SYMBOL_GPL(pdm_soc_i2c_transfer);

int32_t pdm_soc_spi_open(const pdm_spi_config_t *config,
			 pdm_spi_handle_t *handle)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->spi.open)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->spi.open(config, handle);
}
EXPORT_SYMBOL_GPL(pdm_soc_spi_open);

void pdm_soc_spi_close(pdm_spi_handle_t handle)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (adapter && adapter->spi.close)
		adapter->spi.close(handle);
}
EXPORT_SYMBOL_GPL(pdm_soc_spi_close);

int32_t pdm_soc_spi_transfer(pdm_spi_handle_t handle,
			     const pdm_spi_transfer_t *transfers,
			     uint32_t num)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->spi.transfer)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->spi.transfer(handle, transfers, num);
}
EXPORT_SYMBOL_GPL(pdm_soc_spi_transfer);

int32_t pdm_soc_spi_set_config(pdm_spi_handle_t handle,
			       const pdm_spi_config_t *config)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->spi.set_config)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->spi.set_config(handle, config);
}
EXPORT_SYMBOL_GPL(pdm_soc_spi_set_config);

int32_t pdm_soc_can_init(const pdm_can_config_t *config,
			 pdm_can_handle_t *handle)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->can.init)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->can.init(config, handle);
}
EXPORT_SYMBOL_GPL(pdm_soc_can_init);

int32_t pdm_soc_can_deinit(pdm_can_handle_t handle)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->can.deinit)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->can.deinit(handle);
}
EXPORT_SYMBOL_GPL(pdm_soc_can_deinit);

int32_t pdm_soc_can_send(pdm_can_handle_t handle,
			 const pdm_can_frame_t *frame)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->can.send)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->can.send(handle, frame);
}
EXPORT_SYMBOL_GPL(pdm_soc_can_send);

int32_t pdm_soc_can_recv(pdm_can_handle_t handle, pdm_can_frame_t *frame,
			 int32_t timeout)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->can.recv)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->can.recv(handle, frame, timeout);
}
EXPORT_SYMBOL_GPL(pdm_soc_can_recv);

int32_t pdm_soc_can_set_filter(pdm_can_handle_t handle, uint32_t filter_id,
			       uint32_t filter_mask)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->can.set_filter)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->can.set_filter(handle, filter_id, filter_mask);
}
EXPORT_SYMBOL_GPL(pdm_soc_can_set_filter);

int32_t pdm_soc_serial_open(const char *device,
			    const pdm_serial_config_t *config,
			    pdm_serial_handle_t *handle)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->serial.open)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->serial.open(device, config, handle);
}
EXPORT_SYMBOL_GPL(pdm_soc_serial_open);

int32_t pdm_soc_serial_close(pdm_serial_handle_t handle)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->serial.close)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->serial.close(handle);
}
EXPORT_SYMBOL_GPL(pdm_soc_serial_close);

int32_t pdm_soc_serial_write(pdm_serial_handle_t handle, const void *buffer,
			     uint32_t size, int32_t timeout)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->serial.write)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->serial.write(handle, buffer, size, timeout);
}
EXPORT_SYMBOL_GPL(pdm_soc_serial_write);

int32_t pdm_soc_serial_read(pdm_serial_handle_t handle, void *buffer,
			    uint32_t size, int32_t timeout)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->serial.read)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->serial.read(handle, buffer, size, timeout);
}
EXPORT_SYMBOL_GPL(pdm_soc_serial_read);

int32_t pdm_soc_serial_flush(pdm_serial_handle_t handle)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->serial.flush)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->serial.flush(handle);
}
EXPORT_SYMBOL_GPL(pdm_soc_serial_flush);

int32_t pdm_soc_serial_set_config(pdm_serial_handle_t handle,
				  const pdm_serial_config_t *config)
{
	const pdm_soc_adapter_t *adapter = pdm_soc_get_adapter();

	if (!adapter || !adapter->serial.set_config)
		return OSAL_ERR_NOT_SUPPORTED;

	return adapter->serial.set_config(handle, config);
}
EXPORT_SYMBOL_GPL(pdm_soc_serial_set_config);
