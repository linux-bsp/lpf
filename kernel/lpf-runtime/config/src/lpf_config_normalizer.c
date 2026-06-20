// SPDX-License-Identifier: GPL-2.0

#include "osal.h"
#include "lpf_config_normalizer.h"

typedef uint32_t (*lpf_config_normalizer_count_fn)(
	const lpf_config_platform_config_t *platform);
typedef const void *(*lpf_config_normalizer_entry_fn)(
	const lpf_config_platform_config_t *platform, uint32_t index);
typedef bool (*lpf_config_normalizer_enabled_fn)(const void *entry);

typedef struct {
	lpf_config_device_type_t type;
	lpf_config_normalizer_count_fn count;
	lpf_config_normalizer_entry_fn entry;
	lpf_config_normalizer_enabled_fn enabled;
} lpf_config_normalizer_descriptor_t;

static uint32_t lpf_config_normalizer_mcu_count(
	const lpf_config_platform_config_t *platform)
{
	return platform->mcu_count;
}

static const void *lpf_config_normalizer_mcu_entry(
	const lpf_config_platform_config_t *platform, uint32_t index)
{
	return lpf_config_hw_get_mcu(platform, index);
}

static bool lpf_config_normalizer_mcu_enabled(const void *entry)
{
	const lpf_config_mcu_entry_t *mcu = entry;

	return mcu && mcu->enabled;
}

static uint32_t lpf_config_normalizer_led_count(
	const lpf_config_platform_config_t *platform)
{
	return platform->led_count;
}

static const void *lpf_config_normalizer_led_entry(
	const lpf_config_platform_config_t *platform, uint32_t index)
{
	return lpf_config_hw_get_led(platform, index);
}

static bool lpf_config_normalizer_led_enabled(const void *entry)
{
	const lpf_config_led_entry_t *led = entry;

	return led && led->enabled;
}

static const lpf_config_normalizer_descriptor_t
	g_lpf_config_normalizer_descriptors[] = {
		{
			.type = LPF_CONFIG_DEVICE_TYPE_MCU,
			.count = lpf_config_normalizer_mcu_count,
			.entry = lpf_config_normalizer_mcu_entry,
			.enabled = lpf_config_normalizer_mcu_enabled,
		},
		{
			.type = LPF_CONFIG_DEVICE_TYPE_LED,
			.count = lpf_config_normalizer_led_count,
			.entry = lpf_config_normalizer_led_entry,
			.enabled = lpf_config_normalizer_led_enabled,
		},
	};

static void lpf_config_normalizer_clear_devices(
	lpf_config_device_config_t *devices, uint32_t count)
{
	uint32_t i;

	for (i = 0; i < count; i++) {
		devices[i].device_type = LPF_CONFIG_DEVICE_TYPE_INVALID;
		devices[i].index = 0;
		devices[i].entry = NULL;
	}
}

int32_t lpf_config_normalize_devices(
	const lpf_config_platform_config_t *platform,
	lpf_config_device_config_t *devices, uint32_t *count)
{
	uint32_t capacity;
	uint32_t out_index = 0;
	uint32_t desc_index;

	if (!platform || !count)
		return OSAL_ERR_INVALID_PARAM;

	capacity = devices ? *count : 0;
	if (devices && capacity > 0)
		lpf_config_normalizer_clear_devices(devices, capacity);

	for (desc_index = 0;
	     desc_index < OSAL_ARRAY_SIZE(g_lpf_config_normalizer_descriptors);
	     desc_index++) {
		const lpf_config_normalizer_descriptor_t *desc;
		uint32_t device_count;
		uint32_t i;

		desc = &g_lpf_config_normalizer_descriptors[desc_index];
		device_count = desc->count(platform);
		for (i = 0; i < device_count; i++) {
			const void *entry;

			entry = desc->entry(platform, i);
			if (!desc->enabled(entry))
				continue;

			if (devices && out_index < capacity) {
				devices[out_index].device_type = desc->type;
				devices[out_index].index = i;
				devices[out_index].entry = entry;
			}
			out_index++;
		}
	}

	*count = out_index;
	if (!devices)
		return OSAL_SUCCESS;

	if (out_index >= capacity)
		return OSAL_ERR_RESOURCE_LIMIT;

	devices[out_index].device_type = LPF_CONFIG_DEVICE_TYPE_INVALID;
	return OSAL_SUCCESS;
}
