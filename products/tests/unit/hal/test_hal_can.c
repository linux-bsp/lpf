#include "test_framework.h"
/**
 * @file test_hal_can.c
 * @brief HAL CAN驱动单元测试
 */

#include "hal.h"
#include "osal.h"

/*===========================================================================
 * 初始化和清理测试
 *===========================================================================*/

/* 测试用例: CAN初始化 - 成功 */
static void test_hal_can_init_success(void)
{
    hal_can_handle_t handle = NULL;
    hal_can_config_t config = {
        .interface = "can0",
        .baudrate = 500000,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    int32_t ret = HAL_CAN_Init(&config, &handle);

    /* 如果初始化失败，跳过所有CAN测试 */
    TEST_ASSERT_FALSE(ret != OSAL_SUCCESS); // CAN interface not available or down. Please run: ip link set can0 up

    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
    TEST_ASSERT_NOT_NULL(handle);

    /* 尝试发送一个测试帧，检查接口是否真正可用 */
    hal_can_frame_t test_frame = {
        .can_id = 0x001,
        .dlc = 1,
        .data = {0x00}
    };
    ret = HAL_CAN_Send(handle, &test_frame);

    /* 如果发送失败（Network is down），跳过所有CAN测试 */
    if (ret != OSAL_SUCCESS) {
        HAL_CAN_Deinit(handle);
        TEST_ASSERT_FALSE(true); // CAN interface is down. Please run: ip link set can0 type can bitrate 500000 && ip link set can0 up
    }

    HAL_CAN_Deinit(handle);
}

/* 测试用例: CAN初始化 - 空配置 */
static void test_hal_can_init_null_config(void)
{
    hal_can_handle_t handle = NULL;

    int32_t ret = HAL_CAN_Init(NULL, &handle);
    TEST_ASSERT_NOT_EQUAL(OSAL_SUCCESS, ret);
}

/* 测试用例: CAN初始化 - 空句柄 */
static void test_hal_can_init_null_handle(void)
{
    hal_can_config_t config = {
        .interface = "can0",
        .baudrate = 500000,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    int32_t ret = HAL_CAN_Init(&config, NULL);
    TEST_ASSERT_NOT_EQUAL(OSAL_SUCCESS, ret);
}

/* 测试用例: CAN初始化 - 无效接口 */
static void test_hal_can_init_invalid_interface(void)
{
    hal_can_handle_t handle = NULL;
    hal_can_config_t config = {
        .interface = "invalid_can999",
        .baudrate = 500000,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    int32_t ret = HAL_CAN_Init(&config, &handle);
    TEST_ASSERT_NOT_EQUAL(OSAL_SUCCESS, ret);
}

/* 测试用例: CAN清理 */
static void test_hal_can_deinit(void)
{
    hal_can_handle_t handle = NULL;
    hal_can_config_t config = {
        .interface = "can0",
        .baudrate = 500000,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    int32_t ret = HAL_CAN_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    ret = HAL_CAN_Deinit(handle);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
}

/* 测试用例: CAN清理 - 空句柄 */
static void test_hal_can_deinit_null_handle(void)
{
    int32_t ret = HAL_CAN_Deinit(NULL);
    TEST_ASSERT_NOT_EQUAL(OSAL_SUCCESS, ret);
}

/*===========================================================================
 * 发送和接收测试
 *===========================================================================*/

/* 测试用例: CAN发送 - 成功 */
static void test_hal_can_send_success(void)
{
    hal_can_handle_t handle = NULL;
    hal_can_config_t config = {
        .interface = "can0",
        .baudrate = 500000,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };
    hal_can_frame_t frame = {
        .can_id = 0x123,
        .dlc = 8,
        .data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}
    };

    int32_t ret = HAL_CAN_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    ret = HAL_CAN_Send(handle, &frame);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    HAL_CAN_Deinit(handle);
}

/* 测试用例: CAN发送 - 空句柄 */
static void test_hal_can_send_null_handle(void)
{
    hal_can_frame_t frame = {
        .can_id = 0x123,
        .dlc = 8,
        .data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}
    };

    int32_t ret = HAL_CAN_Send(NULL, &frame);
    TEST_ASSERT_NOT_EQUAL(OSAL_SUCCESS, ret);
}

/* 测试用例: CAN发送 - 空帧 */
static void test_hal_can_send_null_frame(void)
{
    hal_can_handle_t handle = NULL;
    hal_can_config_t config = {
        .interface = "can0",
        .baudrate = 500000,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    int32_t ret = HAL_CAN_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    ret = HAL_CAN_Send(handle, NULL);
    TEST_ASSERT_NOT_EQUAL(OSAL_SUCCESS, ret);

    HAL_CAN_Deinit(handle);
}

/* 测试用例: CAN接收 - 超时 */
static void test_hal_can_recv_timeout(void)
{
    hal_can_handle_t handle = NULL;
    hal_can_config_t config = {
        .interface = "can0",
        .baudrate = 500000,
        .rx_timeout = 100,
        .tx_timeout = 1000
    };
    hal_can_frame_t frame;

    int32_t ret = HAL_CAN_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    ret = HAL_CAN_Recv(handle, &frame, 100);
    TEST_ASSERT_EQUAL(OSAL_ERR_TIMEOUT, ret);

    HAL_CAN_Deinit(handle);
}

/* 测试用例: CAN接收 - 空句柄 */
static void test_hal_can_recv_null_handle(void)
{
    hal_can_frame_t frame;

    int32_t ret = HAL_CAN_Recv(NULL, &frame, 100);
    TEST_ASSERT_NOT_EQUAL(OSAL_SUCCESS, ret);
}

/* 测试用例: CAN接收 - 空帧 */
static void test_hal_can_recv_null_frame(void)
{
    hal_can_handle_t handle = NULL;
    hal_can_config_t config = {
        .interface = "can0",
        .baudrate = 500000,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    int32_t ret = HAL_CAN_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    ret = HAL_CAN_Recv(handle, NULL, 100);
    TEST_ASSERT_NOT_EQUAL(OSAL_SUCCESS, ret);

    HAL_CAN_Deinit(handle);
}

/* 测试用例: CAN发送接收回环 */
static void test_hal_can_loopback(void)
{
    hal_can_handle_t handle = NULL;
    hal_can_config_t config = {
        .interface = "can0",
        .baudrate = 500000,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };
    hal_can_frame_t tx_frame = {
        .can_id = 0x456,
        .dlc = 4,
        .data = {0xAA, 0xBB, 0xCC, 0xDD}
    };
    hal_can_frame_t rx_frame;

    int32_t ret = HAL_CAN_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    /* 发送帧 */
    ret = HAL_CAN_Send(handle, &tx_frame);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    /* 接收帧（需要外部回环或另一个CAN节点） */
    ret = HAL_CAN_Recv(handle, &rx_frame, 1000);
    if (OSAL_SUCCESS == ret) {
        TEST_ASSERT_EQUAL(tx_frame.can_id, rx_frame.can_id);
        TEST_ASSERT_EQUAL(tx_frame.dlc, rx_frame.dlc);
        int32_t i;

        for (i = 0; i < tx_frame.dlc; i++) {
            TEST_ASSERT_EQUAL(tx_frame.data[i], rx_frame.data[i]);
        }
    }

    HAL_CAN_Deinit(handle);
}

/*===========================================================================
 * 过滤器测试
 *===========================================================================*/

/* 测试用例: 设置过滤器 - 成功 */
static void test_hal_can_set_filter_success(void)
{
    hal_can_handle_t handle = NULL;
    hal_can_config_t config = {
        .interface = "can0",
        .baudrate = 500000,
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    int32_t ret = HAL_CAN_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    ret = HAL_CAN_SetFilter(handle, 0x100, 0x7FF);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    HAL_CAN_Deinit(handle);
}

/* 测试用例: 设置过滤器 - 空句柄 */
static void test_hal_can_set_filter_null_handle(void)
{
    int32_t ret = HAL_CAN_SetFilter(NULL, 0x100, 0x7FF);
    TEST_ASSERT_NOT_EQUAL(OSAL_SUCCESS, ret);
}

/*===========================================================================
 * 统计信息测试
 *===========================================================================*/

/* 测试用例: 获取统计信息 - 成功 */

/*===========================================================================
 * 配置参数测试
 *===========================================================================*/

/* 测试用例: 不同波特率 */
static void test_hal_can_different_baudrate(void)
{
    hal_can_handle_t handle = NULL;
    hal_can_config_t config = {
        .interface = "can0",
        .baudrate = 250000,  /* 250kbps */
        .rx_timeout = 1000,
        .tx_timeout = 1000
    };

    int32_t ret = HAL_CAN_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    HAL_CAN_Deinit(handle);
}

/*===========================================================================
 * 测试模块注册
 *===========================================================================*/

// HAL CAN驱动测试
    /* 初始化和清理 */
    /* 发送和接收 */
    /* 过滤器 */
    /* 统计信息 */
//     //     //     //     /* 配置参数 */

/* 测试用例数组 - 使用函数指针数组 */
static const test_case_t test_cases[] = {
	{
		.name = "test_hal_can_init_success",
		.func = test_hal_can_init_success,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_hal_can_init_null_config",
		.func = test_hal_can_init_null_config,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_hal_can_init_null_handle",
		.func = test_hal_can_init_null_handle,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_hal_can_init_invalid_interface",
		.func = test_hal_can_init_invalid_interface,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_hal_can_deinit",
		.func = test_hal_can_deinit,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_hal_can_deinit_null_handle",
		.func = test_hal_can_deinit_null_handle,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_hal_can_send_success",
		.func = test_hal_can_send_success,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_hal_can_send_null_handle",
		.func = test_hal_can_send_null_handle,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_hal_can_send_null_frame",
		.func = test_hal_can_send_null_frame,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_hal_can_recv_timeout",
		.func = test_hal_can_recv_timeout,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_hal_can_recv_null_handle",
		.func = test_hal_can_recv_null_handle,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_hal_can_recv_null_frame",
		.func = test_hal_can_recv_null_frame,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_hal_can_loopback",
		.func = test_hal_can_loopback,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_hal_can_set_filter_success",
		.func = test_hal_can_set_filter_success,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_hal_can_set_filter_null_handle",
		.func = test_hal_can_set_filter_null_handle,
		.setup = NULL,
		.teardown = NULL
	},
	{
		.name = "test_hal_can_different_baudrate",
		.func = test_hal_can_different_baudrate,
		.setup = NULL,
		.teardown = NULL
	},
};

/* 测试套件定义 */
static const test_suite_t test_suite = {
	.suite_name = "hal_can",
	.module_name = "hal_can",
	.layer_name = "HAL",
	.cases = test_cases,
	.case_count = OSAL_SIZEOF(test_cases) / OSAL_SIZEOF(test_case_t),
	.suite_setup = NULL,
	.suite_teardown = NULL,
	.metadata = {
		.category = TEST_CATEGORY_UNIT,
		.tags = TEST_TAG_FAST,
		.timeout_ms = 100,
		.description = "HAL hal_can tests"
	}
};

/* 测试套件注册函数 */
__attribute__((constructor))
static void register_hal_can_tests(void)
{
	libutest_register_suite(&test_suite);
}
