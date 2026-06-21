// SPDX-License-Identifier: MIT

#ifndef TEST_LPF_CONFIG_COMPARE_H
#define TEST_LPF_CONFIG_COMPARE_H

#include "lpf/config/lpf_config.h"

#define TEST_LPF_CONFIG_DEVICE_CAPACITY 8U

int test_lpf_config_string_equal(const char *left, const char *right);
int test_lpf_config_compare_devices(
	const lpf_config_device_config_t *left,
	const lpf_config_device_config_t *right);
int test_lpf_config_normalize_platform(
	const lpf_config_platform_config_t *platform,
	lpf_config_device_config_t *devices, uint32_t *count);
const lpf_config_platform_config_t *test_lpf_config_mock_static_config(void);

#endif /* TEST_LPF_CONFIG_COMPARE_H */
