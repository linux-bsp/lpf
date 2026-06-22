// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>

#include "pdm/core/pdm_core.h"
#include "pdm/pdm_errno.h"

#define PDM_DUMMY_SELFTEST_INDEX 0U
#define PDM_DUMMY_SELFTEST_NAME "dummy-selftest0"
#define PDM_DUMMY_SELFTEST_DRIVER "dummy-selftest"
#define PDM_DUMMY_SELFTEST_CAP (1ULL << 63)

typedef struct {
	uint32_t init_count;
	uint32_t exit_count;
	uint32_t probe_count;
	uint32_t remove_count;
	uint32_t events[PDM_DEVICE_EVENT_REMOVED + 1U];
} pdm_dummy_selftest_state_t;

static pdm_dummy_selftest_state_t g_lpf_dummy_selftest_state;

static int32_t pdm_dummy_selftest_fail(const char *message)
{
	pr_err("PDM:DUMMY_SELFTEST: %s\n", message);
	return OSAL_ERR_GENERIC;
}

#define PDM_DUMMY_SELFTEST_EXPECT(condition, message) \
	do { \
		if (!(condition)) \
			return pdm_dummy_selftest_fail(message); \
	} while (0)

static bool pdm_dummy_selftest_is_target(const pdm_device_info_t *info)
{
	return info && info->type == PDM_DEVICE_TYPE_DUMMY &&
	       info->index == PDM_DUMMY_SELFTEST_INDEX;
}

static void pdm_dummy_selftest_event(const pdm_device_event_t *event,
				     void *user_data)
{
	pdm_dummy_selftest_state_t *state = user_data;

	if (!event || !state || !pdm_dummy_selftest_is_target(&event->device))
		return;

	if (event->type <= PDM_DEVICE_EVENT_REMOVED)
		state->events[event->type]++;
}

static int pdm_dummy_selftest_driver_init(void)
{
	g_lpf_dummy_selftest_state.init_count++;
	return 0;
}

static void pdm_dummy_selftest_driver_exit(void)
{
	g_lpf_dummy_selftest_state.exit_count++;
}

static int32_t pdm_dummy_selftest_probe(const pdm_device_t *device)
{
	PDM_DUMMY_SELFTEST_EXPECT(device != NULL, "probe device is NULL");
	PDM_DUMMY_SELFTEST_EXPECT(device->config.type == PDM_DEVICE_TYPE_DUMMY,
				  "probe type mismatch");
	PDM_DUMMY_SELFTEST_EXPECT(device->config.index ==
					  PDM_DUMMY_SELFTEST_INDEX,
				  "probe index mismatch");

	g_lpf_dummy_selftest_state.probe_count++;
	return OSAL_SUCCESS;
}

static void pdm_dummy_selftest_remove(const pdm_device_t *device)
{
	if (!device || device->config.type != PDM_DEVICE_TYPE_DUMMY ||
	    device->config.index != PDM_DUMMY_SELFTEST_INDEX)
		return;

	g_lpf_dummy_selftest_state.remove_count++;
}

static const pdm_driver_t g_lpf_dummy_selftest_driver = {
	.name = PDM_DUMMY_SELFTEST_DRIVER,
	.type = PDM_DEVICE_TYPE_DUMMY,
	.capabilities = PDM_DUMMY_SELFTEST_CAP,
	.init = pdm_dummy_selftest_driver_init,
	.exit = pdm_dummy_selftest_driver_exit,
	.probe = pdm_dummy_selftest_probe,
	.remove = pdm_dummy_selftest_remove,
};

static const pdm_device_config_t g_lpf_dummy_selftest_device = {
	.type = PDM_DEVICE_TYPE_DUMMY,
	.index = PDM_DUMMY_SELFTEST_INDEX,
	.name = PDM_DUMMY_SELFTEST_NAME,
	.capabilities = PDM_DEVICE_CAP_DEBUGFS,
};

static int32_t pdm_dummy_selftest_expect_info(const pdm_device_info_t *info,
					      pdm_device_state_t state,
					      int32_t last_error,
					      uint32_t error_count)
{
	PDM_DUMMY_SELFTEST_EXPECT(pdm_dummy_selftest_is_target(info),
				  "device info target mismatch");
	PDM_DUMMY_SELFTEST_EXPECT(info->state == state,
				  "device state mismatch");
	PDM_DUMMY_SELFTEST_EXPECT(info->last_error == last_error,
				  "device last_error mismatch");
	PDM_DUMMY_SELFTEST_EXPECT(info->error_count == error_count,
				  "device error_count mismatch");
	PDM_DUMMY_SELFTEST_EXPECT(
		(info->capabilities & PDM_DUMMY_SELFTEST_CAP) != 0,
		"driver capability missing");
	PDM_DUMMY_SELFTEST_EXPECT(
		(info->capabilities & PDM_DEVICE_CAP_DEBUGFS) != 0,
		"device capability missing");
	PDM_DUMMY_SELFTEST_EXPECT(osal_strcmp(info->name,
					      PDM_DUMMY_SELFTEST_NAME) == 0,
				  "device name mismatch");
	PDM_DUMMY_SELFTEST_EXPECT(osal_strcmp(info->driver_name,
					      PDM_DUMMY_SELFTEST_DRIVER) == 0,
				  "driver name mismatch");
	return OSAL_SUCCESS;
}

static int32_t pdm_dummy_selftest_run(void)
{
	pdm_device_handle_t *handle = NULL;
	pdm_device_info_t info;
	uint32_t count = 0;
	int32_t ret;

	osal_memset(&g_lpf_dummy_selftest_state, 0,
		    sizeof(g_lpf_dummy_selftest_state));

	ret = pdm_device_event_subscribe(pdm_dummy_selftest_event,
					 &g_lpf_dummy_selftest_state);
	if (ret != OSAL_SUCCESS)
		return ret;

	ret = pdm_driver_register(&g_lpf_dummy_selftest_driver);
	if (ret != OSAL_SUCCESS)
		goto out_unsubscribe;

	ret = pdm_driver_register(&g_lpf_dummy_selftest_driver);
	if (ret != OSAL_ERR_ALREADY_EXISTS) {
		ret = pdm_dummy_selftest_fail("duplicate driver not rejected");
		goto out_unregister;
	}

	ret = pdm_device_register(&g_lpf_dummy_selftest_device);
	if (ret != OSAL_SUCCESS)
		goto out_unregister;

	ret = pdm_device_register(&g_lpf_dummy_selftest_device);
	if (ret != OSAL_ERR_ALREADY_EXISTS) {
		ret = pdm_dummy_selftest_fail("duplicate device not rejected");
		goto out_unregister;
	}

	PDM_DUMMY_SELFTEST_EXPECT(g_lpf_dummy_selftest_state.init_count == 1U,
				  "driver init count mismatch");
	PDM_DUMMY_SELFTEST_EXPECT(g_lpf_dummy_selftest_state.probe_count == 1U,
				  "driver probe count mismatch");

	ret = pdm_device_get_info(PDM_DEVICE_TYPE_DUMMY,
				  PDM_DUMMY_SELFTEST_INDEX, &info);
	if (ret != OSAL_SUCCESS)
		goto out_unregister;
	ret = pdm_dummy_selftest_expect_info(&info, PDM_DEVICE_STATE_BOUND,
					     OSAL_SUCCESS, 0U);
	if (ret != OSAL_SUCCESS)
		goto out_unregister;

	ret = pdm_device_get_info_by_name(PDM_DUMMY_SELFTEST_NAME, &info);
	if (ret != OSAL_SUCCESS)
		goto out_unregister;
	ret = pdm_dummy_selftest_expect_info(&info, PDM_DEVICE_STATE_BOUND,
					     OSAL_SUCCESS, 0U);
	if (ret != OSAL_SUCCESS)
		goto out_unregister;

	handle = pdm_device_get_by_capability(PDM_DUMMY_SELFTEST_CAP, 0U);
	if (!handle) {
		ret = pdm_dummy_selftest_fail("capability handle lookup failed");
		goto out_unregister;
	}
	ret = pdm_device_handle_get_info(handle, &info);
	pdm_device_put(handle);
	handle = NULL;
	if (ret != OSAL_SUCCESS)
		goto out_unregister;
	ret = pdm_dummy_selftest_expect_info(&info, PDM_DEVICE_STATE_BOUND,
					     OSAL_SUCCESS, 0U);
	if (ret != OSAL_SUCCESS)
		goto out_unregister;

	ret = pdm_device_list(NULL, &count);
	if (ret != OSAL_SUCCESS)
		goto out_unregister;
	PDM_DUMMY_SELFTEST_EXPECT(count > 0U, "device list count is zero");

	pdm_device_record_error(PDM_DEVICE_TYPE_DUMMY,
				PDM_DUMMY_SELFTEST_INDEX, OSAL_ERR_BUSY);
	ret = pdm_device_get_info(PDM_DEVICE_TYPE_DUMMY,
				  PDM_DUMMY_SELFTEST_INDEX, &info);
	if (ret != OSAL_SUCCESS)
		goto out_unregister;
	ret = pdm_dummy_selftest_expect_info(&info, PDM_DEVICE_STATE_ERROR,
					     OSAL_ERR_BUSY, 1U);
	if (ret != OSAL_SUCCESS)
		goto out_unregister;

	ret = pdm_device_record_recovery(PDM_DEVICE_TYPE_DUMMY,
					 PDM_DUMMY_SELFTEST_INDEX);
	if (ret != OSAL_SUCCESS)
		goto out_unregister;
	ret = pdm_device_get_info(PDM_DEVICE_TYPE_DUMMY,
				  PDM_DUMMY_SELFTEST_INDEX, &info);
	if (ret != OSAL_SUCCESS)
		goto out_unregister;
	ret = pdm_dummy_selftest_expect_info(&info, PDM_DEVICE_STATE_BOUND,
					     OSAL_ERR_BUSY, 1U);
	if (ret != OSAL_SUCCESS)
		goto out_unregister;

	pdm_driver_unregister(&g_lpf_dummy_selftest_driver);

	PDM_DUMMY_SELFTEST_EXPECT(g_lpf_dummy_selftest_state.remove_count == 1U,
				  "driver remove count mismatch");
	PDM_DUMMY_SELFTEST_EXPECT(g_lpf_dummy_selftest_state.exit_count == 1U,
				  "driver exit count mismatch");
	PDM_DUMMY_SELFTEST_EXPECT(
		g_lpf_dummy_selftest_state.events[PDM_DEVICE_EVENT_REGISTERED] ==
			1U,
		"registered event count mismatch");
	PDM_DUMMY_SELFTEST_EXPECT(
		g_lpf_dummy_selftest_state.events[PDM_DEVICE_EVENT_BOUND] == 1U,
		"bound event count mismatch");
	PDM_DUMMY_SELFTEST_EXPECT(
		g_lpf_dummy_selftest_state
			.events[PDM_DEVICE_EVENT_STATE_CHANGED] == 2U,
		"state event count mismatch");
	PDM_DUMMY_SELFTEST_EXPECT(
		g_lpf_dummy_selftest_state.events[PDM_DEVICE_EVENT_ERROR] == 1U,
		"error event count mismatch");
	PDM_DUMMY_SELFTEST_EXPECT(
		g_lpf_dummy_selftest_state.events[PDM_DEVICE_EVENT_REMOVING] ==
			1U,
		"removing event count mismatch");
	PDM_DUMMY_SELFTEST_EXPECT(
		g_lpf_dummy_selftest_state.events[PDM_DEVICE_EVENT_REMOVED] ==
			1U,
		"removed event count mismatch");

	ret = pdm_device_get_info_by_name(PDM_DUMMY_SELFTEST_NAME, &info);
	PDM_DUMMY_SELFTEST_EXPECT(ret == OSAL_ERR_NAME_NOT_FOUND,
				  "dummy device still discoverable");

	pdm_device_event_unsubscribe(pdm_dummy_selftest_event,
				     &g_lpf_dummy_selftest_state);
	return OSAL_SUCCESS;

out_unregister:
	pdm_driver_unregister(&g_lpf_dummy_selftest_driver);
out_unsubscribe:
	pdm_device_event_unsubscribe(pdm_dummy_selftest_event,
				     &g_lpf_dummy_selftest_state);
	return ret;
}

static int __init pdm_dummy_selftest_init(void)
{
	int32_t ret;

	ret = pdm_dummy_selftest_run();
	if (ret != OSAL_SUCCESS)
		return (int)pdm_status_to_errno(ret);

	pr_info("PDM:DUMMY_SELFTEST: dummy service lifecycle checks passed\n");
	return 0;
}

static void __exit pdm_dummy_selftest_exit(void)
{
	pr_info("PDM:DUMMY_SELFTEST: unloaded\n");
}

module_init(pdm_dummy_selftest_init);
module_exit(pdm_dummy_selftest_exit);

MODULE_AUTHOR("PDM");
MODULE_DESCRIPTION("PDM dummy peripheral service self-test");
MODULE_LICENSE("GPL");
MODULE_SOFTDEP("pre: pdm_core");
