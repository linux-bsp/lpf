// SPDX-License-Identifier: GPL-2.0

#include "pdm_mcu_transport.h"

#ifdef CONFIG_PDM_MCU_TRANSPORT_CAN
extern const pdm_mcu_transport_ops_t pdm_mcu_transport_can_ops;
#endif

#ifdef CONFIG_PDM_MCU_TRANSPORT_UART
extern const pdm_mcu_transport_ops_t pdm_mcu_transport_uart_ops;
#endif

const pdm_mcu_transport_ops_t *
pdm_mcu_transport_get(pdm_config_mcu_interface_t interface)
{
	switch (interface) {
	case PDM_CONFIG_MCU_INTERFACE_CAN:
#ifdef CONFIG_PDM_MCU_TRANSPORT_CAN
		return &pdm_mcu_transport_can_ops;
#else
		return NULL;
#endif
	case PDM_CONFIG_MCU_INTERFACE_SERIAL:
#ifdef CONFIG_PDM_MCU_TRANSPORT_UART
		return &pdm_mcu_transport_uart_ops;
#else
		return NULL;
#endif
	default:
		return NULL;
	}
}
