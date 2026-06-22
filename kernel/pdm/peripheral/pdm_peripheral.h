// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_PERIPHERAL_H
#define PDM_PERIPHERAL_H

int pdm_mcu_driver_init(void);
void pdm_mcu_driver_exit(void);
int pdm_led_driver_init(void);
void pdm_led_driver_exit(void);

#endif /* PDM_PERIPHERAL_H */
