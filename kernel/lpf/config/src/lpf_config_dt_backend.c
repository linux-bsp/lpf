// SPDX-License-Identifier: GPL-2.0

#include "osal.h"
#include "lpf_config_backend.h"

#include <linux/of.h>

#define LPF_CONFIG_DT_ROOT_PATH "/lpf"
#define LPF_CONFIG_DT_COMPATIBLE "lpf,linux-peripheral-framework"
#define LPF_CONFIG_DT_LEGACY_COMPATIBLE "linux-peripheral-framework"
#define LPF_CONFIG_DT_OLD_COMPATIBLE "lpf,platform-config"

#define LPF_CONFIG_DT_GROUP_MCU "mcu"
#define LPF_CONFIG_DT_GROUP_LED "led"

#define LPF_CONFIG_DT_PROP_PLATFORM_NAME "platform-name"
#define LPF_CONFIG_DT_PROP_CHIP_NAME "chip-name"
#define LPF_CONFIG_DT_PROP_PROJECT_NAME "project-name"
#define LPF_CONFIG_DT_PROP_PRODUCT_NAME "product-name"
#define LPF_CONFIG_DT_PROP_CONFIG_VERSION "config-version"

#define LPF_CONFIG_DT_PROP_LABEL "label"
#define LPF_CONFIG_DT_PROP_INTERFACE "interface"
#define LPF_CONFIG_DT_PROP_DEVICE "device"
#define LPF_CONFIG_DT_PROP_BITRATE "bitrate"
#define LPF_CONFIG_DT_PROP_RX_TIMEOUT_MS "rx-timeout-ms"
#define LPF_CONFIG_DT_PROP_TX_TIMEOUT_MS "tx-timeout-ms"
#define LPF_CONFIG_DT_PROP_TX_ID "tx-id"
#define LPF_CONFIG_DT_PROP_RX_ID "rx-id"
#define LPF_CONFIG_DT_PROP_BAUDRATE "baudrate"
#define LPF_CONFIG_DT_PROP_DATA_BITS "data-bits"
#define LPF_CONFIG_DT_PROP_STOP_BITS "stop-bits"
#define LPF_CONFIG_DT_PROP_PARITY "parity"
#define LPF_CONFIG_DT_PROP_FLOW_CONTROL "flow-control"
#define LPF_CONFIG_DT_PROP_CMD_TIMEOUT_MS "cmd-timeout-ms"
#define LPF_CONFIG_DT_PROP_RETRY_COUNT "retry-count"

#define LPF_CONFIG_DT_PROP_CONTROL "control"
#define LPF_CONFIG_DT_PROP_MAX_BRIGHTNESS "max-brightness"
#define LPF_CONFIG_DT_PROP_DEFAULT_BRIGHTNESS "default-brightness"
#define LPF_CONFIG_DT_PROP_GPIO "gpio"
#define LPF_CONFIG_DT_PROP_PIN_MUX "pin-mux"
#define LPF_CONFIG_DT_PROP_ACTIVE_LOW "active-low"
#define LPF_CONFIG_DT_PROP_PULL_UP "pull-up"
#define LPF_CONFIG_DT_PROP_PULL_DOWN "pull-down"
#define LPF_CONFIG_DT_PROP_CONSUMER "consumer"
#define LPF_CONFIG_DT_PROP_PERIOD_NS "period-ns"
#define LPF_CONFIG_DT_PROP_POLARITY_INVERSED "polarity-inversed"

typedef struct {
	lpf_config_platform_config_t platform;
	lpf_config_mcu_entry_t *mcu_entries;
	lpf_config_led_entry_t *led_entries;
} lpf_config_dt_context_t;

static lpf_config_dt_context_t g_lpf_config_dt;
static bool g_lpf_config_dt_loaded;

static const char *lpf_config_dt_string_or_default(const char *value,
						const char *fallback)
{
	return value ? value : fallback;
}

static int32_t lpf_config_dt_read_string(const struct device_node *node,
				      const char *property, const char **out,
				      const char *fallback)
{
	const char *value = NULL;
	int ret;

	if (!out)
		return OSAL_ERR_INVALID_PARAM;

	ret = of_property_read_string(node, property, &value);
	if (ret && !fallback)
		return OSAL_ERR_INVALID_PARAM;

	*out = lpf_config_dt_string_or_default(value, fallback);
	return OSAL_SUCCESS;
}

static uint32_t lpf_config_dt_read_u32_default(const struct device_node *node,
					    const char *property,
					    uint32_t fallback)
{
	uint32_t value;

	if (0 == of_property_read_u32(node, property, &value))
		return value;

	return fallback;
}

static bool lpf_config_dt_read_bool_default(const struct device_node *node,
					 const char *property, bool fallback)
{
	if (of_property_read_bool(node, property))
		return true;

	return fallback;
}

static struct device_node *lpf_config_dt_find_root(void)
{
	struct device_node *root;

	root = of_find_node_by_path(LPF_CONFIG_DT_ROOT_PATH);
	if (root)
		return root;

	root = of_find_compatible_node(NULL, NULL, LPF_CONFIG_DT_COMPATIBLE);
	if (root)
		return root;

	root = of_find_compatible_node(NULL, NULL, LPF_CONFIG_DT_LEGACY_COMPATIBLE);
	if (root)
		return root;

	return of_find_compatible_node(NULL, NULL, LPF_CONFIG_DT_OLD_COMPATIBLE);
}

static struct device_node *lpf_config_dt_find_child(const struct device_node *root,
						 const char *name)
{
	struct device_node *child;

	for_each_available_child_of_node(root, child) {
		if (0 == osal_strcmp(child->name, name))
			return child;
	}

	return NULL;
}

static lpf_config_mcu_interface_t lpf_config_dt_parse_mcu_interface(const char *value)
{
	if (value && 0 == osal_strcmp(value, "serial"))
		return LPF_CONFIG_MCU_INTERFACE_SERIAL;

	if (value && 0 == osal_strcmp(value, "i2c"))
		return LPF_CONFIG_MCU_INTERFACE_I2C;

	if (value && 0 == osal_strcmp(value, "spi"))
		return LPF_CONFIG_MCU_INTERFACE_SPI;

	return LPF_CONFIG_MCU_INTERFACE_CAN;
}

static lpf_config_mcu_parity_t lpf_config_dt_parse_parity(const char *value)
{
	if (value && 0 == osal_strcmp(value, "odd"))
		return LPF_CONFIG_MCU_PARITY_ODD;

	if (value && 0 == osal_strcmp(value, "even"))
		return LPF_CONFIG_MCU_PARITY_EVEN;

	return LPF_CONFIG_MCU_PARITY_NONE;
}

static lpf_config_mcu_flow_control_t lpf_config_dt_parse_flow_control(
	const char *value)
{
	if (value && 0 == osal_strcmp(value, "hw"))
		return LPF_CONFIG_MCU_FLOW_HW;

	if (value && 0 == osal_strcmp(value, "sw"))
		return LPF_CONFIG_MCU_FLOW_SW;

	return LPF_CONFIG_MCU_FLOW_NONE;
}

static lpf_config_led_control_t lpf_config_dt_parse_led_control(const char *value)
{
	if (value && 0 == osal_strcmp(value, "pwm"))
		return LPF_CONFIG_LED_CONTROL_PWM;

	return LPF_CONFIG_LED_CONTROL_GPIO;
}

static int32_t lpf_config_dt_count_available_children(
	const struct device_node *parent)
{
	struct device_node *child;
	uint32_t count = 0;

	if (!parent)
		return 0;

	for_each_available_child_of_node(parent, child)
		count++;

	return count;
}

static void lpf_config_dt_free_entries(void)
{
	osal_free(g_lpf_config_dt.mcu_entries);
	osal_free(g_lpf_config_dt.led_entries);
	g_lpf_config_dt.mcu_entries = NULL;
	g_lpf_config_dt.led_entries = NULL;
}

static int32_t lpf_config_dt_parse_mcu_can(struct device_node *node,
					lpf_config_mcu_config_t *config)
{
	const char *device;
	int32_t ret;

	ret = lpf_config_dt_read_string(node, LPF_CONFIG_DT_PROP_DEVICE,
					&device, NULL);
	if (ret != OSAL_SUCCESS)
		return ret;

	config->hw.can.device = device;
	config->hw.can.bitrate =
		lpf_config_dt_read_u32_default(
			node, LPF_CONFIG_DT_PROP_BITRATE, 500000U);
	config->hw.can.rx_timeout =
		lpf_config_dt_read_u32_default(
			node, LPF_CONFIG_DT_PROP_RX_TIMEOUT_MS, 1000U);
	config->hw.can.tx_timeout =
		lpf_config_dt_read_u32_default(
			node, LPF_CONFIG_DT_PROP_TX_TIMEOUT_MS, 1000U);
	config->hw.can.tx_id =
		lpf_config_dt_read_u32_default(
			node, LPF_CONFIG_DT_PROP_TX_ID, 0U);
	config->hw.can.rx_id =
		lpf_config_dt_read_u32_default(
			node, LPF_CONFIG_DT_PROP_RX_ID, 0U);
	return OSAL_SUCCESS;
}

static int32_t lpf_config_dt_parse_mcu_serial(struct device_node *node,
					   lpf_config_mcu_config_t *config)
{
	const char *device;
	const char *parity = NULL;
	const char *flow_control = NULL;
	int32_t ret;

	ret = lpf_config_dt_read_string(node, LPF_CONFIG_DT_PROP_DEVICE,
					&device, NULL);
	if (ret != OSAL_SUCCESS)
		return ret;

	(void)of_property_read_string(node, LPF_CONFIG_DT_PROP_PARITY, &parity);
	(void)of_property_read_string(node, LPF_CONFIG_DT_PROP_FLOW_CONTROL,
				      &flow_control);

	config->hw.serial.device = device;
	config->hw.serial.baudrate =
		lpf_config_dt_read_u32_default(
			node, LPF_CONFIG_DT_PROP_BAUDRATE, 115200U);
	config->hw.serial.data_bits =
		(uint8_t)lpf_config_dt_read_u32_default(
			node, LPF_CONFIG_DT_PROP_DATA_BITS, 8U);
	config->hw.serial.stop_bits =
		(uint8_t)lpf_config_dt_read_u32_default(
			node, LPF_CONFIG_DT_PROP_STOP_BITS, 1U);
	config->hw.serial.parity = lpf_config_dt_parse_parity(parity);
	config->hw.serial.flow_control =
		lpf_config_dt_parse_flow_control(flow_control);
	return OSAL_SUCCESS;
}

static int32_t lpf_config_dt_parse_mcu_entry(struct device_node *node,
					  lpf_config_mcu_entry_t *entry)
{
	const char *name;
	const char *interface = NULL;
	int32_t ret;

	ret = lpf_config_dt_read_string(node, LPF_CONFIG_DT_PROP_LABEL, &name,
					node->name);
	if (ret != OSAL_SUCCESS)
		return ret;

	(void)of_property_read_string(node, LPF_CONFIG_DT_PROP_INTERFACE,
				      &interface);

	entry->description = lpf_config_dt_string_or_default(name, "mcu");
	entry->enabled = true;
	osal_strncpy(entry->config.name, name,
		     sizeof(entry->config.name) - 1U);
	entry->config.name[sizeof(entry->config.name) - 1U] = '\0';
	entry->config.interface = lpf_config_dt_parse_mcu_interface(interface);
	entry->config.cmd_timeout_ms =
		lpf_config_dt_read_u32_default(
			node, LPF_CONFIG_DT_PROP_CMD_TIMEOUT_MS, 1000U);
	entry->config.retry_count =
		lpf_config_dt_read_u32_default(
			node, LPF_CONFIG_DT_PROP_RETRY_COUNT, 3U);

	switch (entry->config.interface) {
	case LPF_CONFIG_MCU_INTERFACE_CAN:
		return lpf_config_dt_parse_mcu_can(node, &entry->config);
	case LPF_CONFIG_MCU_INTERFACE_SERIAL:
		return lpf_config_dt_parse_mcu_serial(node, &entry->config);
	default:
		return OSAL_ERR_NOT_SUPPORTED;
	}
}

static int32_t lpf_config_dt_parse_mcu_group(struct device_node *root)
{
	struct device_node *group;
	struct device_node *node;
	uint32_t count;
	uint32_t index = 0;
	int32_t ret = OSAL_SUCCESS;

	group = lpf_config_dt_find_child(root, LPF_CONFIG_DT_GROUP_MCU);
	if (!group)
		return OSAL_SUCCESS;

	count = lpf_config_dt_count_available_children(group);
	if (0 == count)
		goto out_put_group;

	g_lpf_config_dt.mcu_entries =
		osal_zalloc(sizeof(*g_lpf_config_dt.mcu_entries) * count);
	if (!g_lpf_config_dt.mcu_entries) {
		ret = OSAL_ERR_NO_MEMORY;
		goto out_put_group;
	}

	for_each_available_child_of_node(group, node) {
		ret = lpf_config_dt_parse_mcu_entry(
			node, &g_lpf_config_dt.mcu_entries[index]);
		if (ret != OSAL_SUCCESS) {
			of_node_put(node);
			goto out_put_group;
		}
		index++;
	}

	g_lpf_config_dt.platform.mcu_count = count;
	g_lpf_config_dt.platform.mcu_array = g_lpf_config_dt.mcu_entries;
	ret = OSAL_SUCCESS;

out_put_group:
	of_node_put(group);
	return ret;
}

static int32_t lpf_config_dt_parse_led_entry(struct device_node *node,
					  lpf_config_led_entry_t *entry)
{
	const char *name;
	const char *control = NULL;
	int32_t ret;

	ret = lpf_config_dt_read_string(node, LPF_CONFIG_DT_PROP_LABEL, &name,
					node->name);
	if (ret != OSAL_SUCCESS)
		return ret;

	(void)of_property_read_string(node, LPF_CONFIG_DT_PROP_CONTROL,
				      &control);

	entry->description = lpf_config_dt_string_or_default(name, "led");
	entry->enabled = true;
	osal_strncpy(entry->config.name, name,
		     sizeof(entry->config.name) - 1U);
	entry->config.name[sizeof(entry->config.name) - 1U] = '\0';
	entry->config.control = lpf_config_dt_parse_led_control(control);
	entry->config.max_brightness =
		lpf_config_dt_read_u32_default(
			node, LPF_CONFIG_DT_PROP_MAX_BRIGHTNESS, 255U);
	entry->config.default_brightness =
		lpf_config_dt_read_u32_default(
			node, LPF_CONFIG_DT_PROP_DEFAULT_BRIGHTNESS, 0U);

	switch (entry->config.control) {
	case LPF_CONFIG_LED_CONTROL_GPIO:
		entry->config.hw.gpio.gpio_num =
			lpf_config_dt_read_u32_default(
				node, LPF_CONFIG_DT_PROP_GPIO, 0U);
		entry->config.hw.gpio.pin_mux =
			lpf_config_dt_read_u32_default(
				node, LPF_CONFIG_DT_PROP_PIN_MUX, 0U);
		entry->config.hw.gpio.active_low =
			lpf_config_dt_read_bool_default(
				node, LPF_CONFIG_DT_PROP_ACTIVE_LOW, false);
		entry->config.hw.gpio.pull_up =
			lpf_config_dt_read_bool_default(
				node, LPF_CONFIG_DT_PROP_PULL_UP, false);
		entry->config.hw.gpio.pull_down =
			lpf_config_dt_read_bool_default(
				node, LPF_CONFIG_DT_PROP_PULL_DOWN, false);
		break;
	case LPF_CONFIG_LED_CONTROL_PWM:
		ret = lpf_config_dt_read_string(node, LPF_CONFIG_DT_PROP_CONSUMER,
					     &entry->config.hw.pwm.consumer,
					     NULL);
		if (ret != OSAL_SUCCESS)
			return ret;
		entry->config.hw.pwm.period_ns =
			lpf_config_dt_read_u32_default(
				node, LPF_CONFIG_DT_PROP_PERIOD_NS, 0U);
		entry->config.hw.pwm.polarity_inversed =
			lpf_config_dt_read_bool_default(
				node, LPF_CONFIG_DT_PROP_POLARITY_INVERSED,
				false);
		break;
	default:
		return OSAL_ERR_NOT_SUPPORTED;
	}

	return OSAL_SUCCESS;
}

static int32_t lpf_config_dt_parse_led_group(struct device_node *root)
{
	struct device_node *group;
	struct device_node *node;
	uint32_t count;
	uint32_t index = 0;
	int32_t ret = OSAL_SUCCESS;

	group = lpf_config_dt_find_child(root, LPF_CONFIG_DT_GROUP_LED);
	if (!group)
		return OSAL_SUCCESS;

	count = lpf_config_dt_count_available_children(group);
	if (0 == count)
		goto out_put_group;

	g_lpf_config_dt.led_entries =
		osal_zalloc(sizeof(*g_lpf_config_dt.led_entries) * count);
	if (!g_lpf_config_dt.led_entries) {
		ret = OSAL_ERR_NO_MEMORY;
		goto out_put_group;
	}

	for_each_available_child_of_node(group, node) {
		ret = lpf_config_dt_parse_led_entry(
			node, &g_lpf_config_dt.led_entries[index]);
		if (ret != OSAL_SUCCESS) {
			of_node_put(node);
			goto out_put_group;
		}
		index++;
	}

	g_lpf_config_dt.platform.led_count = count;
	g_lpf_config_dt.platform.led_array = g_lpf_config_dt.led_entries;
	ret = OSAL_SUCCESS;

out_put_group:
	of_node_put(group);
	return ret;
}

static bool lpf_config_dt_available(void)
{
	struct device_node *root;

	root = lpf_config_dt_find_root();
	if (!root)
		return false;

	of_node_put(root);
	return true;
}

static int32_t lpf_config_dt_load(void)
{
	struct device_node *root;
	int32_t ret;

	if (g_lpf_config_dt_loaded)
		return OSAL_SUCCESS;

	root = lpf_config_dt_find_root();
	if (!root)
		return OSAL_ERR_NOT_SUPPORTED;

	osal_memset(&g_lpf_config_dt, 0, sizeof(g_lpf_config_dt));

	ret = lpf_config_dt_read_string(root, LPF_CONFIG_DT_PROP_PLATFORM_NAME,
				     &g_lpf_config_dt.platform.platform_name,
				     "linux");
	if (ret != OSAL_SUCCESS)
		goto out_error;

	ret = lpf_config_dt_read_string(root, LPF_CONFIG_DT_PROP_CHIP_NAME,
				     &g_lpf_config_dt.platform.chip_name,
				     "unknown");
	if (ret != OSAL_SUCCESS)
		goto out_error;

	ret = lpf_config_dt_read_string(root, LPF_CONFIG_DT_PROP_PROJECT_NAME,
				     &g_lpf_config_dt.platform.project_name,
				     "default");
	if (ret != OSAL_SUCCESS)
		goto out_error;

	ret = lpf_config_dt_read_string(root, LPF_CONFIG_DT_PROP_PRODUCT_NAME,
				     &g_lpf_config_dt.platform.product_name,
				     "lpf");
	if (ret != OSAL_SUCCESS)
		goto out_error;

	ret = lpf_config_dt_read_string(root, LPF_CONFIG_DT_PROP_CONFIG_VERSION,
				     &g_lpf_config_dt.platform.version,
				     "1.0.0");
	if (ret != OSAL_SUCCESS)
		goto out_error;

	ret = lpf_config_dt_parse_mcu_group(root);
	if (ret != OSAL_SUCCESS)
		goto out_error;

	ret = lpf_config_dt_parse_led_group(root);
	if (ret != OSAL_SUCCESS)
		goto out_error;

	g_lpf_config_dt_loaded = true;
	of_node_put(root);
	return OSAL_SUCCESS;

out_error:
	lpf_config_dt_free_entries();
	osal_memset(&g_lpf_config_dt, 0, sizeof(g_lpf_config_dt));
	of_node_put(root);
	return ret;
}

static void lpf_config_dt_unload(void)
{
	lpf_config_dt_free_entries();
	osal_memset(&g_lpf_config_dt, 0, sizeof(g_lpf_config_dt));
	g_lpf_config_dt_loaded = false;
}

static const lpf_config_platform_config_t *lpf_config_dt_active(void)
{
	return g_lpf_config_dt_loaded ? &g_lpf_config_dt.platform : NULL;
}

static const lpf_config_platform_config_t *
lpf_config_dt_find(const char *product, const char *project, const char *version)
{
	const lpf_config_platform_config_t *config;

	config = lpf_config_dt_active();
	if (!config || !product || !project)
		return NULL;

	if (0 != osal_strcmp(config->product_name, product))
		return NULL;

	if (0 != osal_strcmp(config->project_name, project))
		return NULL;

	if (version && 0 != osal_strcmp(config->version, version))
		return NULL;

	return config;
}

static int32_t lpf_config_dt_list(const lpf_config_platform_config_t **configs,
			       uint32_t *count)
{
	if (!configs || !count)
		return OSAL_ERR_GENERIC;

	if (0 == *count) {
		*count = 0;
		return OSAL_SUCCESS;
	}

	if (!g_lpf_config_dt_loaded) {
		*count = 0;
		return OSAL_SUCCESS;
	}

	configs[0] = &g_lpf_config_dt.platform;
	*count = 1;
	return OSAL_SUCCESS;
}

const lpf_config_backend_ops_t g_lpf_config_dt_backend = {
	.name = "dt",
	.available = lpf_config_dt_available,
	.load = lpf_config_dt_load,
	.unload = lpf_config_dt_unload,
	.active = lpf_config_dt_active,
	.find = lpf_config_dt_find,
	.list = lpf_config_dt_list,
};
