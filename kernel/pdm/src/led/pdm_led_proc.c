// SPDX-License-Identifier: GPL-2.0

#include "pdm_led_internal.h"
#include "pdm_proc.h"
#include "pdm_status.h"

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/kstrtox.h>
#include <linux/string.h>

static pdm_proc_entry_t g_pdm_led_proc;

static char *pdm_led_proc_next_token(char **cursor)
{
	char *token;

	while ((token = strsep(cursor, " \t\r\n")) != NULL) {
		if (*token != '\0')
			return token;
	}

	return NULL;
}

static int pdm_led_proc_parse_u32(char **cursor, uint32_t *value)
{
	char *token = pdm_led_proc_next_token(cursor);

	if (!token)
		return -EINVAL;

	return kstrtou32(token, 0, value);
}

static pdm_led_handle_t pdm_led_proc_get_handle(uint32_t index)
{
	if (index >= PDM_LED_MAX_DEVICES)
		return NULL;

	return pdm_led_get(index);
}

static const char *pdm_led_control_name(pconfig_led_control_t control)
{
	switch (control) {
	case PCONFIG_LED_CONTROL_GPIO:
		return "gpio";
	case PCONFIG_LED_CONTROL_PWM:
		return "pwm";
	default:
		return "unknown";
	}
}

static int pdm_led_proc_show(struct seq_file *seq, void *data)
{
	pdm_led_debug_info_t info;
	uint32_t i;
	int32_t ret;

	(void)data;

	seq_puts(seq, "PDM LED\n");
	seq_printf(seq, "max_devices=%u\n", PDM_LED_MAX_DEVICES);
	seq_puts(seq, "devices:\n");

	for (i = 0; i < PDM_LED_MAX_DEVICES; i++) {
		ret = pdm_led_debug_get(i, &info);
		if (ret == OSAL_ERR_INVALID_STATE) {
			seq_puts(seq, "  registry=uninitialized\n");
			return 0;
		}
		if (ret != OSAL_SUCCESS)
			continue;

		if (!info.present) {
			seq_printf(seq, "  [%u] present=0\n", i);
			continue;
		}

		seq_printf(seq,
			   "  [%u] present=1 name=%s control=%s enabled=%u brightness=%u max_brightness=%u\n",
			   i, info.name, pdm_led_control_name(info.control),
			   info.enabled ? 1U : 0U, info.brightness,
			   info.max_brightness);
	}

	seq_puts(seq, "write_commands:\n");
	seq_puts(seq, "  state <index>\n");
	seq_puts(seq, "  enable <index>\n");
	seq_puts(seq, "  disable <index>\n");
	seq_puts(seq, "  set <index> <brightness>\n");

	return 0;
}

static int pdm_led_proc_do_state(pdm_led_handle_t handle, uint32_t index)
{
	pdm_led_state_t state;
	int32_t ret;

	osal_memset(&state, 0, sizeof(state));
	ret = pdm_led_get_state(handle, &state);
	if (ret != OSAL_SUCCESS)
		return pdm_status_to_errno(ret);

	LOG_INFO("PDM_LED",
		 "proc state index=%u enabled=%u brightness=%u max_brightness=%u",
		 index, state.enabled ? 1U : 0U, state.brightness,
		 state.max_brightness);
	return 0;
}

static int pdm_led_proc_write(char *command, size_t count, void *data)
{
	pdm_led_handle_t handle;
	char *cursor = command;
	char *op;
	uint32_t index;
	uint32_t brightness;
	int32_t status;
	int ret;

	(void)count;
	(void)data;

	op = pdm_led_proc_next_token(&cursor);
	if (!op)
		return -EINVAL;

	ret = pdm_led_proc_parse_u32(&cursor, &index);
	if (ret)
		return ret;

	handle = pdm_led_proc_get_handle(index);
	if (!handle)
		return -ENODEV;

	if (!strcmp(op, "state"))
		return pdm_led_proc_do_state(handle, index);

	if (!strcmp(op, "enable") || !strcmp(op, "on")) {
		status = pdm_led_enable(handle);
		if (status != OSAL_SUCCESS)
			return pdm_status_to_errno(status);
		LOG_INFO("PDM_LED", "proc enable index=%u success", index);
		return 0;
	}

	if (!strcmp(op, "disable") || !strcmp(op, "off")) {
		status = pdm_led_disable(handle);
		if (status != OSAL_SUCCESS)
			return pdm_status_to_errno(status);
		LOG_INFO("PDM_LED", "proc disable index=%u success", index);
		return 0;
	}

	if (!strcmp(op, "set") || !strcmp(op, "brightness")) {
		ret = pdm_led_proc_parse_u32(&cursor, &brightness);
		if (ret)
			return ret;

		status = pdm_led_set_brightness(handle, brightness);
		if (status != OSAL_SUCCESS)
			return pdm_status_to_errno(status);
		LOG_INFO("PDM_LED",
			 "proc set index=%u brightness=%u success",
			 index, brightness);
		return 0;
	}

	return -EINVAL;
}

int pdm_led_proc_register(void)
{
	return pdm_proc_register(&g_pdm_led_proc, "led",
				 pdm_led_proc_show, pdm_led_proc_write, NULL);
}

void pdm_led_proc_unregister(void)
{
	pdm_proc_unregister(&g_pdm_led_proc);
}
