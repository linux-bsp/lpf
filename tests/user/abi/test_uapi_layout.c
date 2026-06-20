// SPDX-License-Identifier: MIT

#include <stddef.h>

#include "lpf/lpf_ctl.h"
#include "lpf/lpf_led.h"
#include "lpf/lpf_mcu.h"

#define ABI_ASSERT(condition, name) _Static_assert(condition, #name)
#define ABI_SIZE(type, size) \
	_Static_assert(sizeof(type) == (size), "sizeof(" #type ")")
#define ABI_OFFSET(type, member, offset) \
	_Static_assert(offsetof(type, member) == (offset), \
		       "offsetof(" #type "." #member ")")

ABI_ASSERT(LPF_CTL_ABI_VERSION == 0x00010000U, ctl_abi_version);
ABI_ASSERT(LPF_MCU_ABI_VERSION == 0x00010000U, mcu_abi_version);
ABI_ASSERT(LPF_LED_ABI_VERSION == 0x00010000U, led_abi_version);

ABI_SIZE(struct lpf_ctl_info, 24);
ABI_OFFSET(struct lpf_ctl_info, abi_version, 0);
ABI_OFFSET(struct lpf_ctl_info, module_version_major, 4);
ABI_OFFSET(struct lpf_ctl_info, module_version_minor, 8);
ABI_OFFSET(struct lpf_ctl_info, module_version_patch, 12);
ABI_OFFSET(struct lpf_ctl_info, open_count, 16);
ABI_OFFSET(struct lpf_ctl_info, device_count, 20);

ABI_SIZE(struct lpf_ctl_device_info, 160);
ABI_OFFSET(struct lpf_ctl_device_info, type, 0);
ABI_OFFSET(struct lpf_ctl_device_info, index, 4);
ABI_OFFSET(struct lpf_ctl_device_info, state, 8);
ABI_OFFSET(struct lpf_ctl_device_info, last_error, 12);
ABI_OFFSET(struct lpf_ctl_device_info, error_count, 16);
ABI_OFFSET(struct lpf_ctl_device_info, capabilities, 24);
ABI_OFFSET(struct lpf_ctl_device_info, name, 32);
ABI_OFFSET(struct lpf_ctl_device_info, driver_name, 96);

ABI_SIZE(struct lpf_ctl_device_query, 176);
ABI_OFFSET(struct lpf_ctl_device_query, match_index, 0);
ABI_OFFSET(struct lpf_ctl_device_query, reserved, 4);
ABI_OFFSET(struct lpf_ctl_device_query, required_capabilities, 8);
ABI_OFFSET(struct lpf_ctl_device_query, info, 16);

ABI_SIZE(struct lpf_ctl_device_name_query, 224);
ABI_OFFSET(struct lpf_ctl_device_name_query, name, 0);
ABI_OFFSET(struct lpf_ctl_device_name_query, info, 64);

ABI_SIZE(struct lpf_mcu_info, 24);
ABI_SIZE(struct lpf_mcu_version, 40);
ABI_SIZE(struct lpf_mcu_status, 40);
ABI_SIZE(struct lpf_mcu_command, 528);
ABI_SIZE(struct lpf_mcu_data, 268);

ABI_SIZE(struct lpf_led_info, 24);
ABI_SIZE(struct lpf_led_state, 16);
ABI_SIZE(struct lpf_led_brightness, 8);

ABI_ASSERT(LPF_CTL_IOC_GET_INFO ==
		   _IOR(LPF_CTL_IOC_MAGIC, 0x01, struct lpf_ctl_info),
	   ctl_get_info_ioctl);
ABI_ASSERT(LPF_CTL_IOC_GET_DEVICE ==
		   _IOWR(LPF_CTL_IOC_MAGIC, 0x02,
			 struct lpf_ctl_device_query),
	   ctl_get_device_ioctl);
ABI_ASSERT(LPF_CTL_IOC_GET_DEVICE_BY_NAME ==
		   _IOWR(LPF_CTL_IOC_MAGIC, 0x03,
			 struct lpf_ctl_device_name_query),
	   ctl_get_device_by_name_ioctl);
ABI_ASSERT(LPF_CTL_IOC_GET_DEVICE_BY_CAPABILITY ==
		   _IOWR(LPF_CTL_IOC_MAGIC, 0x04,
			 struct lpf_ctl_device_query),
	   ctl_get_device_by_capability_ioctl);

ABI_ASSERT(LPF_MCU_IOC_GET_INFO ==
		   _IOR(LPF_MCU_IOC_MAGIC, 0x01, struct lpf_mcu_info),
	   mcu_get_info_ioctl);
ABI_ASSERT(LPF_MCU_IOC_GET_VERSION ==
		   _IOWR(LPF_MCU_IOC_MAGIC, 0x02, struct lpf_mcu_version),
	   mcu_get_version_ioctl);
ABI_ASSERT(LPF_MCU_IOC_GET_STATUS ==
		   _IOWR(LPF_MCU_IOC_MAGIC, 0x03, struct lpf_mcu_status),
	   mcu_get_status_ioctl);
ABI_ASSERT(LPF_MCU_IOC_RESET == _IOW(LPF_MCU_IOC_MAGIC, 0x04, __u32),
	   mcu_reset_ioctl);
ABI_ASSERT(LPF_MCU_IOC_COMMAND ==
		   _IOWR(LPF_MCU_IOC_MAGIC, 0x05, struct lpf_mcu_command),
	   mcu_command_ioctl);
ABI_ASSERT(LPF_MCU_IOC_READ_DATA ==
		   _IOWR(LPF_MCU_IOC_MAGIC, 0x06, struct lpf_mcu_data),
	   mcu_read_data_ioctl);
ABI_ASSERT(LPF_MCU_IOC_WRITE_DATA ==
		   _IOW(LPF_MCU_IOC_MAGIC, 0x07, struct lpf_mcu_data),
	   mcu_write_data_ioctl);

ABI_ASSERT(LPF_LED_IOC_GET_INFO ==
		   _IOR(LPF_LED_IOC_MAGIC, 0x01, struct lpf_led_info),
	   led_get_info_ioctl);
ABI_ASSERT(LPF_LED_IOC_GET_STATE ==
		   _IOWR(LPF_LED_IOC_MAGIC, 0x02, struct lpf_led_state),
	   led_get_state_ioctl);
ABI_ASSERT(LPF_LED_IOC_SET_BRIGHTNESS ==
		   _IOW(LPF_LED_IOC_MAGIC, 0x03,
			struct lpf_led_brightness),
	   led_set_brightness_ioctl);
ABI_ASSERT(LPF_LED_IOC_ENABLE == _IOW(LPF_LED_IOC_MAGIC, 0x04, __u32),
	   led_enable_ioctl);
ABI_ASSERT(LPF_LED_IOC_DISABLE == _IOW(LPF_LED_IOC_MAGIC, 0x05, __u32),
	   led_disable_ioctl);

int main(void)
{
	return 0;
}
