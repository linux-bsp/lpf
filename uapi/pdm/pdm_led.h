/*
 * PDM LED UAPI
 */

#ifndef PDM_LED_UAPI_H
#define PDM_LED_UAPI_H

#include <linux/ioctl.h>
#include <linux/types.h>

#define PDM_LED_ABI_VERSION 0x00010000U
#define PDM_LED_DEVICE_NAME "pdm_led"

/* LED device type identifier - shared between kernel and userspace */
#define PDM_LED_DEVICE_TYPE 0x02

/* LED capability flags - shared between kernel and userspace */
#define PDM_LED_CAP_NONE        0ULL
#define PDM_LED_CAP_GPIO        (1ULL << 8)
#define PDM_LED_CAP_PWM         (1ULL << 9)
#define PDM_LED_CAP_USER_IOCTL  (1ULL << 16)

struct pdm_led_info {
	__u32 abi_version;
	__u32 module_version_major;
	__u32 module_version_minor;
	__u32 module_version_patch;
	__u32 open_count;
	__u32 max_devices;
};

struct pdm_led_state {
	__u32 index;
	__u32 brightness;
	__u32 max_brightness;
	__u32 enabled;
};

struct pdm_led_brightness {
	__u32 index;
	__u32 brightness;
};

#define PDM_LED_IOC_MAGIC 'L'
#define PDM_LED_IOC_GET_INFO \
	_IOR(PDM_LED_IOC_MAGIC, 0x01, struct pdm_led_info)
#define PDM_LED_IOC_GET_STATE \
	_IOWR(PDM_LED_IOC_MAGIC, 0x02, struct pdm_led_state)
#define PDM_LED_IOC_SET_BRIGHTNESS \
	_IOW(PDM_LED_IOC_MAGIC, 0x03, struct pdm_led_brightness)
#define PDM_LED_IOC_ENABLE _IOW(PDM_LED_IOC_MAGIC, 0x04, __u32)
#define PDM_LED_IOC_DISABLE _IOW(PDM_LED_IOC_MAGIC, 0x05, __u32)

#endif /* PDM_LED_UAPI_H */
