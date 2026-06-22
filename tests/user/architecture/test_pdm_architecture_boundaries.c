// SPDX-License-Identifier: MIT

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifndef SDK_SOURCE_DIR
#define SDK_SOURCE_DIR "."
#endif

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

typedef struct {
	const char *name;
	const char *path;
} source_file_t;

static char *read_source_file(const char *relative_path)
{
	char path[1024];
	char *buffer;
	FILE *file;
	long size;
	size_t read_size;
	int ret;

	ret = snprintf(path, sizeof(path), "%s/%s", SDK_SOURCE_DIR,
		       relative_path);
	if (ret < 0 || (size_t)ret >= sizeof(path))
		return NULL;

	file = fopen(path, "rb");
	if (!file)
		return NULL;

	if (fseek(file, 0, SEEK_END) != 0) {
		fclose(file);
		return NULL;
	}

	size = ftell(file);
	if (size < 0) {
		fclose(file);
		return NULL;
	}

	if (fseek(file, 0, SEEK_SET) != 0) {
		fclose(file);
		return NULL;
	}

	buffer = malloc((size_t)size + 1U);
	if (!buffer) {
		fclose(file);
		return NULL;
	}

	read_size = fread(buffer, 1, (size_t)size, file);
	fclose(file);
	if (read_size != (size_t)size) {
		free(buffer);
		return NULL;
	}

	buffer[size] = '\0';
	return buffer;
}

static int expect_contains(const char *file_name, const char *content,
			   const char *needle)
{
	if (strstr(content, needle))
		return 0;

	fprintf(stderr, "%s: missing required token: %s\n", file_name,
		needle);
	return 1;
}

static int expect_not_contains(const char *file_name, const char *content,
			       const char *needle)
{
	if (!strstr(content, needle))
		return 0;

	fprintf(stderr, "%s: forbidden token present: %s\n", file_name,
		needle);
	return 1;
}

static int expect_ordered_contains(const char *file_name, const char *content,
				   const char *first, const char *second)
{
	const char *first_pos;
	const char *second_pos;

	first_pos = strstr(content, first);
	second_pos = strstr(content, second);
	if (first_pos && second_pos && first_pos < second_pos)
		return 0;

	fprintf(stderr, "%s: expected %s before %s\n", file_name, first,
		second);
	return 1;
}

static int expect_path_absent(const char *relative_path)
{
	char path[1024];
	struct stat st;
	int ret;

	ret = snprintf(path, sizeof(path), "%s/%s", SDK_SOURCE_DIR,
		       relative_path);
	if (ret < 0 || (size_t)ret >= sizeof(path))
		return 1;

	if (stat(path, &st) != 0)
		return errno == ENOENT ? 0 : 1;

	fprintf(stderr, "forbidden path present: %s\n", relative_path);
	return 1;
}

static int expect_path_present(const char *relative_path)
{
	char path[1024];
	struct stat st;
	int ret;

	ret = snprintf(path, sizeof(path), "%s/%s", SDK_SOURCE_DIR,
		       relative_path);
	if (ret < 0 || (size_t)ret >= sizeof(path))
		return 1;

	if (stat(path, &st) == 0)
		return 0;

	fprintf(stderr, "required path missing: %s\n", relative_path);
	return 1;
}

static int expect_range_not_contains(const char *file_name,
				     const char *content,
				     const char *start_token,
				     const char *end_token,
				     const char *needle)
{
	const char *start;
	const char *end;
	const char *found;

	start = strstr(content, start_token);
	if (!start) {
		fprintf(stderr, "%s: missing range start: %s\n", file_name,
			start_token);
		return 1;
	}

	end = strstr(start, end_token);
	if (!end) {
		fprintf(stderr, "%s: missing range end: %s\n", file_name,
			end_token);
		return 1;
	}

	found = strstr(start, needle);
	if (!found || found >= end)
		return 0;

	fprintf(stderr, "%s: forbidden token present in range %s: %s\n",
		file_name, start_token, needle);
	return 1;
}

static int test_service_context_registries(void)
{
	char *mcu_service;
	char *led_service;
	char *led_internal;
	char *led_makefile;
	char *mcu_chrdev;
	char *led_chrdev;
	int failures = 0;

	mcu_service = read_source_file(
		"kernel/pdm-core/peripheral/mcu/pdm_mcu_service.c");
	led_service = read_source_file(
		"kernel/pdm-core/peripheral/led/pdm_led_service.c");
	led_internal = read_source_file(
		"kernel/pdm-core/peripheral/led/pdm_led_internal.h");
	led_makefile = read_source_file(
		"kernel/pdm-core/peripheral/led/Makefile");
	mcu_chrdev = read_source_file(
		"kernel/pdm-core/peripheral/mcu/pdm_mcu_chrdev.c");
	led_chrdev = read_source_file(
		"kernel/pdm-core/peripheral/led/pdm_led_chrdev.c");

	if (!mcu_service || !led_service || !led_internal || !led_makefile ||
	    !mcu_chrdev || !led_chrdev) {
		fprintf(stderr, "failed to read peripheral service sources\n");
		failures = 1;
		goto out;
	}

	failures += expect_contains(
		"pdm_mcu_service.c", mcu_service,
		"struct pdm_mcu_context *next;");
	failures += expect_contains(
		"pdm_mcu_service.c", mcu_service,
		"static pdm_mcu_context_t *g_mcu_context_list;");
	failures += expect_contains("pdm_mcu_service.c", mcu_service,
				    "pdm_mcu_add_context_locked");
	failures += expect_contains("pdm_mcu_service.c", mcu_service,
				    "pdm_mcu_remove_context_locked");
	failures += expect_not_contains("pdm_mcu_service.c", mcu_service,
					"[PDM_MCU_MAX_DEVICES]");
	failures += expect_not_contains("pdm_mcu_service.c", mcu_service,
					"PDM_MCU_MAX_DEVICES");

	failures += expect_contains(
		"pdm_led_internal.h", led_internal,
		"struct pdm_led_context *next;");
	failures += expect_contains(
		"pdm_led_service.c", led_service,
		"static pdm_led_context_t *g_led_context_list;");
	failures += expect_contains("pdm_led_service.c", led_service,
				    "pdm_led_add_context_locked");
	failures += expect_contains("pdm_led_service.c", led_service,
				    "pdm_led_remove_context_locked");
	failures += expect_not_contains("pdm_led_service.c", led_service,
					"[PDM_LED_MAX_DEVICES]");
	failures += expect_not_contains("pdm_led_service.c", led_service,
					"PDM_LED_MAX_DEVICES");
	failures += expect_not_contains("pdm_led_service.c", led_service,
					"pdm_hw_gpio_");
	failures += expect_not_contains("pdm_led_service.c", led_service,
					"pdm_hw_pwm_");
	failures += expect_contains("led/Makefile", led_makefile,
				    "pdm-core/peripheral/led/pdm_led_gpio.o");
	failures += expect_contains("led/Makefile", led_makefile,
				    "pdm-core/peripheral/led/pdm_led_pwm.o");

	failures += expect_contains(
		"pdm_mcu_chrdev.c", mcu_chrdev,
		"static pdm_chrdev_t g_lpf_mcu_chrdevs[PDM_MCU_MAX_DEVICES];");
	failures += expect_contains(
		"pdm_led_chrdev.c", led_chrdev,
		"static pdm_chrdev_t g_lpf_led_chrdevs[PDM_LED_MAX_DEVICES];");

out:
	free(led_chrdev);
	free(mcu_chrdev);
	free(led_makefile);
	free(led_internal);
	free(led_service);
	free(mcu_service);
	return failures ? 1 : 0;
}

static int test_mcu_transport_is_service_owned(void)
{
	char *mcu_internal;
	char *mcu_makefile;
	int failures = 0;

	mcu_internal = read_source_file(
		"kernel/pdm-core/peripheral/mcu/pdm_mcu_internal.h");
	mcu_makefile = read_source_file(
		"kernel/pdm-core/peripheral/mcu/Makefile");

	if (!mcu_internal || !mcu_makefile) {
		fprintf(stderr, "failed to read MCU transport ownership sources\n");
		failures = 1;
		goto out;
	}

	failures += expect_path_absent("kernel/pdm-runtime/transport/mcu");
	failures += expect_path_absent("kernel/pdm-runtime");
	failures += expect_path_absent("kernel/include/pdm/transport/mcu");
	failures += expect_contains("pdm_mcu_internal.h", mcu_internal,
				    "#include \"pdm_mcu_transport.h\"");
	failures += expect_not_contains("pdm_mcu_internal.h", mcu_internal,
					"pdm/transport/mcu");
	failures += expect_contains("mcu/Makefile", mcu_makefile,
				    "pdm-core/peripheral/mcu/pdm_mcu_transport.o");
	failures += expect_contains(
		"mcu/Makefile", mcu_makefile,
		"pdm-core/peripheral/mcu/pdm_mcu_can.o");
	failures += expect_contains(
		"mcu/Makefile", mcu_makefile,
		"pdm-core/peripheral/mcu/pdm_mcu_uart.o");

out:
	free(mcu_makefile);
	free(mcu_internal);
	return failures ? 1 : 0;
}

static int test_hw_sources_are_capability_grouped(void)
{
	char *hw_makefile;
	char *hw_core;
	char *hw_gpio;
	int failures = 0;

	hw_makefile = read_source_file("kernel/pdm-core/hw/Makefile");
	hw_core = read_source_file("kernel/pdm-core/hw/core/pdm_hw.c");
	hw_gpio = read_source_file("kernel/pdm-core/hw/gpio/pdm_hw_gpio.c");

	if (!hw_makefile || !hw_core || !hw_gpio) {
		fprintf(stderr, "failed to read HW grouping sources\n");
		failures = 1;
		goto out;
	}

	failures += expect_path_present("kernel/pdm-core/include/pdm_hw_internal.h");
	failures += expect_path_absent("kernel/pdm-core/hw/pdm_hw_internal.h");
	failures += expect_path_absent("kernel/pdm-core/hw/pdm_hw.c");
	failures += expect_path_absent("kernel/pdm-core/hw/pdm_hw_gpio.c");
	failures += expect_path_absent("kernel/pdm-core/hw/pdm_hw_pwm.c");
	failures += expect_path_absent("kernel/pdm-core/hw/pdm_hw_bus_i2c.c");
	failures += expect_path_absent("kernel/pdm-core/hw/pdm_hw_bus_spi.c");
	failures += expect_path_absent("kernel/pdm-core/hw/i2c/pdm_hw_bus_i2c.c");
	failures += expect_path_absent("kernel/pdm-core/hw/spi/pdm_hw_bus_spi.c");
	failures += expect_path_absent(
		"kernel/pdm-core/hw/pdm_hw_transport_can.c");
	failures += expect_path_absent(
		"kernel/pdm-core/hw/pdm_hw_transport_uart.c");
	failures += expect_path_absent(
		"kernel/pdm-core/hw/can/pdm_hw_transport_can.c");
	failures += expect_path_absent(
		"kernel/pdm-core/hw/uart/pdm_hw_transport_uart.c");
	failures += expect_path_absent("kernel/pdm-core/hw/bus/i2c");
	failures += expect_path_absent("kernel/pdm-core/hw/bus/spi");
	failures += expect_path_absent("kernel/pdm-core/hw/transport/can");
	failures += expect_path_absent("kernel/pdm-core/hw/transport/uart");

	failures += expect_contains("hw/Makefile", hw_makefile,
				    "pdm-core/hw/core/pdm_hw.o");
	failures += expect_contains("hw/Makefile", hw_makefile,
				    "pdm-core/hw/gpio/pdm_hw_gpio.o");
	failures += expect_contains("hw/Makefile", hw_makefile,
				    "pdm-core/hw/pwm/pdm_hw_pwm.o");
	failures += expect_contains("hw/Makefile", hw_makefile,
				    "pdm-core/hw/i2c/pdm_hw_i2c.o");
	failures += expect_contains("hw/Makefile", hw_makefile,
				    "pdm-core/hw/spi/pdm_hw_spi.o");
	failures += expect_contains(
		"hw/Makefile", hw_makefile,
		"pdm-core/hw/can/pdm_hw_can.o");
	failures += expect_contains(
		"hw/Makefile", hw_makefile,
		"pdm-core/hw/uart/pdm_hw_uart.o");

	failures += expect_contains("pdm_hw.c", hw_core,
				    "#include \"pdm_hw_internal.h\"");
	failures += expect_contains("pdm_hw_gpio.c", hw_gpio,
				    "#include \"pdm_hw_internal.h\"");

out:
	free(hw_gpio);
	free(hw_core);
	free(hw_makefile);
	return failures ? 1 : 0;
}

static int test_static_config_sources_are_version_named(void)
{
	char *config_makefile;
	char *config_header;
	char *config_backend;
	char *config_static_header;
	char *static_backend;
	char *static_table;
	char *x86_config;
	char *mock_config;
	char *x86_defconfig;
	char *mock_defconfig;
	int failures = 0;

	config_makefile = read_source_file("kernel/pdm-configs/Makefile");
	config_header = read_source_file("kernel/include/pdm/config/pdm_config.h");
	config_backend = read_source_file(
		"kernel/pdm-core/config/src/pdm_config_backend.c");
	config_static_header = read_source_file(
		"kernel/pdm-core/config/src/pdm_config_static.h");
	static_backend = read_source_file(
		"kernel/pdm-core/config/src/pdm_config_static_backend.c");
	static_table = read_source_file("kernel/pdm-configs/pdm_config_configs.c");
	x86_config = read_source_file(
		"kernel/pdm-configs/configs/ubuntu/x86_modules/pdm_config_ubuntu_x86_modules_v1.c");
	mock_config = read_source_file(
		"kernel/pdm-configs/configs/ubuntu/x86_mock_modules/pdm_config_ubuntu_x86_mock_modules_v1.c");
	x86_defconfig =
		read_source_file("configs/ubuntu/ubuntu_x86_modules_defconfig");
	mock_defconfig = read_source_file(
		"configs/ubuntu/ubuntu_x86_mock_modules_defconfig");
	if (!config_makefile || !config_header || !config_backend ||
	    !config_static_header || !static_backend || !static_table ||
	    !x86_config || !mock_config || !x86_defconfig || !mock_defconfig) {
		fprintf(stderr, "failed to read static config sources\n");
		failures = 1;
		goto out;
	}

	failures += expect_path_absent(
		"kernel/pdm-configs/configs/kernel/x86_modules/1.0.0");
	failures += expect_path_absent(
		"kernel/pdm-configs/configs/kernel/x86_mock_modules/1.0.0");
	failures += expect_path_absent(
		"kernel/pdm-configs/configs/kernel/x86_modules/pdm_config_kernel_x86_modules_v1.c");
	failures += expect_path_absent(
		"kernel/pdm-configs/configs/kernel/x86_mock_modules/pdm_config_kernel_x86_mock_modules_v1.c");
	failures += expect_path_absent(
		"configs/kernel/kernel_x86_modules_defconfig");
	failures += expect_path_absent(
		"configs/kernel/kernel_x86_mock_modules_defconfig");
	failures += expect_path_absent(
		"kernel/pdm-configs/configs/pdm_config_configs.c");
	failures += expect_path_absent(
		"kernel/pdm-configs/configs/pdm_config_static_start.c");
	failures += expect_path_absent(
		"kernel/pdm-configs/configs/pdm_config_static_end.c");
	failures += expect_path_present("kernel/pdm-configs/pdm_config_configs.c");
	failures += expect_path_present(
		"kernel/pdm-configs/pdm_config_static_start.c");
	failures += expect_path_present(
		"kernel/pdm-configs/pdm_config_static_end.c");
	failures += expect_path_present(
		"kernel/pdm-configs/configs/ubuntu/x86_modules/pdm_config_ubuntu_x86_modules_v1.c");
	failures += expect_path_present(
		"kernel/pdm-configs/configs/ubuntu/x86_mock_modules/pdm_config_ubuntu_x86_mock_modules_v1.c");
	failures += expect_path_present(
		"configs/ubuntu/ubuntu_x86_modules_defconfig");
	failures += expect_path_present(
		"configs/ubuntu/ubuntu_x86_mock_modules_defconfig");
	failures += expect_contains("ubuntu_x86_modules_defconfig",
				    x86_defconfig,
				    "CONFIG_LPF_CONFIG_UBUNTU_X86_MODULES=y");
	failures += expect_contains("ubuntu_x86_mock_modules_defconfig",
				    mock_defconfig,
				    "CONFIG_LPF_CONFIG_UBUNTU_X86_MOCK_MODULES=y");
	failures += expect_not_contains("ubuntu_x86_modules_defconfig",
					x86_defconfig,
					"CONFIG_LPF_CONFIG_KERNEL_X86");
	failures += expect_not_contains("ubuntu_x86_mock_modules_defconfig",
					mock_defconfig,
					"CONFIG_LPF_CONFIG_KERNEL_X86");
	failures += expect_contains(
		"config/Makefile", config_makefile,
		"pdm-configs/configs/ubuntu/x86_modules/pdm_config_ubuntu_x86_modules_v1.o");
	failures += expect_contains(
		"config/Makefile", config_makefile,
		"pdm-configs/configs/ubuntu/x86_mock_modules/pdm_config_ubuntu_x86_mock_modules_v1.o");
	failures += expect_contains(
		"config/Makefile", config_makefile,
		"pdm-configs/pdm_config_configs.o");
	failures += expect_contains(
		"config/Makefile", config_makefile,
		"pdm-configs/pdm_config_static_start.o");
	failures += expect_contains(
		"config/Makefile", config_makefile,
		"pdm-configs/pdm_config_static_end.o");
	failures += expect_contains(
		"config/Makefile", config_makefile,
		"pdm_configs-y += pdm-configs/pdm_config_configs.o");
	failures += expect_contains(
		"config/Makefile", config_makefile,
		"pdm_configs-$(CONFIG_LPF_CONFIG_UBUNTU_X86_MODULES)");
	failures += expect_not_contains("config/Makefile", config_makefile,
					"CONFIG_LPF_CONFIG_KERNEL_X86");
	failures += expect_ordered_contains("pdm_config_backend.c",
					    config_backend,
					    "&g_lpf_config_static_backend",
					    "&g_lpf_config_dt_backend");
	failures += expect_not_contains("config/Makefile", config_makefile,
					"/1.0.0/");
	failures += expect_not_contains("pdm_config.h", config_header,
					"g_lpf_config_platform_table");
	failures += expect_contains("pdm_config_static.h", config_static_header,
				    "pdm_config_static_table_t");
	failures += expect_contains("pdm_config_static.h", config_static_header,
				    "pdm_config_static_register");
	failures += expect_contains("pdm_config_configs.c", static_table,
				    "EXPORT_SYMBOL_GPL(g_lpf_config_platform_table)");
	failures += expect_contains("pdm_config_configs.c", static_table,
				    "pdm_config_static_start");
	failures += expect_contains("pdm_config_configs.c", static_table,
				    "pdm_config_static_end");
	failures += expect_not_contains("pdm_config_configs.c", static_table,
					"CONFIG_LPF_CONFIG_KERNEL_X86");
	failures += expect_not_contains("pdm_config_configs.c", static_table,
					"g_lpf_config_kernel_x86");
	failures += expect_contains("pdm_config_static_backend.c", static_backend,
				    "symbol_get(g_lpf_config_platform_table)");
	failures += expect_contains("pdm_config_static_backend.c", static_backend,
				    "symbol_put(g_lpf_config_platform_table)");
	failures += expect_not_contains("pdm_config_static_backend.c",
					static_backend,
					"pdm_config_static_register_table");
	failures += expect_not_contains("pdm_config_ubuntu_x86_modules_v1.c",
					x86_config,
					"g_lpf_config_platform_table");
	failures += expect_contains("pdm_config_ubuntu_x86_modules_v1.c",
				    x86_config,
				    "pdm_config_static_register");
	failures += expect_contains("pdm_config_ubuntu_x86_modules_v1.c",
				    x86_config, ".product_name = \"ubuntu\"");
	failures += expect_not_contains("pdm_config_ubuntu_x86_mock_modules_v1.c",
					mock_config,
					"g_lpf_config_platform_table");
	failures += expect_contains("pdm_config_ubuntu_x86_mock_modules_v1.c",
				    mock_config,
				    "pdm_config_static_register");
	failures += expect_contains("pdm_config_ubuntu_x86_mock_modules_v1.c",
				    mock_config,
				    "g_lpf_config_ubuntu_x86_mock_modules_nodes");
	failures += expect_contains("pdm_config_ubuntu_x86_mock_modules_v1.c",
				    mock_config,
				    ".device_node_count = OSAL_ARRAY_SIZE");
	failures += expect_contains("pdm_config_ubuntu_x86_mock_modules_v1.c",
				    mock_config,
				    ".device_nodes = g_lpf_config_ubuntu_x86_mock_modules_nodes");
	failures += expect_contains("pdm_config_ubuntu_x86_mock_modules_v1.c",
				    mock_config,
				    ".compatible = \"pdm,mcu\"");
	failures += expect_contains("pdm_config_ubuntu_x86_mock_modules_v1.c",
				    mock_config,
				    ".compatible = \"pdm,led\"");
	failures += expect_contains("pdm_config_ubuntu_x86_mock_modules_v1.c",
				    mock_config,
				    ".mcu_count = 0");
	failures += expect_contains("pdm_config_ubuntu_x86_mock_modules_v1.c",
				    mock_config,
				    ".led_count = 0");
	failures += expect_contains("pdm_config_ubuntu_x86_mock_modules_v1.c",
				    mock_config, ".product_name = \"ubuntu\"");

out:
	free(mock_defconfig);
	free(x86_defconfig);
	free(mock_config);
	free(x86_config);
	free(static_table);
	free(static_backend);
	free(config_static_header);
	free(config_backend);
	free(config_header);
	free(config_makefile);
	return failures ? 1 : 0;
}

static int test_peripheral_layer_dependencies(void)
{
	static const source_file_t files[] = {
		{ "pdm_runtime.c",
		  "kernel/pdm-core/runtime/pdm_runtime.c" },
		{ "pdm_runtime_config.c",
		  "kernel/pdm-core/runtime/pdm_runtime_config.c" },
		{ "pdm_mcu_service.c",
		  "kernel/pdm-core/peripheral/mcu/pdm_mcu_service.c" },
		{ "pdm_mcu_chrdev.c",
		  "kernel/pdm-core/peripheral/mcu/pdm_mcu_chrdev.c" },
		{ "pdm_mcu_proc.c",
		  "kernel/pdm-core/peripheral/mcu/pdm_mcu_proc.c" },
		{ "pdm_led_service.c",
		  "kernel/pdm-core/peripheral/led/pdm_led_service.c" },
		{ "pdm_led_gpio.c",
		  "kernel/pdm-core/peripheral/led/pdm_led_gpio.c" },
		{ "pdm_led_pwm.c",
		  "kernel/pdm-core/peripheral/led/pdm_led_pwm.c" },
		{ "pdm_led_chrdev.c",
		  "kernel/pdm-core/peripheral/led/pdm_led_chrdev.c" },
		{ "pdm_led_proc.c",
		  "kernel/pdm-core/peripheral/led/pdm_led_proc.c" },
		{ "pdm_mcu_transport.c",
		  "kernel/pdm-core/peripheral/mcu/pdm_mcu_transport.c" },
		{ "pdm_mcu_can.c",
		  "kernel/pdm-core/peripheral/mcu/pdm_mcu_can.c" },
		{ "pdm_mcu_uart.c",
		  "kernel/pdm-core/peripheral/mcu/pdm_mcu_uart.c" },
	};
	static const char *forbidden_tokens[] = {
		"pdm/soc/",
		"pdm/compat/",
		"pdm_soc_",
		"pdm_compat_",
	};
	size_t i;
	size_t j;
	int failures = 0;

	for (i = 0; i < ARRAY_SIZE(files); i++) {
		char *content = read_source_file(files[i].path);

		if (!content) {
			fprintf(stderr, "%s: failed to read source\n",
				files[i].name);
			failures++;
			continue;
		}

		for (j = 0; j < ARRAY_SIZE(forbidden_tokens); j++)
			failures += expect_not_contains(
				files[i].name, content, forbidden_tokens[j]);

		free(content);
	}

	return failures ? 1 : 0;
}

static int test_uapi_headers_are_abi_only(void)
{
	static const source_file_t headers[] = {
		{ "pdm_ctl.h", "uapi/pdm/pdm_ctl.h" },
		{ "pdm_mcu.h", "uapi/pdm/pdm_mcu.h" },
		{ "pdm_led.h", "uapi/pdm/pdm_led.h" },
	};
	static const char *forbidden_tokens[] = {
		"pdi_",
		"PDI_",
		"osal_",
		"OSAL_",
		"<stdio.h>",
		"<stdlib.h>",
		"<unistd.h>",
		"<fcntl.h>",
	};
	size_t i;
	size_t j;
	int failures = 0;

	for (i = 0; i < ARRAY_SIZE(headers); i++) {
		char *content = read_source_file(headers[i].path);

		if (!content) {
			fprintf(stderr, "%s: failed to read header\n",
				headers[i].name);
			failures++;
			continue;
		}

		for (j = 0; j < ARRAY_SIZE(forbidden_tokens); j++)
			failures += expect_not_contains(
				headers[i].name, content, forbidden_tokens[j]);

		free(content);
	}

	return failures ? 1 : 0;
}

static int test_soc_adapter_header_dependencies(void)
{
	char *content;
	int failures = 0;

	content = read_source_file("kernel/include/pdm/soc/pdm_soc_adapter.h");
	if (!content) {
		fprintf(stderr, "pdm_soc_adapter.h: failed to read header\n");
		return 1;
	}

	failures += expect_not_contains("pdm_soc_adapter.h", content,
					"pdm/hw/");
	failures += expect_contains("pdm_soc_adapter.h", content,
				    "pdm/types/pdm_gpio_types.h");
	failures += expect_contains("pdm_soc_adapter.h", content,
				    "pdm/types/pdm_pwm_types.h");
	failures += expect_contains("pdm_soc_adapter.h", content,
				    "pdm/types/pdm_i2c_types.h");
	failures += expect_contains("pdm_soc_adapter.h", content,
				    "pdm/types/pdm_spi_types.h");
	failures += expect_contains("pdm_soc_adapter.h", content,
				    "pdm/types/pdm_can_types.h");
	failures += expect_contains("pdm_soc_adapter.h", content,
				    "pdm/types/pdm_serial_types.h");

	free(content);
	return failures ? 1 : 0;
}

static int test_runtime_config_mapping_is_registered(void)
{
	char *runtime_config;
	char *runtime_header;
	char *mcu_driver;
	char *led_driver;
	int failures = 0;

	runtime_config = read_source_file(
		"kernel/pdm-core/runtime/pdm_runtime_config.c");
	runtime_header = read_source_file(
		"kernel/pdm-core/include/pdm_runtime_internal.h");
	mcu_driver = read_source_file(
		"kernel/pdm-core/peripheral/mcu/pdm_mcu_config_driver.c");
	led_driver = read_source_file(
		"kernel/pdm-core/peripheral/led/pdm_led_config_driver.c");

	if (!runtime_config || !runtime_header || !mcu_driver || !led_driver) {
		fprintf(stderr, "failed to read runtime config driver sources\n");
		failures = 1;
		goto out;
	}

	failures += expect_contains("pdm_runtime_config.c", runtime_config,
				    "pdm_runtime_config_driver_first");
	failures += expect_contains("pdm_runtime_config.c", runtime_config,
				    "pdm_config_get_device_nodes");
	failures += expect_contains("pdm_runtime_config.c", runtime_config,
				    "driver->compatible");
	failures += expect_contains("pdm_runtime_config.c", runtime_config,
				    "node->compatible");
	failures += expect_contains("pdm_runtime_internal.h", runtime_header,
				    "pdm_runtime_config_compatible_driver_register");
	failures += expect_not_contains("pdm_runtime_config.c", runtime_config,
					"PDM_CONFIG_DEVICE_TYPE_MCU");
	failures += expect_not_contains("pdm_runtime_config.c", runtime_config,
					"PDM_CONFIG_DEVICE_TYPE_LED");
	failures += expect_not_contains("pdm_runtime_config.c", runtime_config,
					"pdm_config_get()");
	failures += expect_not_contains("pdm_runtime_config.c", runtime_config,
					"mcu_count");
	failures += expect_not_contains("pdm_runtime_config.c", runtime_config,
					"led_count");
	failures += expect_contains("pdm_mcu_config_driver.c", mcu_driver,
				    "pdm_runtime_config_driver_register");
	failures += expect_contains("pdm_led_config_driver.c", led_driver,
				    "pdm_runtime_config_driver_register");
	failures += expect_contains("pdm_mcu_config_driver.c", mcu_driver,
				    "pdm_device_register");
	failures += expect_contains("pdm_led_config_driver.c", led_driver,
				    "pdm_device_register");
	failures += expect_contains("pdm_mcu_config_driver.c", mcu_driver,
				    "pdm_config_device_node_t");
	failures += expect_contains("pdm_led_config_driver.c", led_driver,
				    "pdm_config_device_node_t");
	failures += expect_not_contains("pdm_mcu_config_driver.c", mcu_driver,
					"pdm_config_hw_get_mcu");
	failures += expect_not_contains("pdm_led_config_driver.c", led_driver,
					"pdm_config_hw_get_led");
	failures += expect_not_contains("pdm_mcu_config_driver.c", mcu_driver,
					"mcu_count");
	failures += expect_not_contains("pdm_led_config_driver.c", led_driver,
					"led_count");

out:
	free(led_driver);
	free(mcu_driver);
	free(runtime_header);
	free(runtime_config);
	return failures ? 1 : 0;
}

static int test_runtime_entries_are_classed(void)
{
	char *runtime_header;
	char *mcu_service;
	char *led_service;
	char *config_selftest;
	int failures = 0;

	runtime_header = read_source_file(
		"kernel/pdm-core/include/pdm_runtime_internal.h");
	mcu_service = read_source_file(
		"kernel/pdm-core/peripheral/mcu/pdm_mcu_service.c");
	led_service = read_source_file(
		"kernel/pdm-core/peripheral/led/pdm_led_service.c");
	config_selftest = read_source_file(
		"kernel/pdm-core/config/selftest/pdm_config_of_selftest.c");

	if (!runtime_header || !mcu_service || !led_service || !config_selftest) {
		fprintf(stderr, "failed to read runtime entry sources\n");
		failures = 1;
		goto out;
	}

	failures += expect_contains("pdm_runtime_internal.h", runtime_header,
				    "pdm_runtime_service_register");
	failures += expect_contains("pdm_runtime_internal.h", runtime_header,
				    "pdm_runtime_selftest_register");
	failures += expect_not_contains("pdm_runtime_internal.h", runtime_header,
					"#define pdm_runtime_register");
	failures += expect_contains("pdm_mcu_service.c", mcu_service,
				    "pdm_runtime_service_register");
	failures += expect_contains("pdm_led_service.c", led_service,
				    "pdm_runtime_service_register");
	failures += expect_contains("pdm_config_of_selftest.c", config_selftest,
				    "pdm_runtime_selftest_register");

out:
	free(config_selftest);
	free(led_service);
	free(mcu_service);
	free(runtime_header);
	return failures ? 1 : 0;
}

static int test_core_lifecycle_is_module_owned(void)
{
	char *core_header;
	char *driver_header;
	char *device_header;
	char *core_source;
	char *runtime_source;
	int failures = 0;

	core_header = read_source_file("kernel/include/pdm/core/pdm_core.h");
	driver_header = read_source_file("kernel/include/pdm/core/pdm_driver.h");
	device_header = read_source_file("kernel/include/pdm/core/pdm_device.h");
	core_source = read_source_file("kernel/pdm-core/core/pdm_core.c");
	runtime_source = read_source_file(
		"kernel/pdm-core/runtime/pdm_runtime.c");

	if (!core_header || !driver_header || !device_header || !core_source ||
	    !runtime_source) {
		fprintf(stderr, "failed to read core lifecycle sources\n");
		failures = 1;
		goto out;
	}

	failures += expect_not_contains("pdm_core.h", core_header,
					"pdm_core_init");
	failures += expect_not_contains("pdm_core.h", core_header,
					"pdm_core_deinit");
	failures += expect_contains("pdm_core.c", core_source,
				    "static int32_t pdm_core_init(void)");
	failures += expect_contains("pdm_core.c", core_source,
				    "static void pdm_core_deinit(void)");
	failures += expect_path_absent(
		"kernel/pdm-core/runtime/pdm_runtime_module.c");
	failures += expect_contains("pdm_core.c", core_source,
				    "pdm_runtime_init()");
	failures += expect_contains("pdm_core.c", core_source,
				    "pdm_runtime_exit()");
	failures += expect_not_contains("pdm_core.c", core_source,
					"EXPORT_SYMBOL_GPL(pdm_core_init)");
	failures += expect_not_contains("pdm_core.c", core_source,
					"EXPORT_SYMBOL_GPL(pdm_core_deinit)");
	failures += expect_range_not_contains(
		"pdm_core.c", core_source,
		"int32_t pdm_driver_register(const pdm_driver_t *driver)",
		"EXPORT_SYMBOL_GPL(pdm_driver_register);",
		"pdm_core_init");
	failures += expect_range_not_contains(
		"pdm_core.c", core_source,
		"int32_t pdm_device_register(const pdm_device_config_t *config)",
		"EXPORT_SYMBOL_GPL(pdm_device_register);",
		"pdm_core_init");
	failures += expect_range_not_contains(
		"pdm_core.c", core_source,
		"int32_t pdm_device_event_subscribe",
		"EXPORT_SYMBOL_GPL(pdm_device_event_subscribe);",
		"pdm_core_init");
	failures += expect_not_contains("pdm_runtime.c", runtime_source,
					"pdm_core_init");
	failures += expect_contains("pdm_core.h", core_header,
				    "LPF-owned device model");
	failures += expect_contains("pdm_core.h", core_header,
				    "matches devices to drivers by LPF device type");
	failures += expect_contains("pdm_driver.h", driver_header,
				    "Match key used by LPF Core");
	failures += expect_contains("pdm_device.h", device_header,
				    "Match key used to find the registered LPF driver");
	failures += expect_contains("pdm_core.c", core_source,
				    "pdm_match_driver_by_type_locked");

out:
	free(runtime_source);
	free(core_source);
	free(device_header);
	free(driver_header);
	free(core_header);
	return failures ? 1 : 0;
}

static int test_instance_devnode_mode_policy(void)
{
	char *core_config;
	char *chrdev_source;
	int failures = 0;

	core_config = read_source_file("kernel/pdm-core/Config.in");
	chrdev_source = read_source_file("kernel/pdm-core/core/pdm_chrdev.c");

	if (!core_config || !chrdev_source) {
		fprintf(stderr, "failed to read instance devnode policy sources\n");
		failures = 1;
		goto out;
	}

	failures += expect_contains("Config.in", core_config,
				    "config PDM_INSTANCE_DEVNODE_MODE");
	failures += expect_contains("Config.in", core_config,
				    "default \"0660\"");
	failures += expect_contains("pdm_chrdev.c", chrdev_source,
				    "CONFIG_LPF_INSTANCE_DEVNODE_MODE");
	failures += expect_contains("pdm_chrdev.c", chrdev_source,
				    "PDM_INSTANCE_DEVNODE_MODE_FALLBACK 0660");
	failures += expect_contains("pdm_chrdev.c", chrdev_source,
				    "pdm_chrdev_parse_mode");
	failures += expect_contains("pdm_chrdev.c", chrdev_source,
				    "pdm_chrdev_instance_mode()");
	failures += expect_contains("pdm_chrdev.c", chrdev_source,
				    "PDM_CTL_DEVNODE_MODE");
	failures += expect_not_contains("pdm_chrdev.c", chrdev_source,
					"chrdev->miscdev.mode = 0666;");

out:
	free(chrdev_source);
	free(core_config);
	return failures ? 1 : 0;
}

static int test_no_separate_bus_layer(void)
{
	char *plan;
	char *peripheral_readme;
	int failures = 0;

	plan = read_source_file("docs/pdm_architecture_refactor_plan.md");
	peripheral_readme = read_source_file(
		"kernel/pdm-core/peripheral/README.md");
	if (!plan || !peripheral_readme) {
		fprintf(stderr, "failed to read bus-layer policy docs\n");
		failures = 1;
		goto out;
	}

	failures += expect_path_absent("kernel/pdm-core/bus");
	failures += expect_path_absent("kernel/pdm-runtime/bus");
	failures += expect_contains("pdm_architecture_refactor_plan.md", plan,
				    "do not create extra");
	failures += expect_contains("peripheral/README.md", peripheral_readme,
				    "does not expose a separate physical bus layer");

out:
	free(peripheral_readme);
	free(plan);
	return failures ? 1 : 0;
}

static int test_mock_smoke_covers_runtime_surfaces(void)
{
	char *script;
	char *makefile;
	char *tests_cmake;
	char *smoke_cmake;
	char *smoke_source;
	int failures = 0;

	script = read_source_file("scripts/pdm_mock_module_smoke.sh");
	makefile = read_source_file("Makefile");
	tests_cmake = read_source_file("tests/user/CMakeLists.txt");
	smoke_cmake = read_source_file("tests/user/smoke/CMakeLists.txt");
	smoke_source = read_source_file(
		"tests/user/smoke/test_lpf_mock_runtime_smoke.c");
	if (!script || !makefile || !tests_cmake || !smoke_cmake ||
	    !smoke_source) {
		fprintf(stderr, "failed to read mock smoke sources\n");
		failures = 1;
		goto out;
	}

	failures += expect_contains("pdm_mock_module_smoke.sh", script,
				    "pdm_mock_runtime_smoke");
	failures += expect_contains("pdm_mock_module_smoke.sh", script,
				    "/dev/pdm_ctl");
	failures += expect_contains("pdm_mock_module_smoke.sh", script,
				    "/dev/pdm/mcu0");
	failures += expect_contains("pdm_mock_module_smoke.sh", script,
				    "/dev/pdm/led1");
	failures += expect_contains("pdm_mock_module_smoke.sh", script,
				    "PDM_EXPECT_INSTANCE_DEVNODE_MODE");
	failures += expect_contains("pdm_mock_module_smoke.sh", script,
				    "check_devnode_modes");
	failures += expect_contains("pdm_mock_module_smoke.sh", script,
				    "expect_not_world_writable");
	failures += expect_contains("pdm_mock_module_smoke.sh", script,
				    "expect_no_write_bits");
	failures += expect_contains("pdm_mock_module_smoke.sh", script,
				    "expect_has_write_bits");
	failures += expect_contains("Makefile", makefile,
				    "PDM_EXPECT_INSTANCE_DEVNODE_MODE=$(CONFIG_LPF_INSTANCE_DEVNODE_MODE)");
	failures += expect_contains("pdm_mock_module_smoke.sh", script,
				    "/proc/pdm/mcu");
	failures += expect_contains("pdm_mock_module_smoke.sh", script,
				    "/sys/kernel/debug/pdm/led");
	failures += expect_contains("tests/user/CMakeLists.txt", tests_cmake,
				    "add_subdirectory(smoke)");
	failures += expect_contains("smoke/CMakeLists.txt", smoke_cmake,
				    "add_executable(pdm_mock_runtime_smoke");
	failures += expect_not_contains("smoke/CMakeLists.txt", smoke_cmake,
					"add_test(");
	failures += expect_contains("test_lpf_mock_runtime_smoke.c",
				    smoke_source,
				    "pdi_get_device_by_name");
	failures += expect_contains("test_lpf_mock_runtime_smoke.c",
				    smoke_source,
				    "pdi_mcu_get_info");
	failures += expect_contains("test_lpf_mock_runtime_smoke.c",
				    smoke_source,
				    "pdi_led_get_info");

out:
	free(smoke_source);
	free(smoke_cmake);
	free(tests_cmake);
	free(makefile);
	free(script);
	return failures ? 1 : 0;
}

static int test_pdi_coverage_tracks_discovery_paths(void)
{
	char *pdi_mock;
	char *plan;
	int failures = 0;

	pdi_mock = read_source_file("tests/user/pdi/test_pdi_ioctl_mock.c");
	plan = read_source_file("docs/pdm_architecture_refactor_plan.md");
	if (!pdi_mock || !plan) {
		fprintf(stderr, "failed to read PDI coverage sources\n");
		failures = 1;
		goto out;
	}

	failures += expect_contains("test_pdi_ioctl_mock.c", pdi_mock,
				    "test_control_discovery");
	failures += expect_contains("test_pdi_ioctl_mock.c", pdi_mock,
				    "PDM_CTL_IOC_GET_DEVICE_BY_CAPABILITY");
	failures += expect_contains("test_pdi_ioctl_mock.c", pdi_mock,
				    "pdi_get_device_by_capability");
	failures += expect_contains("test_pdi_ioctl_mock.c", pdi_mock,
				    "\"missing\"");
	failures += expect_contains("test_pdi_ioctl_mock.c", pdi_mock,
				    "\"wrong-type\"");
	failures += expect_contains("test_pdi_ioctl_mock.c", pdi_mock,
				    "pdi_mcu_get_info");
	failures += expect_contains("test_pdi_ioctl_mock.c", pdi_mock,
				    "pdi_led_get_info");
	failures += expect_contains("pdm_architecture_refactor_plan.md",
				    plan, "Add ABI/PDI coverage for current");

out:
	free(plan);
	free(pdi_mock);
	return failures ? 1 : 0;
}

int main(void)
{
	int ret = 0;

	ret += test_service_context_registries();
	ret += test_mcu_transport_is_service_owned();
	ret += test_hw_sources_are_capability_grouped();
	ret += test_static_config_sources_are_version_named();
	ret += test_peripheral_layer_dependencies();
	ret += test_uapi_headers_are_abi_only();
	ret += test_soc_adapter_header_dependencies();
	ret += test_runtime_config_mapping_is_registered();
	ret += test_runtime_entries_are_classed();
	ret += test_core_lifecycle_is_module_owned();
	ret += test_instance_devnode_mode_policy();
	ret += test_no_separate_bus_layer();
	ret += test_mock_smoke_covers_runtime_surfaces();
	ret += test_pdi_coverage_tracks_discovery_paths();

	return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}
