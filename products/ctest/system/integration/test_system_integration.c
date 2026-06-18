/**
 * @file test_system_integration.c
 * @brief Full stack integration tests for the embedded middleware framework
 */

#include <test_framework/test_framework.h>
#include <test_framework/test_system.h>
#include "osal.h"
#include "hal.h"
#include "pdl.h"
#include "prl.h"
#include "pconfig.h"
#include "aconfig.h"

#define TEST_PTY_NAME_MAX         256
#define TEST_UART_BAUDRATE        115200
#define TEST_UART_DATA_BITS       8
#define TEST_UART_STOP_BITS       1
#define TEST_UART_TIMEOUT_MS      1000
#define TEST_RESPONSE_TIMEOUT_MS  2000

#define TEST_PCONFIG_PLATFORM     "generic"
#define TEST_PCONFIG_CHIP         "framework"
#define TEST_PCONFIG_PROJECT      "ctest"
#define TEST_PCONFIG_PRODUCT      "middleware"
#define TEST_PCONFIG_MCU_NAME     "mcu0"
#define TEST_PCONFIG_SERIAL_LINK   "/tmp/es_middleware_ctest_mcu0"

#define TEST_RESPONSE_TAG         0x55
#define TEST_RESPONSE_PREFIX      0xAA

static int32_t g_pty_master_fd = -1;
static int32_t g_pty_slave_fd = -1;
static char g_pty_slave_name[TEST_PTY_NAME_MAX];
static osal_thread_t g_responder_thread;
static bool g_responder_running = false;
static bool g_responder_ready = false;
static osal_mutex_t g_responder_mutex;
static osal_cond_t g_responder_cond;

static const uint8_t g_test_payload[] = {0x10, 0x20, 0x30, 0x40};

static void set_raw_mode(int32_t fd)
{
	osal_termios_t term;

	if (OSAL_tcgetattr(fd, &term) != 0) {
		return;
	}

	term.c_iflag = 0;
	term.c_oflag = 0;
	term.c_lflag = 0;
	term.c_cflag &= ~(OSAL_PARENB | OSAL_CSTOPB | OSAL_CSIZE);
	term.c_cflag |= (OSAL_CREAD | OSAL_CLOCAL | OSAL_CS8);
	term.c_ispeed = OSAL_B115200;
	term.c_ospeed = OSAL_B115200;
	(void)OSAL_tcsetattr(fd, OSAL_TCSANOW, &term);
}

static int32_t create_pty_pair(void)
{
	int32_t ret;

	ret = OSAL_openpty(&g_pty_master_fd, &g_pty_slave_fd, g_pty_slave_name, NULL, NULL);
	if (ret != 0) {
		return ret;
	}

	set_raw_mode(g_pty_master_fd);
	set_raw_mode(g_pty_slave_fd);
	return OSAL_SUCCESS;
}

static void destroy_pty_pair(void)
{
	if (g_pty_master_fd >= 0) {
		OSAL_close(g_pty_master_fd);
		g_pty_master_fd = -1;
	}
	if (g_pty_slave_fd >= 0) {
		OSAL_close(g_pty_slave_fd);
		g_pty_slave_fd = -1;
	}
}

static int32_t bind_test_serial_device(void)
{
	(void)OSAL_unlink(TEST_PCONFIG_SERIAL_LINK);

	if (0 != OSAL_symlink(g_pty_slave_name, TEST_PCONFIG_SERIAL_LINK)) {
		return OSAL_ERR_GENERIC;
	}

	return OSAL_SUCCESS;
}

static void* pty_responder_thread(void *arg)
{
	uint8_t rx_buffer[512];
	uint8_t tx_buffer[512];
	(void)arg;

	OSAL_pthread_mutex_lock(&g_responder_mutex);
	g_responder_ready = true;
	OSAL_pthread_cond_signal(&g_responder_cond);
	OSAL_pthread_mutex_unlock(&g_responder_mutex);

	while (g_responder_running) {
		osal_pollfd_t pfd;
		pfd.fd = g_pty_master_fd;
		pfd.events = OSAL_POLLIN;
		pfd.revents = 0;

		int32_t pret = OSAL_poll(&pfd, 1, 100);
		if (!g_responder_running) {
			break;
		}
		if (pret <= 0 || !(pfd.revents & OSAL_POLLIN)) {
			continue;
		}

		osal_ssize_t rx_len = OSAL_read(g_pty_master_fd, rx_buffer, OSAL_sizeof(rx_buffer));
		if (rx_len <= 0) {
			continue;
		}

		prl_decode_ctx_t decode_ctx;
		OSAL_memset(&decode_ctx, 0, sizeof(decode_ctx));
		decode_ctx.buffer = rx_buffer;
		decode_ctx.buffer_len = (size_t)rx_len;
		decode_ctx.payload = tx_buffer;
		decode_ctx.payload_size = OSAL_sizeof(tx_buffer);
		if (prl_device_decode(&decode_ctx) != OSAL_SUCCESS) {
			continue;
		}

		uint8_t payload[16];
		uint32_t payload_len = 0;
		OSAL_memset(payload, 0, OSAL_sizeof(payload));

		switch (decode_ctx.msg_type) {
		case PRL_MCU_MSG_GET_VERSION:
			payload[0] = 1;
			payload[1] = 2;
			payload[2] = 3;
			payload_len = 3;
			break;
		case PRL_MCU_MSG_GET_STATUS:
			payload[0] = PDL_MCU_STATE_READY;
			payload[1] = 0x00;
			payload[2] = 0x00;
			payload[3] = 0x00;
			payload[4] = 0x00;
			payload_len = 5;
			break;
		case PRL_MCU_MSG_RESET:
			payload[0] = TEST_RESPONSE_PREFIX;
			payload_len = 1;
			break;
		case PRL_MCU_MSG_EXECUTE_CMD:
			payload[0] = TEST_RESPONSE_PREFIX;
			payload[1] = (decode_ctx.payload_len > 0 && decode_ctx.payload) ? ((const uint8_t *)decode_ctx.payload)[0] : 0x00;
			payload_len = 2;
			break;
		case PRL_MCU_MSG_WRITE_DATA:
			payload[0] = 0xC1;
			payload[1] = 0x01;
			payload_len = 2;
			break;
		case PRL_MCU_MSG_READ_DATA:
			payload[0] = 0xDE;
			payload[1] = 0xAD;
			payload[2] = 0xBE;
			payload[3] = 0xEF;
			payload_len = 4;
			break;
		default:
			payload[0] = 0x00;
			payload_len = 1;
			break;
		}

		prl_encode_ctx_t encode_ctx;
		OSAL_memset(&encode_ctx, 0, sizeof(encode_ctx));
		encode_ctx.dev_type = PRL_DEV_TYPE_MCU;
		encode_ctx.msg_type = TEST_RESPONSE_TAG;
		encode_ctx.payload = payload;
		encode_ctx.payload_len = (uint16_t)payload_len;
		encode_ctx.buffer = tx_buffer;
		encode_ctx.buffer_size = OSAL_sizeof(tx_buffer);

		int encoded_len = prl_device_encode(&encode_ctx);
		if (encoded_len > 0) {
			(void)OSAL_write(g_pty_master_fd, tx_buffer, (osal_size_t)encoded_len);
		}
	}

	return NULL;
}

static void setup_full_stack(void)
{
	int32_t ret;

	OSAL_memset(&g_responder_mutex, 0, sizeof(g_responder_mutex));
	OSAL_memset(&g_responder_cond, 0, sizeof(g_responder_cond));

	ret = OSAL_pthread_mutex_init(&g_responder_mutex, NULL);
	if (ret != OSAL_SUCCESS) {
		return;
	}
	ret = OSAL_pthread_cond_init(&g_responder_cond, NULL);
	if (ret != OSAL_SUCCESS) {
		OSAL_pthread_mutex_destroy(&g_responder_mutex);
		return;
	}

	ret = create_pty_pair();
	if (ret != OSAL_SUCCESS) {
		OSAL_pthread_cond_destroy(&g_responder_cond);
		OSAL_pthread_mutex_destroy(&g_responder_mutex);
		return;
	}

	ret = bind_test_serial_device();
	if (ret != OSAL_SUCCESS) {
		destroy_pty_pair();
		OSAL_pthread_cond_destroy(&g_responder_cond);
		OSAL_pthread_mutex_destroy(&g_responder_mutex);
		return;
	}

	ret = PRL_init();
	if (ret != OSAL_SUCCESS) {
		OSAL_unlink(TEST_PCONFIG_SERIAL_LINK);
		destroy_pty_pair();
		OSAL_pthread_cond_destroy(&g_responder_cond);
		OSAL_pthread_mutex_destroy(&g_responder_mutex);
		return;
	}

	g_responder_running = true;
	ret = OSAL_pthread_create(&g_responder_thread, NULL, pty_responder_thread, NULL);
	if (ret != OSAL_SUCCESS) {
		g_responder_running = false;
		PRL_deinit();
		OSAL_unlink(TEST_PCONFIG_SERIAL_LINK);
		destroy_pty_pair();
		OSAL_pthread_cond_destroy(&g_responder_cond);
		OSAL_pthread_mutex_destroy(&g_responder_mutex);
		return;
	}

	OSAL_pthread_mutex_lock(&g_responder_mutex);
	while (!g_responder_ready) {
		OSAL_pthread_cond_wait(&g_responder_cond, &g_responder_mutex);
	}
	OSAL_pthread_mutex_unlock(&g_responder_mutex);
}

static void teardown_full_stack(void)
{
	g_responder_running = false;
	if (g_pty_master_fd >= 0) {
		(void)OSAL_write(g_pty_master_fd, "\0", 1);
	}
	(void)OSAL_pthread_join(g_responder_thread, NULL);

	PRL_deinit();
	OSAL_unlink(TEST_PCONFIG_SERIAL_LINK);
	destroy_pty_pair();
	OSAL_pthread_cond_destroy(&g_responder_cond);
	OSAL_pthread_mutex_destroy(&g_responder_mutex);
}

static void test_full_stack_configuration_path(void)
{
	const pconfig_platform_config_t *board = PCONFIG_GetBoard();
	const aconfig_config_table_t *table = ACONFIG_GetTable();
	pdl_mcu_handle_t handle = NULL;
	pdl_mcu_version_t version;
	pdl_mcu_status_t status;
	uint8_t response[32];
	uint32_t response_len = 0;
	int32_t ret;

	TEST_ASSERT_NOT_NULL(table);
	TEST_ASSERT_NOT_NULL(board);
	TEST_ASSERT_EQUAL_STRING(TEST_PCONFIG_PLATFORM, board->platform_name);
	TEST_ASSERT_EQUAL_STRING(TEST_PCONFIG_PRODUCT, board->product_name);
	TEST_ASSERT_EQUAL(1u, board->mcu_count);
	TEST_ASSERT_EQUAL_STRING("ctest_aconfig", table->name);

	ret = PDL_MCU_init(0, &handle);
	TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
	TEST_ASSERT_NOT_NULL(handle);

	ret = PDL_MCU_get_version(handle, &version);
	TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
	TEST_ASSERT_EQUAL(1, version.major);
	TEST_ASSERT_EQUAL(2, version.minor);
	TEST_ASSERT_EQUAL(3, version.patch);

	ret = PDL_MCU_get_status(handle, &status);
	TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
	TEST_ASSERT_TRUE(status.state == PDL_MCU_STATE_READY);
	TEST_ASSERT_TRUE(status.error_code == 0);

	ret = PDL_MCU_send_command(handle, 0x42, g_test_payload, OSAL_sizeof(g_test_payload), response, OSAL_sizeof(response), &response_len);
	TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
	TEST_ASSERT_TRUE(response_len >= 2);
	TEST_ASSERT_EQUAL(TEST_RESPONSE_PREFIX, response[0]);
	TEST_ASSERT_EQUAL(0x42, response[1]);

	ret = PDL_MCU_deinit(handle);
	TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
}

static void test_full_stack_repeated_roundtrip(void)
{
	pdl_mcu_handle_t handle = NULL;
	uint8_t response[32];
	uint32_t response_len = 0;
	int32_t ret;
	uint32_t i;

	ret = PDL_MCU_init(0, &handle);
	TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
	TEST_ASSERT_NOT_NULL(handle);

	for (i = 0; i < 3; i++) {
		ret = PDL_MCU_reset(handle);
		TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

		ret = PDL_MCU_send_command(handle, (uint8_t)(0x21 + i), NULL, 0, response, OSAL_sizeof(response), &response_len);
		TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
		TEST_ASSERT_TRUE(response_len >= 2);
	}

	ret = PDL_MCU_deinit(handle);
	TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
}

static const test_case_t test_cases[] = {
	{
		.name = "test_full_stack_configuration_path",
		.func = test_full_stack_configuration_path,
		.setup = setup_full_stack,
		.teardown = teardown_full_stack
	},
	{
		.name = "test_full_stack_repeated_roundtrip",
		.func = test_full_stack_repeated_roundtrip,
		.setup = setup_full_stack,
		.teardown = teardown_full_stack
	},
};

static const test_suite_t test_suite = {
	.suite_name = "system_integration",
	.module_name = "system_integration",
	.layer_name = "SYSTEM",
	.cases = test_cases,
	.case_count = OSAL_sizeof(test_cases) / OSAL_sizeof(test_case_t),
	.suite_setup = NULL,
	.suite_teardown = NULL,
	.metadata = {
		.category = TEST_CATEGORY_SYSTEM,
		.tags = TEST_TAG_SLOW,
		.timeout_ms = 15000,
		.description = "Full stack integration tests for ACONFIG -> PDL -> PCONFIG -> HAL"
	}
};

__attribute__((constructor))
static void register_system_integration_tests(void)
{
	libutest_register_suite(&test_suite);
}
