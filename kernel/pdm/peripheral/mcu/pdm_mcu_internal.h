// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_MCU_INTERNAL_H
#define PDM_MCU_INTERNAL_H

#include <linux/fs.h>
#include <linux/ktime.h>
#include <linux/mutex.h>
#include <linux/types.h>

#include "pdm/core/pdm_client.h"
#include "pdm/core/pdm_device.h"
#include "pdm/pdm_mcu.h"

#define PDM_MCU_DEFAULT_RX_TIMEOUT_MS 100U
#define PDM_MCU_UART_PATH_LEN 96U
#define PDM_MCU_CAN_IFNAME_LEN 16U

enum pdm_mcu_backend_type {
	PDM_MCU_BACKEND_MEMORY = 0,
	PDM_MCU_BACKEND_UART,
	PDM_MCU_BACKEND_CAN,
};

struct socket;
struct pdm_mcu_instance;

struct pdm_mcu_transport_ops {
	enum pdm_mcu_backend_type type;
	const char *name;
	u64 capability;
	int (*setup)(struct pdm_mcu_instance *inst);
	void (*cleanup)(struct pdm_mcu_instance *inst);
	int (*reset)(struct pdm_mcu_instance *inst);
	int (*command)(struct pdm_mcu_instance *inst,
			 struct pdm_mcu_command *command);
	int (*read_data)(struct pdm_mcu_instance *inst,
			 struct pdm_mcu_data *data);
	int (*write_data)(struct pdm_mcu_instance *inst,
			  const struct pdm_mcu_data *data);
};

struct pdm_mcu_instance {
	struct pdm_client client;
	struct pdm_device *pdm_dev;
	struct mutex lock;
	const struct pdm_mcu_transport_ops *ops;
	ktime_t start_time;
	bool online;
	u32 state;
	s32 last_error;
	union {
		struct {
			struct file *file;
			char path[PDM_MCU_UART_PATH_LEN];
			u32 baudrate;
			u32 rx_timeout_ms;
		} uart;
		struct {
			struct socket *sock;
			char ifname[PDM_MCU_CAN_IFNAME_LEN];
			u32 rx_timeout_ms;
			bool extended_id;
		} can;
	} transport;
};

const struct pdm_mcu_transport_ops *pdm_mcu_transport_select(const char *compatible);
extern const struct pdm_mcu_transport_ops pdm_mcu_uart_ops;
extern const struct pdm_mcu_transport_ops pdm_mcu_can_ops;

#endif /* PDM_MCU_INTERNAL_H */
