// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>

#include "osal.h"
#include "hal_can.h"
#include "hal_gpio.h"
#include "hal_i2c.h"
#include "hal_pwm.h"
#include "hal_serial.h"
#include "hal_spi.h"

#define HAL_MOCK_SELFTEST_GPIO 7U

typedef struct {
	uint32_t count;
	uint32_t gpio_num;
	hal_gpio_level_t level;
} hal_mock_gpio_irq_state_t;

static int hal_mock_status_to_errno(int32_t status)
{
	if (status == OSAL_SUCCESS)
		return 0;
	if (status > 0 && status < 4096)
		return -status;
	return -EINVAL;
}

static int32_t hal_mock_expect_success(const char *name, int32_t status)
{
	if (status == OSAL_SUCCESS)
		return OSAL_SUCCESS;

	pr_err("LPF:HAL_SELFTEST: %s failed: %d\n", name, status);
	return status;
}

static void hal_mock_gpio_irq_callback(uint32_t gpio_num,
				       hal_gpio_level_t level,
				       void *user_data)
{
	hal_mock_gpio_irq_state_t *state = user_data;

	if (!state)
		return;

	state->count++;
	state->gpio_num = gpio_num;
	state->level = level;
}

static int32_t hal_mock_selftest_gpio(void)
{
	hal_mock_gpio_irq_state_t irq_state = { 0 };
	hal_gpio_config_t config = {
		.direction = HAL_GPIO_DIR_OUTPUT,
		.initial_level = HAL_GPIO_LEVEL_LOW,
		.edge = HAL_GPIO_EDGE_BOTH,
		.callback = hal_mock_gpio_irq_callback,
		.user_data = &irq_state,
	};
	hal_gpio_direction_t direction;
	hal_gpio_level_t level;
	int32_t ret;

	ret = hal_gpio_init(HAL_MOCK_SELFTEST_GPIO, &config);
	if (ret != OSAL_SUCCESS)
		return hal_mock_expect_success("hal_gpio_init", ret);

	ret = hal_gpio_get_direction(HAL_MOCK_SELFTEST_GPIO, &direction);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	if (direction != HAL_GPIO_DIR_OUTPUT) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_deinit;
	}

	ret = hal_gpio_get_level(HAL_MOCK_SELFTEST_GPIO, &level);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	if (level != HAL_GPIO_LEVEL_LOW) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_deinit;
	}

	ret = hal_gpio_set_level(HAL_MOCK_SELFTEST_GPIO, HAL_GPIO_LEVEL_HIGH);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	if (irq_state.count != 1U ||
	    irq_state.gpio_num != HAL_MOCK_SELFTEST_GPIO ||
	    irq_state.level != HAL_GPIO_LEVEL_HIGH) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_deinit;
	}

	ret = hal_gpio_disable_interrupt(HAL_MOCK_SELFTEST_GPIO);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	ret = hal_gpio_set_level(HAL_MOCK_SELFTEST_GPIO, HAL_GPIO_LEVEL_LOW);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	if (irq_state.count != 1U) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_deinit;
	}

	ret = hal_gpio_enable_interrupt(HAL_MOCK_SELFTEST_GPIO);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	ret = hal_gpio_set_level(HAL_MOCK_SELFTEST_GPIO, HAL_GPIO_LEVEL_HIGH);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	if (irq_state.count != 2U || irq_state.level != HAL_GPIO_LEVEL_HIGH)
		ret = OSAL_ERR_INVALID_STATE;

out_deinit:
	(void)hal_gpio_deinit(HAL_MOCK_SELFTEST_GPIO);
	return hal_mock_expect_success("gpio path", ret);
}

static int32_t hal_mock_selftest_pwm(void)
{
	hal_pwm_config_t config = {
		.consumer = "lpf-mock-pwm0",
		.period_ns = 1000000U,
		.duty_ns = 250000U,
		.enabled = false,
		.polarity_inversed = false,
	};
	hal_pwm_state_t state;
	hal_pwm_handle_t handle = NULL;
	int32_t ret;

	ret = hal_pwm_init(&config, &handle);
	if (ret != OSAL_SUCCESS)
		return hal_mock_expect_success("hal_pwm_init", ret);

	ret = hal_pwm_get_state(handle, &state);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	if (state.period_ns != config.period_ns ||
	    state.duty_ns != config.duty_ns || state.enabled) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_deinit;
	}

	state.duty_ns = 500000U;
	state.enabled = true;
	ret = hal_pwm_apply(handle, &state);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;

	ret = hal_pwm_get_state(handle, &state);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	if (state.duty_ns != 500000U || !state.enabled) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_deinit;
	}

	ret = hal_pwm_disable(handle);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	ret = hal_pwm_get_state(handle, &state);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	if (state.enabled) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_deinit;
	}

	ret = hal_pwm_enable(handle);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	ret = hal_pwm_get_state(handle, &state);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	if (!state.enabled)
		ret = OSAL_ERR_INVALID_STATE;

out_deinit:
	(void)hal_pwm_deinit(handle);
	return hal_mock_expect_success("pwm path", ret);
}

static int32_t hal_mock_selftest_can(void)
{
	hal_can_config_t config = {
		.interface = "mock-can0",
		.baudrate = 500000U,
		.rx_timeout = 10U,
		.tx_timeout = 10U,
	};
	hal_can_frame_t tx = {
		.can_id = 0x123U,
		.dlc = 3U,
		.data = { 0x11U, 0x22U, 0x33U },
	};
	hal_can_frame_t rx;
	hal_can_handle_t handle = NULL;
	int32_t ret;

	ret = hal_can_init(&config, &handle);
	if (ret != OSAL_SUCCESS)
		return hal_mock_expect_success("hal_can_init", ret);

	ret = hal_can_set_filter(handle, tx.can_id, 0x7FFU);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	ret = hal_can_send(handle, &tx);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;

	osal_memset(&rx, 0, sizeof(rx));
	ret = hal_can_recv(handle, &rx, 10);
	if (ret != OSAL_SUCCESS)
		goto out_deinit;
	if (rx.can_id != tx.can_id || rx.dlc != tx.dlc ||
	    rx.data[0] != tx.data[0] || rx.data[1] != tx.data[1] ||
	    rx.data[2] != tx.data[2])
		ret = OSAL_ERR_INVALID_STATE;

out_deinit:
	(void)hal_can_deinit(handle);
	return hal_mock_expect_success("can path", ret);
}

static int32_t hal_mock_selftest_serial(void)
{
	hal_serial_config_t config = {
		.baud_rate = 115200U,
		.data_bits = 8U,
		.stop_bits = 1U,
		.parity = HAL_SERIAL_PARITY_NONE,
		.flow_control = HAL_SERIAL_FLOW_NONE,
	};
	uint8_t tx[] = { 0x41U, 0x42U, 0x43U };
	uint8_t rx[sizeof(tx)];
	hal_serial_handle_t handle = NULL;
	int32_t ret;

	ret = hal_serial_open("mock-tty0", &config, &handle);
	if (ret != OSAL_SUCCESS)
		return hal_mock_expect_success("hal_serial_open", ret);

	ret = hal_serial_write(handle, tx, sizeof(tx), 10);
	if (ret != (int32_t)sizeof(tx)) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_close;
	}

	osal_memset(rx, 0, sizeof(rx));
	ret = hal_serial_read(handle, rx, sizeof(rx), 10);
	if (ret != (int32_t)sizeof(rx)) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_close;
	}
	if (osal_memcmp(rx, tx, sizeof(tx)) != 0) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_close;
	}

	config.baud_rate = 57600U;
	ret = hal_serial_set_config(handle, &config);
	if (ret != OSAL_SUCCESS)
		goto out_close;
	ret = hal_serial_flush(handle);
	if (ret != OSAL_SUCCESS)
		goto out_close;
	ret = hal_serial_read(handle, rx, sizeof(rx), 0);
	if (ret != 0)
		ret = OSAL_ERR_INVALID_STATE;

out_close:
	(void)hal_serial_close(handle);
	return hal_mock_expect_success("serial path", ret);
}

static int32_t hal_mock_selftest_i2c(void)
{
	hal_i2c_config_t config = {
		.device = "i2c-mock0",
		.timeout = 10U,
	};
	uint8_t write_buf[] = { 0x10U, 0x20U };
	uint8_t read_buf[4];
	hal_i2c_handle_t handle = NULL;
	int32_t ret;

	ret = hal_i2c_open(&config, &handle);
	if (ret != OSAL_SUCCESS)
		return hal_mock_expect_success("hal_i2c_open", ret);

	ret = hal_i2c_write(handle, 0x50U, write_buf, sizeof(write_buf));
	if (ret != OSAL_SUCCESS)
		goto out_close;

	osal_memset(read_buf, 0, sizeof(read_buf));
	ret = hal_i2c_read(handle, 0x50U, read_buf, sizeof(read_buf));
	if (ret != OSAL_SUCCESS)
		goto out_close;
	if (read_buf[0] != 0U || read_buf[1] != 1U ||
	    read_buf[2] != 2U || read_buf[3] != 3U) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_close;
	}

	ret = hal_i2c_write_reg(handle, 0x50U, 0x01U, write_buf,
				sizeof(write_buf));
	if (ret != OSAL_SUCCESS)
		goto out_close;

	osal_memset(read_buf, 0, sizeof(read_buf));
	ret = hal_i2c_read_reg(handle, 0x50U, 0x01U, read_buf,
			       sizeof(read_buf));
	if (ret != OSAL_SUCCESS)
		goto out_close;
	if (read_buf[3] != 3U)
		ret = OSAL_ERR_INVALID_STATE;

out_close:
	hal_i2c_close(handle);
	return hal_mock_expect_success("i2c path", ret);
}

static int32_t hal_mock_selftest_spi(void)
{
	hal_spi_config_t config = {
		.device = "mock-spi0.0",
		.mode = SPI_MODE_0,
		.bits_per_word = 8U,
		.max_speed_hz = 1000000U,
		.timeout = 10U,
	};
	uint8_t tx[] = { 0xA5U, 0x5AU };
	uint8_t rx[sizeof(tx)];
	hal_spi_handle_t handle = NULL;
	int32_t ret;

	ret = hal_spi_open(&config, &handle);
	if (ret != OSAL_SUCCESS)
		return hal_mock_expect_success("hal_spi_open", ret);

	osal_memset(rx, 0, sizeof(rx));
	ret = hal_spi_transfer(handle, tx, rx, sizeof(tx));
	if (ret != OSAL_SUCCESS)
		goto out_close;
	if (osal_memcmp(rx, tx, sizeof(tx)) != 0) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_close;
	}

	osal_memset(rx, 0xFF, sizeof(rx));
	ret = hal_spi_read(handle, rx, sizeof(rx));
	if (ret != OSAL_SUCCESS)
		goto out_close;
	if (rx[0] != 0U || rx[1] != 0U) {
		ret = OSAL_ERR_INVALID_STATE;
		goto out_close;
	}

	config.max_speed_hz = 2000000U;
	ret = hal_spi_set_config(handle, &config);

out_close:
	hal_spi_close(handle);
	return hal_mock_expect_success("spi path", ret);
}

static int32_t hal_mock_selftest_run(void)
{
	int32_t ret;

	ret = hal_mock_selftest_gpio();
	if (ret != OSAL_SUCCESS)
		return ret;
	ret = hal_mock_selftest_pwm();
	if (ret != OSAL_SUCCESS)
		return ret;
	ret = hal_mock_selftest_can();
	if (ret != OSAL_SUCCESS)
		return ret;
	ret = hal_mock_selftest_serial();
	if (ret != OSAL_SUCCESS)
		return ret;
	ret = hal_mock_selftest_i2c();
	if (ret != OSAL_SUCCESS)
		return ret;
	return hal_mock_selftest_spi();
}

static int __init hal_mock_selftest_init(void)
{
	int32_t ret;

	ret = hal_mock_selftest_run();
	if (ret != OSAL_SUCCESS)
		return hal_mock_status_to_errno(ret);

	pr_info("LPF:HAL_SELFTEST: mock HAL path checks passed\n");
	return 0;
}

static void __exit hal_mock_selftest_exit(void)
{
	pr_info("LPF:HAL_SELFTEST: unloaded\n");
}

module_init(hal_mock_selftest_init);
module_exit(hal_mock_selftest_exit);

MODULE_AUTHOR("LPF");
MODULE_DESCRIPTION("LPF HAL mock backend self-test");
MODULE_LICENSE("GPL");
MODULE_SOFTDEP("pre: osal lpf_core hal");
