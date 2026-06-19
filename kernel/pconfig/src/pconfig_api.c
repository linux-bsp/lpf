/************************************************************************
 * 硬件配置库API实现
 *
 * 命名规范：
 * - PCONFIG_*       - 通用接口
 * - PCONFIG_HW_*    - 硬件配置接口
 ************************************************************************/

#include "osal.h"
#include "pconfig/pconfig.h"
#include "generated/gen_version.h"

#include <linux/module.h>

#define PCONFIG_MAX_DEVICES 32U

typedef uint32_t (*pconfig_device_count_fn)(
	const pconfig_platform_config_t *platform);
typedef const void *(*pconfig_device_entry_fn)(
	const pconfig_platform_config_t *platform, uint32_t index);
typedef bool (*pconfig_device_enabled_fn)(const void *entry);
typedef int32_t (*pconfig_device_validate_fn)(uint32_t index,
					      const void *entry);
typedef void (*pconfig_device_print_fn)(uint32_t index, const void *entry);

typedef struct {
	const char *name;
	pconfig_device_type_t type;
	pconfig_device_count_fn count;
	pconfig_device_entry_fn entry;
	pconfig_device_enabled_fn enabled;
	pconfig_device_validate_fn validate;
	pconfig_device_print_fn print;
} pconfig_device_descriptor_t;

static pconfig_device_config_t g_pconfig_devices[PCONFIG_MAX_DEVICES + 1U];
static bool g_pconfig_initialized;

static bool pconfig_table_is_valid(void)
{
	return g_pconfig_platform_table.configs &&
	       g_pconfig_platform_table.count > 0 &&
	       g_pconfig_platform_table.current_index <
		       g_pconfig_platform_table.count;
}

static uint32_t pconfig_mcu_count(const pconfig_platform_config_t *platform)
{
	return platform->mcu_count;
}

static const void *pconfig_mcu_entry(const pconfig_platform_config_t *platform,
				     uint32_t index)
{
	return pconfig_hw_get_mcu(platform, index);
}

static bool pconfig_mcu_enabled(const void *entry)
{
	const pconfig_mcu_entry_t *mcu = entry;

	return mcu && mcu->enabled;
}

static bool pconfig_mcu_parity_valid(pconfig_mcu_parity_t parity)
{
	return parity == PCONFIG_MCU_PARITY_NONE ||
	       parity == PCONFIG_MCU_PARITY_ODD ||
	       parity == PCONFIG_MCU_PARITY_EVEN;
}

static bool
pconfig_mcu_flow_control_valid(pconfig_mcu_flow_control_t flow_control)
{
	return flow_control == PCONFIG_MCU_FLOW_NONE ||
	       flow_control == PCONFIG_MCU_FLOW_HW ||
	       flow_control == PCONFIG_MCU_FLOW_SW;
}

static int32_t pconfig_validate_mcu_can(uint32_t index,
					const pconfig_mcu_config_t *config)
{
	if (NULL == config->hw.can.device || 0 == config->hw.can.bitrate) {
		LOG_ERROR("PCONFIG", "MCU[%u] invalid CAN config", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	if (0 == config->hw.can.rx_timeout || 0 == config->hw.can.tx_timeout) {
		LOG_ERROR("PCONFIG", "MCU[%u] invalid CAN timeout", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	return OSAL_SUCCESS;
}

static int32_t pconfig_validate_mcu_serial(uint32_t index,
					   const pconfig_mcu_config_t *config)
{
	if (NULL == config->hw.serial.device ||
	    0 == config->hw.serial.baudrate) {
		LOG_ERROR("PCONFIG", "MCU[%u] invalid serial config", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	if (config->hw.serial.data_bits < 5 ||
	    config->hw.serial.data_bits > 8 ||
	    config->hw.serial.stop_bits < 1 ||
	    config->hw.serial.stop_bits > 2) {
		LOG_ERROR("PCONFIG", "MCU[%u] invalid serial frame config",
			  index);
		return OSAL_ERR_INVALID_PARAM;
	}

	if (!pconfig_mcu_parity_valid(config->hw.serial.parity) ||
	    !pconfig_mcu_flow_control_valid(config->hw.serial.flow_control)) {
		LOG_ERROR("PCONFIG", "MCU[%u] invalid serial mode", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	return OSAL_SUCCESS;
}

static int32_t pconfig_validate_mcu_entry(uint32_t index, const void *entry)
{
	const pconfig_mcu_entry_t *mcu = entry;
	const pconfig_mcu_config_t *config;

	if (NULL == mcu)
		return OSAL_ERR_INVALID_PARAM;

	if (!mcu->enabled)
		return OSAL_SUCCESS;

	config = &mcu->config;
	if ('\0' == config->name[0]) {
		LOG_ERROR("PCONFIG", "MCU[%u] missing name", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	if (0 == config->cmd_timeout_ms) {
		LOG_ERROR("PCONFIG", "MCU[%u] invalid command timeout", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	switch (config->interface) {
	case PCONFIG_MCU_INTERFACE_CAN:
		return pconfig_validate_mcu_can(index, config);
	case PCONFIG_MCU_INTERFACE_SERIAL:
		return pconfig_validate_mcu_serial(index, config);
	default:
		LOG_ERROR("PCONFIG", "MCU[%u] unsupported interface", index);
		return OSAL_ERR_NOT_SUPPORTED;
	}
}

static void pconfig_print_mcu_entry(uint32_t index, const void *entry)
{
	const pconfig_mcu_entry_t *mcu = entry;

	LOG_INFO("PCONFIG", "  MCU[%u]: %s", index,
		 mcu && mcu->description ? mcu->description : "N/A");
}

static uint32_t pconfig_led_count(const pconfig_platform_config_t *platform)
{
	return platform->led_count;
}

static const void *pconfig_led_entry(const pconfig_platform_config_t *platform,
				     uint32_t index)
{
	return pconfig_hw_get_led(platform, index);
}

static bool pconfig_led_enabled(const void *entry)
{
	const pconfig_led_entry_t *led = entry;

	return led && led->enabled;
}

static int32_t pconfig_validate_led_entry(uint32_t index,
					  const void *entry)
{
	const pconfig_led_entry_t *led = entry;
	const pconfig_led_config_t *config;

	if (NULL == led)
		return OSAL_ERR_INVALID_PARAM;

	if (!led->enabled)
		return OSAL_SUCCESS;

	config = &led->config;
	if ('\0' == config->name[0]) {
		LOG_ERROR("PCONFIG", "LED[%u] missing name", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	if (0 == config->max_brightness) {
		LOG_ERROR("PCONFIG", "LED[%u] max brightness is zero", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	if (config->default_brightness > config->max_brightness) {
		LOG_ERROR("PCONFIG", "LED[%u] default brightness exceeds max",
			  index);
		return OSAL_ERR_INVALID_PARAM;
	}

	switch (config->control) {
	case PCONFIG_LED_CONTROL_GPIO:
		break;
	case PCONFIG_LED_CONTROL_PWM:
		if (NULL == config->hw.pwm.consumer ||
		    0 == config->hw.pwm.period_ns) {
			LOG_ERROR("PCONFIG", "LED[%u] invalid PWM config",
				  index);
			return OSAL_ERR_INVALID_PARAM;
		}
		break;
	default:
		LOG_ERROR("PCONFIG", "LED[%u] invalid control type", index);
		return OSAL_ERR_INVALID_PARAM;
	}

	return OSAL_SUCCESS;
}

static void pconfig_print_led_entry(uint32_t index, const void *entry)
{
	const pconfig_led_entry_t *led = entry;

	LOG_INFO("PCONFIG", "  LED[%u]: %s", index,
		 led && led->description ? led->description : "N/A");
}

static const pconfig_device_descriptor_t g_pconfig_device_descriptors[] = {
	{
		.name = "MCU",
		.type = PCONFIG_DEVICE_TYPE_MCU,
		.count = pconfig_mcu_count,
		.entry = pconfig_mcu_entry,
		.enabled = pconfig_mcu_enabled,
		.validate = pconfig_validate_mcu_entry,
		.print = pconfig_print_mcu_entry,
	},
	{
		.name = "LED",
		.type = PCONFIG_DEVICE_TYPE_LED,
		.count = pconfig_led_count,
		.entry = pconfig_led_entry,
		.enabled = pconfig_led_enabled,
		.validate = pconfig_validate_led_entry,
		.print = pconfig_print_led_entry,
	},
};

static int32_t
pconfig_build_device_list(const pconfig_platform_config_t *platform)
{
	uint32_t out_index = 0;
	uint32_t desc_index;

	osal_memset(g_pconfig_devices, 0, sizeof(g_pconfig_devices));

	for (desc_index = 0;
	     desc_index < OSAL_ARRAY_SIZE(g_pconfig_device_descriptors);
	     desc_index++) {
		const pconfig_device_descriptor_t *desc;
		uint32_t count;
		uint32_t i;

		desc = &g_pconfig_device_descriptors[desc_index];
		count = desc->count(platform);
		for (i = 0; i < count; i++) {
			const void *entry;

			if (out_index >= PCONFIG_MAX_DEVICES) {
				LOG_ERROR("PCONFIG",
					  "Device list capacity exceeded");
				return OSAL_ERR_RESOURCE_LIMIT;
			}

			entry = desc->entry(platform, i);
			if (!desc->enabled(entry))
				continue;

			g_pconfig_devices[out_index].device_type = desc->type;
			g_pconfig_devices[out_index].index = i;
			g_pconfig_devices[out_index].entry = entry;
			out_index++;
		}
	}

	g_pconfig_devices[out_index].device_type = PCONFIG_DEVICE_TYPE_INVALID;
	LOG_INFO("PCONFIG", "Initialized %u device configs", out_index);
	return OSAL_SUCCESS;
}

const pconfig_platform_config_t *pconfig_get_board(void)
{
	if (!pconfig_table_is_valid())
		return NULL;

	return g_pconfig_platform_table
		.configs[g_pconfig_platform_table.current_index];
}
EXPORT_SYMBOL_GPL(pconfig_get_board);

const pconfig_device_config_t *pconfig_get(void)
{
	return g_pconfig_initialized ? g_pconfig_devices : NULL;
}
EXPORT_SYMBOL_GPL(pconfig_get);

const pconfig_platform_config_t *pconfig_find(const char *product,
					      const char *project,
					      const char *version)
{
	uint32_t i;
	const pconfig_platform_config_t *config;

	if (NULL == product || NULL == project ||
	    NULL == g_pconfig_platform_table.configs)
		return NULL;

	for (i = 0; i < g_pconfig_platform_table.count; i++) {
		config = g_pconfig_platform_table.configs[i];
		if (NULL == config)
			continue;

		if (NULL == config->product_name || NULL == config->project_name)
			continue;

		if (0 != osal_strcmp(config->product_name, product))
			continue;

		if (0 != osal_strcmp(config->project_name, project))
			continue;

		if (version) {
			if (!config->version)
				continue;
			if (0 != osal_strcmp(config->version, version))
				continue;
		}

		return config;
	}

	return NULL;
}
EXPORT_SYMBOL_GPL(pconfig_find);

int32_t pconfig_list(const pconfig_platform_config_t **configs, uint32_t *count)
{
	uint32_t max_count;
	uint32_t actual_count;
	uint32_t i;

	if (NULL == configs || NULL == count)
		return OSAL_ERR_GENERIC;

	if (NULL == g_pconfig_platform_table.configs) {
		*count = 0;
		return OSAL_SUCCESS;
	}

	max_count = *count;
	actual_count = (g_pconfig_platform_table.count < max_count) ?
			       g_pconfig_platform_table.count :
			       max_count;

	for (i = 0; i < actual_count; i++)
		configs[i] = g_pconfig_platform_table.configs[i];

	*count = actual_count;
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(pconfig_list);

int32_t pconfig_validate(const pconfig_platform_config_t *config)
{
	uint32_t desc_index;
	uint32_t i;
	int32_t ret;

	if (NULL == config)
		return OSAL_ERR_GENERIC;

	if (NULL == config->platform_name || NULL == config->chip_name ||
	    NULL == config->project_name || NULL == config->product_name ||
	    NULL == config->version) {
		LOG_ERROR("PCONFIG", "Missing platform identity");
		return OSAL_ERR_GENERIC;
	}

	if (config->mcu_count > 0 && NULL == config->mcu_array) {
		LOG_ERROR("PCONFIG", "Missing MCU config array");
		return OSAL_ERR_GENERIC;
	}

	if (config->led_count > 0 && NULL == config->led_array) {
		LOG_ERROR("PCONFIG", "Missing LED config array");
		return OSAL_ERR_GENERIC;
	}

	for (desc_index = 0;
	     desc_index < OSAL_ARRAY_SIZE(g_pconfig_device_descriptors);
	     desc_index++) {
		const pconfig_device_descriptor_t *desc;
		uint32_t count;

		desc = &g_pconfig_device_descriptors[desc_index];
		count = desc->count(config);
		for (i = 0; i < count; i++) {
			ret = desc->validate(i, desc->entry(config, i));
			if (ret != OSAL_SUCCESS)
				return ret;
		}
	}

	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(pconfig_validate);

void pconfig_print(const pconfig_platform_config_t *config)
{
	uint32_t desc_index;

	if (NULL == config)
		return;

	LOG_INFO("PCONFIG", "Platform: %s", config->platform_name);
	LOG_INFO("PCONFIG", "Chip: %s", config->chip_name);
	LOG_INFO("PCONFIG", "Project: %s", config->project_name);
	LOG_INFO("PCONFIG", "Product: %s", config->product_name);
	LOG_INFO("PCONFIG", "Version: %s", config->version);

	for (desc_index = 0;
	     desc_index < OSAL_ARRAY_SIZE(g_pconfig_device_descriptors);
	     desc_index++) {
		const pconfig_device_descriptor_t *desc;
		uint32_t count;
		uint32_t i;

		desc = &g_pconfig_device_descriptors[desc_index];
		count = desc->count(config);
		for (i = 0; i < count; i++)
			desc->print(i, desc->entry(config, i));
	}
}
EXPORT_SYMBOL_GPL(pconfig_print);

void pconfig_print_version(void)
{
	osal_log(OS_LOG_LEVEL_INFO, "PCONFIG",
		 "module_version=%u.%u.%u lpf_version=%s git=%s build_time=%s build_by=%s@%s compiler=%s arch=%s kernel=%s",
		 PCONFIG_VERSION_MAJOR, PCONFIG_VERSION_MINOR,
		 PCONFIG_VERSION_PATCH, LPF_VERSION,
		 LPF_GIT_COMMIT, LPF_COMPILE_TIME,
		 LPF_COMPILE_BY, LPF_COMPILE_HOST,
		 LPF_COMPILER, LPF_BUILD_ARCH,
		 LPF_BUILD_KERNEL);
}
EXPORT_SYMBOL_GPL(pconfig_print_version);

int32_t pconfig_load(void)
{
	const pconfig_platform_config_t *platform;
	int32_t ret;

	if (g_pconfig_initialized)
		return OSAL_SUCCESS;

	pconfig_print_version();

	platform = pconfig_get_board();
	if (NULL == platform) {
		LOG_ERROR("PCONFIG", "No active platform config");
		return OSAL_ERR_GENERIC;
	}

	ret = pconfig_validate(platform);
	if (ret != OSAL_SUCCESS) {
		LOG_ERROR("PCONFIG", "Invalid platform config");
		return ret;
	}

	ret = pconfig_build_device_list(platform);
	if (ret != OSAL_SUCCESS)
		return ret;

	pconfig_print(platform);
	g_pconfig_initialized = true;
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(pconfig_load);

void pconfig_unload(void)
{
	osal_memset(g_pconfig_devices, 0, sizeof(g_pconfig_devices));
	g_pconfig_initialized = false;
	LOG_INFO("PCONFIG", "Deinitialized");
}
EXPORT_SYMBOL_GPL(pconfig_unload);

static int __init pconfig_init(void)
{
	int32_t ret;

	ret = pconfig_load();
	if (ret != OSAL_SUCCESS)
		return -ret;

	LOG_INFO("PCONFIG", "loaded");
	return 0;
}

static void __exit pconfig_exit(void)
{
	pconfig_unload();
	LOG_INFO("PCONFIG", "unloaded");
}

module_init(pconfig_init);
module_exit(pconfig_exit);

MODULE_AUTHOR("LPF");
MODULE_DESCRIPTION("LPF PConfig kernel module");
MODULE_LICENSE("GPL");
MODULE_SOFTDEP("pre: osal");
