/*
 * LPF PDI MCU UAPI
 *
 * This header defines the ioctl ABI shared by /dev/pdm_mcu and the
 * userspace PDI MCU client.
 */

#ifndef PDI_MCU_H
#define PDI_MCU_H

#include <linux/ioctl.h>
#include <linux/types.h>

#define PDI_MCU_ABI_VERSION 0x00010000U
#define PDI_MCU_DEVICE_NAME "pdm_mcu"
#define PDI_MCU_DEFAULT_DEVICE "/dev/pdm_mcu"
#define PDI_MCU_MAX_TRANSFER_SIZE 256U
#define PDI_MCU_MAX_WRITE_SIZE 248U

enum pdi_mcu_state {
	PDI_MCU_STATE_UNINITIALIZED = 0x00,
	PDI_MCU_STATE_INIT = 0x01,
	PDI_MCU_STATE_READY = 0x02,
	PDI_MCU_STATE_BUSY = 0x03,
	PDI_MCU_STATE_ERROR = 0x04,
	PDI_MCU_STATE_OFFLINE = 0x05,
};

struct pdi_mcu_info {
	__u32 abi_version;
	__u32 module_version_major;
	__u32 module_version_minor;
	__u32 module_version_patch;
	__u32 open_count;
	__u32 max_devices;
};

struct pdi_mcu_version {
	__u32 index;
	__u8 major;
	__u8 minor;
	__u8 patch;
	__u8 build;
	char version_string[32];
};

struct pdi_mcu_status {
	__u32 index;
	__u32 online;
	__u32 state;
	__u32 uptime_sec;
	__u32 error_code;
	__s32 temperature_milli_celsius;
	__u32 voltage_mv;
	__u64 timestamp_us;
};

struct pdi_mcu_command {
	__u32 index;
	__u32 command;
	__u32 tx_len;
	__u32 rx_len;
	__u8 tx_data[PDI_MCU_MAX_TRANSFER_SIZE];
	__u8 rx_data[PDI_MCU_MAX_TRANSFER_SIZE];
};

struct pdi_mcu_data {
	__u32 index;
	__u32 address;
	__u32 len;
	__u8 data[PDI_MCU_MAX_TRANSFER_SIZE];
};

#define PDI_MCU_IOC_MAGIC 'M'
#define PDI_MCU_IOC_GET_INFO \
	_IOR(PDI_MCU_IOC_MAGIC, 0x01, struct pdi_mcu_info)
#define PDI_MCU_IOC_GET_VERSION \
	_IOWR(PDI_MCU_IOC_MAGIC, 0x02, struct pdi_mcu_version)
#define PDI_MCU_IOC_GET_STATUS \
	_IOWR(PDI_MCU_IOC_MAGIC, 0x03, struct pdi_mcu_status)
#define PDI_MCU_IOC_RESET _IOW(PDI_MCU_IOC_MAGIC, 0x04, __u32)
#define PDI_MCU_IOC_COMMAND \
	_IOWR(PDI_MCU_IOC_MAGIC, 0x05, struct pdi_mcu_command)
#define PDI_MCU_IOC_READ_DATA \
	_IOWR(PDI_MCU_IOC_MAGIC, 0x06, struct pdi_mcu_data)
#define PDI_MCU_IOC_WRITE_DATA \
	_IOW(PDI_MCU_IOC_MAGIC, 0x07, struct pdi_mcu_data)

#endif /* PDI_MCU_H */
