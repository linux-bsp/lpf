// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_HW_H
#define PDM_HW_H

#include "pdm/hw/pdm_hw_gpio.h"
#include "pdm/hw/pdm_hw_pwm.h"
#include "pdm/hw/pdm_hw_can.h"
#include "pdm/hw/pdm_hw_uart.h"
#include "pdm/hw/pdm_hw_i2c.h"
#include "pdm/hw/pdm_hw_spi.h"

#define PDM_HW_VERSION_MAJOR 0x01
#define PDM_HW_VERSION_MINOR 0x00
#define PDM_HW_VERSION_PATCH 0x00

int32_t pdm_hw_runtime_init(void);
void pdm_hw_runtime_exit(void);

#endif /* PDM_HW_H */
