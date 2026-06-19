/*
 * LPF PDI LED UAPI
 */

#ifndef PDI_LED_H
#define PDI_LED_H

#include <linux/ioctl.h>
#include <linux/types.h>

#define PDI_LED_ABI_VERSION 0x00010000U
#define PDI_LED_DEVICE_NAME "pdm_led"
#define PDI_LED_DEFAULT_DEVICE "/dev/pdm_led"

struct pdi_led_info {
	__u32 abi_version;
	__u32 module_version_major;
	__u32 module_version_minor;
	__u32 module_version_patch;
	__u32 open_count;
	__u32 max_devices;
};

struct pdi_led_state {
	__u32 index;
	__u32 brightness;
	__u32 max_brightness;
	__u32 enabled;
};

struct pdi_led_brightness {
	__u32 index;
	__u32 brightness;
};

#define PDI_LED_IOC_MAGIC 'L'
#define PDI_LED_IOC_GET_INFO \
	_IOR(PDI_LED_IOC_MAGIC, 0x01, struct pdi_led_info)
#define PDI_LED_IOC_GET_STATE \
	_IOWR(PDI_LED_IOC_MAGIC, 0x02, struct pdi_led_state)
#define PDI_LED_IOC_SET_BRIGHTNESS \
	_IOW(PDI_LED_IOC_MAGIC, 0x03, struct pdi_led_brightness)
#define PDI_LED_IOC_ENABLE _IOW(PDI_LED_IOC_MAGIC, 0x04, __u32)
#define PDI_LED_IOC_DISABLE _IOW(PDI_LED_IOC_MAGIC, 0x05, __u32)

#endif /* PDI_LED_H */
