/*
 * PDM MCU UAPI
 *
 * This header defines the ioctl ABI shared by /dev/pdm/mcuN and the
 * userspace PDI MCU client.
 */

#ifndef PDM_MCU_UAPI_H
#define PDM_MCU_UAPI_H

#include <linux/ioctl.h>
#include <linux/types.h>

#define PDM_MCU_ABI_VERSION 0x00020000U
#define PDM_MCU_DEVICE_NAME "pdm_mcu"
#define PDM_MCU_MAX_TRANSFER_SIZE 256U

/* MCU device type identifier - shared between kernel and userspace */
#define PDM_MCU_DEVICE_TYPE 0x01

/* MCU capability flags - shared between kernel and userspace */
#define PDM_MCU_CAP_NONE           0ULL
#define PDM_MCU_CAP_TRANSPORT_CAN  (1ULL << 0)
#define PDM_MCU_CAP_TRANSPORT_UART (1ULL << 1)
#define PDM_MCU_CAP_TRANSPORT_I2C  (1ULL << 2)
#define PDM_MCU_CAP_TRANSPORT_SPI  (1ULL << 3)
#define PDM_MCU_CAP_USER_IOCTL     (1ULL << 16)

enum pdm_mcu_state {
	PDM_MCU_STATE_UNINITIALIZED = 0x00,
	PDM_MCU_STATE_INIT = 0x01,
	PDM_MCU_STATE_READY = 0x02,
	PDM_MCU_STATE_BUSY = 0x03,
	PDM_MCU_STATE_ERROR = 0x04,
	PDM_MCU_STATE_OFFLINE = 0x05,
};

struct pdm_mcu_info {
	__u32 abi_version;
	__u32 module_version_major;
	__u32 module_version_minor;
	__u32 module_version_patch;
	__u32 open_count;
	__u32 max_devices;
};

struct pdm_mcu_version {
	__u8 major;
	__u8 minor;
	__u8 patch;
	__u8 build;
	char version_string[32];
};

struct pdm_mcu_status {
	__u32 online;
	__u32 state;
	__u32 uptime_sec;
	__u32 error_code;
	__s32 temperature_milli_celsius;
	__u32 voltage_mv;
	__u64 timestamp_us;
};

#define PDM_MCU_CMD_F_NEED_RESPONSE (1U << 0)

struct pdm_mcu_command {
	__u32 command;
	__u32 flags;
	__u32 tx_len;
	__u32 rx_len;
	__u32 actual_rx_len;
	__u8 tx_data[PDM_MCU_MAX_TRANSFER_SIZE];
	__u8 rx_data[PDM_MCU_MAX_TRANSFER_SIZE];
};

#define PDM_MCU_IOC_MAGIC 'M'
#define PDM_MCU_IOC_GET_INFO \
	_IOR(PDM_MCU_IOC_MAGIC, 0x01, struct pdm_mcu_info)
#define PDM_MCU_IOC_GET_VERSION \
	_IOR(PDM_MCU_IOC_MAGIC, 0x02, struct pdm_mcu_version)
#define PDM_MCU_IOC_GET_STATUS \
	_IOR(PDM_MCU_IOC_MAGIC, 0x03, struct pdm_mcu_status)
#define PDM_MCU_IOC_RESET _IO(PDM_MCU_IOC_MAGIC, 0x04)
#define PDM_MCU_IOC_COMMAND \
	_IOWR(PDM_MCU_IOC_MAGIC, 0x05, struct pdm_mcu_command)

#endif /* PDM_MCU_UAPI_H */
