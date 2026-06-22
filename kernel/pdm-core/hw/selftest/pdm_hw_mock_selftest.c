// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>

#include "osal.h"
#include "pdm/hw/pdm_hw_can.h"
#include "pdm/hw/pdm_hw_gpio.h"
#include "pdm/hw/pdm_hw_i2c.h"
#include "pdm/hw/pdm_hw_pwm.h"
#include "pdm/hw/pdm_hw_uart.h"
#include "pdm/hw/pdm_hw_spi.h"

#define PDM_HW_MOCK_SELFTEST_GPIO 7U
#define PDM_HW_MOCK_SELFTEST_HIGH_GPIO 300U
#define PDM_HW_MOCK_SELFTEST_PWM_HANDLES 40U
#define PDM_HW_MOCK_SELFTEST_BUS_HANDLES 20U

typedef struct {
	uint32_t count;
	uint32_t gpio_num;
	pdm_gpio_level_t level;
} pdm_hw_mock_gpio_irq_state_t;

typedef struct {
	pdm_hw_pwm_handle_t pwm_handles[PDM_HW_MOCK_SELFTEST_PWM_HANDLES];
	pdm_hw_bus_i2c_handle_t i2c_handles[PDM_HW_MOCK_SELFTEST_BUS_HANDLES];
	pdm_hw_bus_spi_handle_t spi_handles[PDM_HW_MOCK_SELFTEST_BUS_HANDLES];
	pdm_hw_transport_can_handle_t can_handles[PDM_HW_MOCK_SELFTEST_BUS_HANDLES];
	pdm_hw_transport_uart_handle_t serial_handles[PDM_HW_MOCK_SELFTEST_BUS_HANDLES];
} pdm_hw_mock_dynamic_resource_handles_t;

static int pdm_hw_mock_status_to_errno(int32_t status)
{
	if (status == OSAL_SUCCESS)
		return 0;
	if (status > 0 && status < 4096)
		return -status;
	return -EINVAL;
}

static int32_t pdm_hw_mock_expect_success(const char *name, int32_t status)
{
	if (status == OSAL_SUCCESS)
		return OSAL_SUCCESS;

	pr_err("PDM:HW_SELFTEST: %s failed: %d\n", name, status);
	return status;
}

static void pdm_hw_mock_gpio_irq_callback(uint32_t gpio_num,
				       pdm_gpio_level_t level,
				       void *user_data)
{
	pdm_hw_mock_gpio_irq_state_t *state = user_data;

	if (!state)
		return;

	state->count++;
	state->gpio_num = gpio_num;
	state->level = level;
}

static int32_t pdm_hw_mock_selftest_gpio(void)
{
	pdm_hw_mock_gpio_irq_state_t irq_state = { 0 };
	pdm_gpio_config_t config = {
		.direction = PDM_GPIO_DIR_OUTPUT,
		.initial_level = PDM_GPIO_LEVEL_LOW,
		.edge = PDM_GPIO_EDGE_BOTH,
		.callback = pdm_hw_mock_gpio_irq_callback,
		.user_data = &irq_state,
	};
	pdm_gpio_direction_t direction;
	pdm_gpio_level_t level;
	int32_t ret;

	ret = pdm_hw_gpio_init(PDM_HW_MOCK_SELFTEST_GPIO, &config);
	if (ret != OSAL_SUCCESS)
		return pdm_hw_mock_expect_success("pdm_hw_gpio_init", ret);

	ret = pdm_hw_gpio_get_direction(PDM_HW_MOCK_SELFTEST_GPIO, &direction);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	if (direction != PDM_GPIO_DIR_OUTPUT) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_deinit;
	}

	ret = pdm_hw_gpio_get_level(PDM_HW_MOCK_SELFTEST_GPIO, &level);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	if (level != PDM_GPIO_LEVEL_LOW) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_deinit;
	}

	ret = pdm_hw_gpio_set_level(PDM_HW_MOCK_SELFTEST_GPIO, PDM_GPIO_LEVEL_HIGH);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	if (irq_state.count != 1U ||
	    irq_state.gpio_num != PDM_HW_MOCK_SELFTEST_GPIO ||
	    irq_state.level != PDM_GPIO_LEVEL_HIGH) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_deinit;
	}

	ret = pdm_hw_gpio_disable_interrupt(PDM_HW_MOCK_SELFTEST_GPIO);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	ret = pdm_hw_gpio_set_level(PDM_HW_MOCK_SELFTEST_GPIO, PDM_GPIO_LEVEL_LOW);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	if (irq_state.count != 1U) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_deinit;
	}

	ret = pdm_hw_gpio_enable_interrupt(PDM_HW_MOCK_SELFTEST_GPIO);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	ret = pdm_hw_gpio_set_level(PDM_HW_MOCK_SELFTEST_GPIO, PDM_GPIO_LEVEL_HIGH);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	if (irq_state.count != 2U || irq_state.level != PDM_GPIO_LEVEL_HIGH)
		ret = OSAL_ERR_INVALID_STATE;

out_deinit:
	(void)pdm_hw_gpio_deinit(PDM_HW_MOCK_SELFTEST_GPIO);
	return pdm_hw_mock_expect_success("gpio path", ret);
}

static int32_t pdm_hw_mock_selftest_pwm(void)
{
	pdm_pwm_config_t config = {
		.consumer = "lpf-mock-pwm0",
		.period_ns = 1000000U,
		.duty_ns = 250000U,
		.enabled = false,
		.polarity_inversed = false,
	};
	pdm_pwm_state_t state;
	pdm_hw_pwm_handle_t handle = NULL;
	int32_t ret;

	ret = pdm_hw_pwm_init(&config, &handle);
	if (ret != OSAL_SUCCESS)
		return pdm_hw_mock_expect_success("pdm_hw_pwm_init", ret);

	ret = pdm_hw_pwm_get_state(handle, &state);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	if (state.period_ns != config.period_ns ||
	    state.duty_ns != config.duty_ns || state.enabled) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_deinit;
	}

	state.duty_ns = 500000U;
	state.enabled = true;
	ret = pdm_hw_pwm_apply(handle, &state);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;

	ret = pdm_hw_pwm_get_state(handle, &state);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	if (state.duty_ns != 500000U || !state.enabled) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_deinit;
	}

	ret = pdm_hw_pwm_disable(handle);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	ret = pdm_hw_pwm_get_state(handle, &state);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	if (state.enabled) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_deinit;
	}

	ret = pdm_hw_pwm_enable(handle);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	ret = pdm_hw_pwm_get_state(handle, &state);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	if (!state.enabled)
		ret = OSAL_ERR_INVALID_STATE;

out_deinit:
	(void)pdm_hw_pwm_deinit(handle);
	return pdm_hw_mock_expect_success("pwm path", ret);
}

static int32_t pdm_hw_mock_selftest_can(void)
{
	pdm_can_config_t config = {
		.interface = "mock-can0",
		.baudrate = 500000U,
		.rx_timeout = 10U,
		.tx_timeout = 10U,
	};
	pdm_can_frame_t tx = {
		.can_id = 0x123U,
		.dlc = 3U,
		.data = { 0x11U, 0x22U, 0x33U },
	};
	pdm_can_frame_t rx;
	pdm_hw_transport_can_handle_t handle = NULL;
	int32_t ret;

	ret = pdm_hw_transport_can_init(&config, &handle);
	if (ret != OSAL_SUCCESS)
		return pdm_hw_mock_expect_success("pdm_hw_transport_can_init", ret);

	ret = pdm_hw_transport_can_set_filter(handle, tx.can_id, 0x7FFU);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	ret = pdm_hw_transport_can_send(handle, &tx);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;

	osal_memset(&rx, 0, sizeof(rx));
	ret = pdm_hw_transport_can_recv(handle, &rx, 10);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	if (rx.can_id != tx.can_id || rx.dlc != tx.dlc ||
	    rx.data[0] != tx.data[0] || rx.data[1] != tx.data[1] ||
	    rx.data[2] != tx.data[2])
		ret = OSAL_ERR_INVALID_STATE;

out_deinit:
	(void)pdm_hw_transport_can_deinit(handle);
	return pdm_hw_mock_expect_success("can path", ret);
}

static int32_t pdm_hw_mock_selftest_serial(void)
{
	pdm_serial_config_t config = {
		.baud_rate = 115200U,
		.data_bits = 8U,
		.stop_bits = 1U,
		.parity = PDM_SERIAL_PARITY_NONE,
		.flow_control = PDM_SERIAL_FLOW_NONE,
	};
	uint8_t tx[] = { 0x41U, 0x42U, 0x43U };
	uint8_t rx[sizeof(tx)];
	pdm_hw_transport_uart_handle_t handle = NULL;
	int32_t ret;

	ret = pdm_hw_transport_uart_open("mock-tty0", &config, &handle);
	if (ret != OSAL_SUCCESS)
		return pdm_hw_mock_expect_success("pdm_hw_transport_uart_open", ret);

	ret = pdm_hw_transport_uart_write(handle, tx, sizeof(tx), 10);
	if (ret != (int32_t)sizeof(tx)) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_close;
	}

	osal_memset(rx, 0, sizeof(rx));
	ret = pdm_hw_transport_uart_read(handle, rx, sizeof(rx), 10);
	if (ret != (int32_t)sizeof(rx)) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_close;
	}
	if (osal_memcmp(rx, tx, sizeof(tx)) != 0) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_close;
	}

	config.baud_rate = 57600U;
	ret = pdm_hw_transport_uart_set_config(handle, &config);
	if (ret != OSAL_SUCCESS)
		goto out_close;
	ret = pdm_hw_transport_uart_flush(handle);
	if (ret != OSAL_SUCCESS)
		goto out_close;
	ret = pdm_hw_transport_uart_read(handle, rx, sizeof(rx), 0);
	if (ret != 0)
		ret = OSAL_ERR_INVALID_STATE;

out_close:
	(void)pdm_hw_transport_uart_close(handle);
	return pdm_hw_mock_expect_success("serial path", ret);
}

static int32_t pdm_hw_mock_selftest_i2c(void)
{
	pdm_i2c_config_t config = {
		.device = "i2c-mock0",
		.timeout = 10U,
	};
	uint8_t write_buf[] = { 0x10U, 0x20U };
	uint8_t read_buf[4];
	pdm_hw_bus_i2c_handle_t handle = NULL;
	int32_t ret;

	ret = pdm_hw_bus_i2c_open(&config, &handle);
	if (ret != OSAL_SUCCESS)
		return pdm_hw_mock_expect_success("pdm_hw_bus_i2c_open", ret);

	ret = pdm_hw_bus_i2c_write(handle, 0x50U, write_buf, sizeof(write_buf));
	if (ret != OSAL_SUCCESS)
		goto out_close;

	osal_memset(read_buf, 0, sizeof(read_buf));
	ret = pdm_hw_bus_i2c_read(handle, 0x50U, read_buf, sizeof(read_buf));
	if (ret != OSAL_SUCCESS)
		goto out_close;
	if (read_buf[0] != 0U || read_buf[1] != 1U ||
	    read_buf[2] != 2U || read_buf[3] != 3U) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_close;
	}

	ret = pdm_hw_bus_i2c_write_reg(handle, 0x50U, 0x01U, write_buf,
				sizeof(write_buf));
	if (ret != OSAL_SUCCESS)
		goto out_close;

	osal_memset(read_buf, 0, sizeof(read_buf));
	ret = pdm_hw_bus_i2c_read_reg(handle, 0x50U, 0x01U, read_buf,
			       sizeof(read_buf));
	if (ret != OSAL_SUCCESS)
		goto out_close;
	if (read_buf[3] != 3U)
		ret = OSAL_ERR_INVALID_STATE;

out_close:
	pdm_hw_bus_i2c_close(handle);
	return pdm_hw_mock_expect_success("i2c path", ret);
}

static int32_t pdm_hw_mock_selftest_spi(void)
{
	pdm_spi_config_t config = {
		.device = "mock-spi0.0",
		.mode = PDM_SPI_MODE_0,
		.bits_per_word = 8U,
		.max_speed_hz = 1000000U,
		.timeout = 10U,
	};
	uint8_t tx[] = { 0xA5U, 0x5AU };
	uint8_t rx[sizeof(tx)];
	pdm_hw_bus_spi_handle_t handle = NULL;
	int32_t ret;

	ret = pdm_hw_bus_spi_open(&config, &handle);
	if (ret != OSAL_SUCCESS)
		return pdm_hw_mock_expect_success("pdm_hw_bus_spi_open", ret);

	osal_memset(rx, 0, sizeof(rx));
	ret = pdm_hw_bus_spi_transfer(handle, tx, rx, sizeof(tx));
	if (ret != OSAL_SUCCESS)
		goto out_close;
	if (osal_memcmp(rx, tx, sizeof(tx)) != 0) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_close;
	}

	osal_memset(rx, 0xFF, sizeof(rx));
	ret = pdm_hw_bus_spi_read(handle, rx, sizeof(rx));
	if (ret != OSAL_SUCCESS)
		goto out_close;
	if (rx[0] != 0U || rx[1] != 0U) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_close;
	}

	config.max_speed_hz = 2000000U;
	ret = pdm_hw_bus_spi_set_config(handle, &config);

out_close:
	pdm_hw_bus_spi_close(handle);
	return pdm_hw_mock_expect_success("spi path", ret);
}

static int32_t pdm_hw_mock_selftest_dynamic_resources(void)
{
	pdm_gpio_config_t gpio_config = {
		.direction = PDM_GPIO_DIR_OUTPUT,
		.initial_level = PDM_GPIO_LEVEL_LOW,
		.edge = PDM_GPIO_EDGE_NONE,
	};
	pdm_pwm_config_t pwm_config = {
		.consumer = "lpf-mock-pwm-scale",
		.period_ns = 1000000U,
		.duty_ns = 100000U,
	};
	pdm_i2c_config_t i2c_config = {
		.device = "i2c-mock-scale",
		.timeout = 10U,
	};
	pdm_spi_config_t spi_config = {
		.device = "mock-spi-scale",
		.mode = PDM_SPI_MODE_0,
		.bits_per_word = 8U,
		.max_speed_hz = 1000000U,
		.timeout = 10U,
	};
	pdm_can_config_t can_config = {
		.interface = "mock-can-scale",
		.baudrate = 500000U,
		.rx_timeout = 10U,
		.tx_timeout = 10U,
	};
	pdm_serial_config_t serial_config = {
		.baud_rate = 115200U,
		.data_bits = 8U,
		.stop_bits = 1U,
		.parity = PDM_SERIAL_PARITY_NONE,
		.flow_control = PDM_SERIAL_FLOW_NONE,
	};
	pdm_hw_mock_dynamic_resource_handles_t *handles;
	bool gpio_initialized = false;
	uint32_t i;
	int32_t ret = OSAL_SUCCESS;

	handles = osal_zalloc(sizeof(*handles));
	if (!handles)
		return pdm_hw_mock_expect_success("dynamic resource paths",
						  OSAL_ERR_NO_MEMORY);

	ret = pdm_hw_gpio_init(PDM_HW_MOCK_SELFTEST_HIGH_GPIO, &gpio_config);
	if (ret != OSAL_SUCCESS)
		goto out;
	gpio_initialized = true;

	for (i = 0; i < PDM_HW_MOCK_SELFTEST_PWM_HANDLES; i++) {
		ret = pdm_hw_pwm_init(&pwm_config, &handles->pwm_handles[i]);
		if (ret != OSAL_SUCCESS)
			goto out;
	}

	for (i = 0; i < PDM_HW_MOCK_SELFTEST_BUS_HANDLES; i++) {
		ret = pdm_hw_bus_i2c_open(&i2c_config,
					   &handles->i2c_handles[i]);
		if (ret != OSAL_SUCCESS)
			goto out;
		ret = pdm_hw_bus_spi_open(&spi_config,
					   &handles->spi_handles[i]);
		if (ret != OSAL_SUCCESS)
			goto out;
		ret = pdm_hw_transport_can_init(&can_config,
						 &handles->can_handles[i]);
		if (ret != OSAL_SUCCESS)
			goto out;
		ret = pdm_hw_transport_uart_open("mock-tty-scale",
						 &serial_config,
						 &handles->serial_handles[i]);
		if (ret != OSAL_SUCCESS)
			goto out;
	}

out:
	for (i = 0; i < PDM_HW_MOCK_SELFTEST_BUS_HANDLES; i++) {
		if (handles->serial_handles[i])
			(void)pdm_hw_transport_uart_close(
				handles->serial_handles[i]);
		if (handles->can_handles[i])
			(void)pdm_hw_transport_can_deinit(
				handles->can_handles[i]);
		if (handles->spi_handles[i])
			(void)pdm_hw_bus_spi_close(handles->spi_handles[i]);
		if (handles->i2c_handles[i])
			(void)pdm_hw_bus_i2c_close(handles->i2c_handles[i]);
	}
	for (i = 0; i < PDM_HW_MOCK_SELFTEST_PWM_HANDLES; i++) {
		if (handles->pwm_handles[i])
			(void)pdm_hw_pwm_deinit(handles->pwm_handles[i]);
	}
	if (gpio_initialized)
		(void)pdm_hw_gpio_deinit(PDM_HW_MOCK_SELFTEST_HIGH_GPIO);
	osal_free(handles);

	return pdm_hw_mock_expect_success("dynamic resource paths", ret);
}

static int32_t pdm_hw_mock_selftest_run(void)
{
	int32_t ret;

	ret = pdm_hw_mock_selftest_gpio();
	if (ret != OSAL_SUCCESS)
		return ret;
	ret = pdm_hw_mock_selftest_pwm();
	if (ret != OSAL_SUCCESS)
		return ret;
	ret = pdm_hw_mock_selftest_can();
	if (ret != OSAL_SUCCESS)
		return ret;
	ret = pdm_hw_mock_selftest_serial();
	if (ret != OSAL_SUCCESS)
		return ret;
	ret = pdm_hw_mock_selftest_i2c();
	if (ret != OSAL_SUCCESS)
		return ret;
	ret = pdm_hw_mock_selftest_spi();
	if (ret != OSAL_SUCCESS)
		return ret;
	return pdm_hw_mock_selftest_dynamic_resources();
}

static int __init pdm_hw_mock_selftest_init(void)
{
	int32_t ret;

	ret = pdm_hw_mock_selftest_run();
	if (ret != OSAL_SUCCESS)
		return pdm_hw_mock_status_to_errno(ret);

	pr_info("PDM:HW_SELFTEST: mock PDM HW path checks passed\n");
	return 0;
}

static void __exit pdm_hw_mock_selftest_exit(void)
{
	pr_info("PDM:HW_SELFTEST: unloaded\n");
}

module_init(pdm_hw_mock_selftest_init);
module_exit(pdm_hw_mock_selftest_exit);

MODULE_AUTHOR("PDM");
MODULE_DESCRIPTION("PDM HW mock backend self-test");
MODULE_LICENSE("GPL");
MODULE_SOFTDEP("pre: osal pdm_core");
