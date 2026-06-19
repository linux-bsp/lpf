// SPDX-License-Identifier: GPL-2.0

#include "osal.h"
#include "pconfig_backend.h"

#include <linux/of.h>

#define PCONFIG_DT_COMPATIBLE "linux-peripheral-framework"
#define PCONFIG_DT_LEGACY_COMPATIBLE "lpf,platform-config"
#define PCONFIG_DT_ROOT_PATH "/lpf"

typedef struct {
	pconfig_platform_config_t platform;
	pconfig_mcu_entry_t *mcu_entries;
	pconfig_led_entry_t *led_entries;
} pconfig_dt_context_t;

static pconfig_dt_context_t g_pconfig_dt;
static bool g_pconfig_dt_loaded;

static const char *pconfig_dt_string_or_default(const char *value,
						const char *fallback)
{
	return value ? value : fallback;
}

static int32_t pconfig_dt_read_string(const struct device_node *node,
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

	*out = pconfig_dt_string_or_default(value, fallback);
	return OSAL_SUCCESS;
}

static uint32_t pconfig_dt_read_u32_default(const struct device_node *node,
					    const char *property,
					    uint32_t fallback)
{
	uint32_t value;

	if (0 == of_property_read_u32(node, property, &value))
		return value;

	return fallback;
}

static bool pconfig_dt_read_bool_default(const struct device_node *node,
					 const char *property, bool fallback)
{
	if (of_property_read_bool(node, property))
		return true;

	return fallback;
}

static struct device_node *pconfig_dt_find_root(void)
{
	struct device_node *root;

	root = of_find_node_by_path(PCONFIG_DT_ROOT_PATH);
	if (root)
		return root;

	root = of_find_compatible_node(NULL, NULL, PCONFIG_DT_COMPATIBLE);
	if (root)
		return root;

	return of_find_compatible_node(NULL, NULL, PCONFIG_DT_LEGACY_COMPATIBLE);
}

static struct device_node *pconfig_dt_find_child(const struct device_node *root,
						 const char *name)
{
	struct device_node *child;

	for_each_available_child_of_node(root, child) {
		if (0 == osal_strcmp(child->name, name))
			return child;
	}

	return NULL;
}

static pconfig_mcu_interface_t pconfig_dt_parse_mcu_interface(const char *value)
{
	if (value && 0 == osal_strcmp(value, "serial"))
		return PCONFIG_MCU_INTERFACE_SERIAL;

	if (value && 0 == osal_strcmp(value, "i2c"))
		return PCONFIG_MCU_INTERFACE_I2C;

	if (value && 0 == osal_strcmp(value, "spi"))
		return PCONFIG_MCU_INTERFACE_SPI;

	return PCONFIG_MCU_INTERFACE_CAN;
}

static pconfig_mcu_parity_t pconfig_dt_parse_parity(const char *value)
{
	if (value && 0 == osal_strcmp(value, "odd"))
		return PCONFIG_MCU_PARITY_ODD;

	if (value && 0 == osal_strcmp(value, "even"))
		return PCONFIG_MCU_PARITY_EVEN;

	return PCONFIG_MCU_PARITY_NONE;
}

static pconfig_mcu_flow_control_t pconfig_dt_parse_flow_control(
	const char *value)
{
	if (value && 0 == osal_strcmp(value, "hw"))
		return PCONFIG_MCU_FLOW_HW;

	if (value && 0 == osal_strcmp(value, "sw"))
		return PCONFIG_MCU_FLOW_SW;

	return PCONFIG_MCU_FLOW_NONE;
}

static pconfig_led_control_t pconfig_dt_parse_led_control(const char *value)
{
	if (value && 0 == osal_strcmp(value, "pwm"))
		return PCONFIG_LED_CONTROL_PWM;

	return PCONFIG_LED_CONTROL_GPIO;
}

static int32_t pconfig_dt_count_available_children(
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

static void pconfig_dt_free_entries(void)
{
	osal_free(g_pconfig_dt.mcu_entries);
	osal_free(g_pconfig_dt.led_entries);
	g_pconfig_dt.mcu_entries = NULL;
	g_pconfig_dt.led_entries = NULL;
}

static int32_t pconfig_dt_parse_mcu_can(struct device_node *node,
					pconfig_mcu_config_t *config)
{
	const char *device;
	int32_t ret;

	ret = pconfig_dt_read_string(node, "device", &device, NULL);
	if (ret != OSAL_SUCCESS)
		return ret;

	config->hw.can.device = device;
	config->hw.can.bitrate =
		pconfig_dt_read_u32_default(node, "bitrate", 500000U);
	config->hw.can.rx_timeout =
		pconfig_dt_read_u32_default(node, "rx-timeout-ms", 1000U);
	config->hw.can.tx_timeout =
		pconfig_dt_read_u32_default(node, "tx-timeout-ms", 1000U);
	config->hw.can.tx_id =
		pconfig_dt_read_u32_default(node, "tx-id", 0U);
	config->hw.can.rx_id =
		pconfig_dt_read_u32_default(node, "rx-id", 0U);
	return OSAL_SUCCESS;
}

static int32_t pconfig_dt_parse_mcu_serial(struct device_node *node,
					   pconfig_mcu_config_t *config)
{
	const char *device;
	const char *parity = NULL;
	const char *flow_control = NULL;
	int32_t ret;

	ret = pconfig_dt_read_string(node, "device", &device, NULL);
	if (ret != OSAL_SUCCESS)
		return ret;

	(void)of_property_read_string(node, "parity", &parity);
	(void)of_property_read_string(node, "flow-control", &flow_control);

	config->hw.serial.device = device;
	config->hw.serial.baudrate =
		pconfig_dt_read_u32_default(node, "baudrate", 115200U);
	config->hw.serial.data_bits =
		(uint8_t)pconfig_dt_read_u32_default(node, "data-bits", 8U);
	config->hw.serial.stop_bits =
		(uint8_t)pconfig_dt_read_u32_default(node, "stop-bits", 1U);
	config->hw.serial.parity = pconfig_dt_parse_parity(parity);
	config->hw.serial.flow_control =
		pconfig_dt_parse_flow_control(flow_control);
	return OSAL_SUCCESS;
}

static int32_t pconfig_dt_parse_mcu_entry(struct device_node *node,
					  pconfig_mcu_entry_t *entry)
{
	const char *name;
	const char *interface = NULL;
	int32_t ret;

	ret = pconfig_dt_read_string(node, "label", &name, node->name);
	if (ret != OSAL_SUCCESS)
		return ret;

	(void)of_property_read_string(node, "interface", &interface);

	entry->description = pconfig_dt_string_or_default(name, "mcu");
	entry->enabled = true;
	osal_strncpy(entry->config.name, name,
		     sizeof(entry->config.name) - 1U);
	entry->config.name[sizeof(entry->config.name) - 1U] = '\0';
	entry->config.interface = pconfig_dt_parse_mcu_interface(interface);
	entry->config.cmd_timeout_ms =
		pconfig_dt_read_u32_default(node, "cmd-timeout-ms", 1000U);
	entry->config.retry_count =
		pconfig_dt_read_u32_default(node, "retry-count", 3U);

	switch (entry->config.interface) {
	case PCONFIG_MCU_INTERFACE_CAN:
		return pconfig_dt_parse_mcu_can(node, &entry->config);
	case PCONFIG_MCU_INTERFACE_SERIAL:
		return pconfig_dt_parse_mcu_serial(node, &entry->config);
	default:
		return OSAL_ERR_NOT_SUPPORTED;
	}
}

static int32_t pconfig_dt_parse_mcu_group(struct device_node *root)
{
	struct device_node *group;
	struct device_node *node;
	uint32_t count;
	uint32_t index = 0;
	int32_t ret = OSAL_SUCCESS;

	group = pconfig_dt_find_child(root, "mcu");
	if (!group)
		return OSAL_SUCCESS;

	count = pconfig_dt_count_available_children(group);
	if (0 == count)
		goto out_put_group;

	g_pconfig_dt.mcu_entries =
		osal_zalloc(sizeof(*g_pconfig_dt.mcu_entries) * count);
	if (!g_pconfig_dt.mcu_entries) {
		ret = OSAL_ERR_NO_MEMORY;
		goto out_put_group;
	}

	for_each_available_child_of_node(group, node) {
		ret = pconfig_dt_parse_mcu_entry(
			node, &g_pconfig_dt.mcu_entries[index]);
		if (ret != OSAL_SUCCESS) {
			of_node_put(node);
			goto out_put_group;
		}
		index++;
	}

	g_pconfig_dt.platform.mcu_count = count;
	g_pconfig_dt.platform.mcu_array = g_pconfig_dt.mcu_entries;
	ret = OSAL_SUCCESS;

out_put_group:
	of_node_put(group);
	return ret;
}

static int32_t pconfig_dt_parse_led_entry(struct device_node *node,
					  pconfig_led_entry_t *entry)
{
	const char *name;
	const char *control = NULL;
	int32_t ret;

	ret = pconfig_dt_read_string(node, "label", &name, node->name);
	if (ret != OSAL_SUCCESS)
		return ret;

	(void)of_property_read_string(node, "control", &control);

	entry->description = pconfig_dt_string_or_default(name, "led");
	entry->enabled = true;
	osal_strncpy(entry->config.name, name,
		     sizeof(entry->config.name) - 1U);
	entry->config.name[sizeof(entry->config.name) - 1U] = '\0';
	entry->config.control = pconfig_dt_parse_led_control(control);
	entry->config.max_brightness =
		pconfig_dt_read_u32_default(node, "max-brightness", 255U);
	entry->config.default_brightness =
		pconfig_dt_read_u32_default(node, "default-brightness", 0U);

	switch (entry->config.control) {
	case PCONFIG_LED_CONTROL_GPIO:
		entry->config.hw.gpio.gpio_num =
			pconfig_dt_read_u32_default(node, "gpio", 0U);
		entry->config.hw.gpio.pin_mux =
			pconfig_dt_read_u32_default(node, "pin-mux", 0U);
		entry->config.hw.gpio.active_low =
			pconfig_dt_read_bool_default(node, "active-low", false);
		entry->config.hw.gpio.pull_up =
			pconfig_dt_read_bool_default(node, "pull-up", false);
		entry->config.hw.gpio.pull_down =
			pconfig_dt_read_bool_default(node, "pull-down", false);
		break;
	case PCONFIG_LED_CONTROL_PWM:
		ret = pconfig_dt_read_string(node, "consumer",
					     &entry->config.hw.pwm.consumer,
					     NULL);
		if (ret != OSAL_SUCCESS)
			return ret;
		entry->config.hw.pwm.period_ns =
			pconfig_dt_read_u32_default(node, "period-ns", 0U);
		entry->config.hw.pwm.polarity_inversed =
			pconfig_dt_read_bool_default(
				node, "polarity-inversed", false);
		break;
	default:
		return OSAL_ERR_NOT_SUPPORTED;
	}

	return OSAL_SUCCESS;
}

static int32_t pconfig_dt_parse_led_group(struct device_node *root)
{
	struct device_node *group;
	struct device_node *node;
	uint32_t count;
	uint32_t index = 0;
	int32_t ret = OSAL_SUCCESS;

	group = pconfig_dt_find_child(root, "led");
	if (!group)
		return OSAL_SUCCESS;

	count = pconfig_dt_count_available_children(group);
	if (0 == count)
		goto out_put_group;

	g_pconfig_dt.led_entries =
		osal_zalloc(sizeof(*g_pconfig_dt.led_entries) * count);
	if (!g_pconfig_dt.led_entries) {
		ret = OSAL_ERR_NO_MEMORY;
		goto out_put_group;
	}

	for_each_available_child_of_node(group, node) {
		ret = pconfig_dt_parse_led_entry(
			node, &g_pconfig_dt.led_entries[index]);
		if (ret != OSAL_SUCCESS) {
			of_node_put(node);
			goto out_put_group;
		}
		index++;
	}

	g_pconfig_dt.platform.led_count = count;
	g_pconfig_dt.platform.led_array = g_pconfig_dt.led_entries;
	ret = OSAL_SUCCESS;

out_put_group:
	of_node_put(group);
	return ret;
}

static bool pconfig_dt_available(void)
{
	struct device_node *root;

	root = pconfig_dt_find_root();
	if (!root)
		return false;

	of_node_put(root);
	return true;
}

static int32_t pconfig_dt_load(void)
{
	struct device_node *root;
	int32_t ret;

	if (g_pconfig_dt_loaded)
		return OSAL_SUCCESS;

	root = pconfig_dt_find_root();
	if (!root)
		return OSAL_ERR_NOT_SUPPORTED;

	osal_memset(&g_pconfig_dt, 0, sizeof(g_pconfig_dt));

	ret = pconfig_dt_read_string(root, "platform-name",
				     &g_pconfig_dt.platform.platform_name,
				     "linux");
	if (ret != OSAL_SUCCESS)
		goto out_error;

	ret = pconfig_dt_read_string(root, "chip-name",
				     &g_pconfig_dt.platform.chip_name,
				     "unknown");
	if (ret != OSAL_SUCCESS)
		goto out_error;

	ret = pconfig_dt_read_string(root, "project-name",
				     &g_pconfig_dt.platform.project_name,
				     "default");
	if (ret != OSAL_SUCCESS)
		goto out_error;

	ret = pconfig_dt_read_string(root, "product-name",
				     &g_pconfig_dt.platform.product_name,
				     "lpf");
	if (ret != OSAL_SUCCESS)
		goto out_error;

	ret = pconfig_dt_read_string(root, "config-version",
				     &g_pconfig_dt.platform.version,
				     "1.0.0");
	if (ret != OSAL_SUCCESS)
		goto out_error;

	ret = pconfig_dt_parse_mcu_group(root);
	if (ret != OSAL_SUCCESS)
		goto out_error;

	ret = pconfig_dt_parse_led_group(root);
	if (ret != OSAL_SUCCESS)
		goto out_error;

	g_pconfig_dt_loaded = true;
	of_node_put(root);
	return OSAL_SUCCESS;

out_error:
	pconfig_dt_free_entries();
	osal_memset(&g_pconfig_dt, 0, sizeof(g_pconfig_dt));
	of_node_put(root);
	return ret;
}

static void pconfig_dt_unload(void)
{
	pconfig_dt_free_entries();
	osal_memset(&g_pconfig_dt, 0, sizeof(g_pconfig_dt));
	g_pconfig_dt_loaded = false;
}

static const pconfig_platform_config_t *pconfig_dt_active(void)
{
	return g_pconfig_dt_loaded ? &g_pconfig_dt.platform : NULL;
}

static const pconfig_platform_config_t *
pconfig_dt_find(const char *product, const char *project, const char *version)
{
	const pconfig_platform_config_t *config;

	config = pconfig_dt_active();
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

static int32_t pconfig_dt_list(const pconfig_platform_config_t **configs,
			       uint32_t *count)
{
	if (!configs || !count)
		return OSAL_ERR_GENERIC;

	if (0 == *count) {
		*count = 0;
		return OSAL_SUCCESS;
	}

	if (!g_pconfig_dt_loaded) {
		*count = 0;
		return OSAL_SUCCESS;
	}

	configs[0] = &g_pconfig_dt.platform;
	*count = 1;
	return OSAL_SUCCESS;
}

const pconfig_backend_ops_t g_pconfig_dt_backend = {
	.name = "dt",
	.available = pconfig_dt_available,
	.load = pconfig_dt_load,
	.unload = pconfig_dt_unload,
	.active = pconfig_dt_active,
	.find = pconfig_dt_find,
	.list = pconfig_dt_list,
};
