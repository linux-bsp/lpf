// SPDX-License-Identifier: GPL-2.0

#include "pdm/compat/pdm_compat_can.h"
#include "pdm/compat/pdm_compat_gpio.h"
#include "pdm/compat/pdm_compat_i2c.h"
#include "pdm/compat/pdm_compat_pwm.h"
#include "pdm/compat/pdm_compat_serial.h"
#include "pdm/compat/pdm_compat_spi.h"
#include "pdm/soc/pdm_soc_adapter.h"

const pdm_soc_adapter_t g_lpf_soc_generic_linux_adapter = {
	.name = "generic-linux",
	.gpio = {
		.request = pdm_compat_gpio_request,
		.free = pdm_compat_gpio_free,
		.set_direction = pdm_compat_gpio_set_direction,
		.set_level = pdm_compat_gpio_set_level,
		.get_level = pdm_compat_gpio_get_level,
		.request_interrupt = pdm_compat_gpio_request_interrupt,
		.free_interrupt = pdm_compat_gpio_free_interrupt,
		.enable_interrupt = pdm_compat_gpio_enable_interrupt,
		.disable_interrupt = pdm_compat_gpio_disable_interrupt,
	},
	.pwm = {
		.get = pdm_compat_pwm_get,
		.put = pdm_compat_pwm_put,
		.apply = pdm_compat_pwm_apply,
		.get_state = pdm_compat_pwm_get_state,
		.enable = pdm_compat_pwm_enable,
		.disable = pdm_compat_pwm_disable,
	},
	.i2c = {
		.open = pdm_compat_i2c_open,
		.close = pdm_compat_i2c_close,
		.transfer = pdm_compat_i2c_transfer,
	},
	.spi = {
		.open = pdm_compat_spi_open,
		.close = pdm_compat_spi_close,
		.transfer = pdm_compat_spi_transfer,
		.set_config = pdm_compat_spi_set_config,
	},
	.can = {
		.init = pdm_compat_can_init,
		.deinit = pdm_compat_can_deinit,
		.send = pdm_compat_can_send,
		.recv = pdm_compat_can_recv,
		.set_filter = pdm_compat_can_set_filter,
	},
	.serial = {
		.open = pdm_compat_serial_open,
		.close = pdm_compat_serial_close,
		.write = pdm_compat_serial_write,
		.read = pdm_compat_serial_read,
		.flush = pdm_compat_serial_flush,
		.set_config = pdm_compat_serial_set_config,
	},
};
