/************************************************************************
 * HAL层 - GPIO硬件抽象接口 (Linux实现)
 *
 * 使用Linux sysfs GPIO接口实现
 ************************************************************************/

#include "osal.h"
#include "hal.h"
#include "hal_gpio_internal.h"

/*===========================================================================
 * GPIO中断监听线程管理
 *===========================================================================*/

#define MAX_GPIO_PINS 256

typedef struct {
	uint32_t gpio_num;
	int value_fd;
	hal_gpio_isr_callback_t callback;
	void *user_data;
	osal_thread_t thread;
	bool enabled;
	bool running;
} gpio_isr_context_t;

static gpio_isr_context_t gpio_isr_table[MAX_GPIO_PINS];
static osal_mutex_t gpio_isr_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool gpio_module_initialized = false;
static osal_flock_t *g_gpio_flock = NULL;

/*===========================================================================
 * Private static helper functions
 *===========================================================================*/

/**
 * @brief 写入字符串到文件
 */
static int32_t gpio_write_file(const char *path, const char *value)
{
	int32_t fd;
	uint32_t len;
	int32_t written;

	fd = osal_open(path, OSAL_O_WRONLY, 0);
	if (fd < 0) {
		int32_t err = osal_get_errno();
		return err;
	}

	len = (uint32_t)osal_strlen(value);
	written = osal_write(fd, value, len);
	osal_close(fd);

	if (written != (int32_t)len) {
		int32_t err = osal_get_errno();
		return err;
	}

	return OSAL_SUCCESS;
}

/**
 * @brief 从文件读取字符串
 */
static int32_t gpio_read_file(const char *path, char *buffer, size_t size)
{
	int32_t fd;
	int32_t len;

	fd = osal_open(path, OSAL_O_RDONLY, 0);
	if (fd < 0) {
		int32_t err = osal_get_errno();
		return err;
	}

	len = osal_read(fd, buffer, (uint32_t)(size - 1));
	osal_close(fd);

	if (len < 0) {
		int32_t err = osal_get_errno();
		return err;
	}

	buffer[len] = '\0';
	return OSAL_SUCCESS;
}

/**
 * @brief 导出GPIO引脚
 */
static int32_t gpio_export(uint32_t gpio_num)
{
	char gpio_str[16];
	char path[256];

	/* 检查GPIO是否已导出 */
	osal_snprintf(path, OSAL_sizeof(path), "/sys/class/gpio/gpio%u", gpio_num);
	if (osal_access(path, OSAL_F_OK) == 0) {
		return OSAL_SUCCESS; /* 已导出 */
	}

	/* 导出GPIO */
	osal_snprintf(gpio_str, OSAL_sizeof(gpio_str), "%u", gpio_num);
	return gpio_write_file("/sys/class/gpio/export", gpio_str);
}

/**
 * @brief 取消导出GPIO引脚
 */
static int32_t gpio_unexport(uint32_t gpio_num)
{
	char gpio_str[16];
	osal_snprintf(gpio_str, OSAL_sizeof(gpio_str), "%u", gpio_num);
	return gpio_write_file("/sys/class/gpio/unexport", gpio_str);
}

/**
 * @brief GPIO中断监听线程
 */
static void *gpio_isr_thread(void *arg)
{
	gpio_isr_context_t *ctx = (gpio_isr_context_t *)arg;
	osal_pollfd_t pfd;
	char value_str[4];
	hal_gpio_level_t level;
	int32_t bytes_read;
	int ret;
	int32_t len;

	pfd.fd = ctx->value_fd;
	pfd.events = OSAL_POLLPRI | OSAL_POLLERR;

	/* 清除初始中断 */
	osal_lseek(ctx->value_fd, 0, OSAL_SEEK_SET);
	bytes_read = osal_read(ctx->value_fd, value_str, OSAL_sizeof(value_str));
	(void)bytes_read; /* Suppress unused result warning */

	while (ctx->running) {
		ret = osal_poll(&pfd, 1, 1000); /* 1秒超时 */

		if (ret > 0 && (pfd.revents & OSAL_POLLPRI)) {
			if (!ctx->enabled) {
				/* 清除中断但不调用回调 */
				osal_lseek(ctx->value_fd, 0, OSAL_SEEK_SET);
				bytes_read =
					osal_read(ctx->value_fd, value_str, OSAL_sizeof(value_str));
				(void)bytes_read; /* Suppress unused result warning */
				continue;
			}

			/* 读取当前电平 */
			osal_lseek(ctx->value_fd, 0, OSAL_SEEK_SET);
			len = osal_read(ctx->value_fd, value_str, OSAL_sizeof(value_str));
			if (len > 0) {
				value_str[len] = '\0';
				level = (value_str[0] == '1') ? HAL_GPIO_LEVEL_HIGH :
												HAL_GPIO_LEVEL_LOW;

				/* 调用回调函数 */
				if (ctx->callback) {
					ctx->callback(ctx->gpio_num, level, ctx->user_data);
				}
			}
		}
	}

	return NULL;
}

/**
 * @brief GPIO模块初始化
 */
static int32_t gpio_module_init(void)
{
	int32_t ret;
	uint32_t i;

	if (gpio_module_initialized) {
		return OSAL_SUCCESS; /* 已初始化 */
	}

	/* 初始化GPIO中断上下文表 - 修复未初始化问题 */
	for (i = 0; i < MAX_GPIO_PINS; i++) {
		gpio_isr_table[i].gpio_num = 0;
		gpio_isr_table[i].value_fd = -1;
		gpio_isr_table[i].callback = NULL;
		gpio_isr_table[i].user_data = NULL;
		gpio_isr_table[i].enabled = false;
		gpio_isr_table[i].running = false;
	}

	/* 创建文件锁 */
	ret = osal_flock_create(HAL_GPIO_LOCK_PATH, &g_gpio_flock);
	if (ret != OSAL_SUCCESS) {
		return ret;
	}

	gpio_module_initialized = true;
	return OSAL_SUCCESS;
}

/**
 * @brief GPIO模块清理
 */
static void gpio_module_cleanup(void)
{
	if (gpio_module_initialized) {
		osal_pthread_mutex_destroy(&gpio_isr_mutex);
		gpio_module_initialized = false;
	}

	if (g_gpio_flock != NULL) {
		osal_flock_destroy(g_gpio_flock);
		g_gpio_flock = NULL;
	}
}

/*===========================================================================
 * Primary API functions
 *===========================================================================*/

int32_t hal_gpio_set_direction(uint32_t gpio_num,
							   hal_gpio_direction_t direction)
{
	char path[256];
	const char *dir_str;
	int32_t ret;

	if (gpio_num >= MAX_GPIO_PINS) {
		return OSAL_ERR_INVALID_PARAM;
	}

	/* 获取文件锁 */
	ret = osal_flock_lock(g_gpio_flock, OSAL_FLOCK_EXCLUSIVE);
	if (ret != OSAL_SUCCESS) {
		return ret;
	}

	/* 获取互斥锁 */
	osal_pthread_mutex_lock(&gpio_isr_mutex);

	dir_str = (direction == HAL_GPIO_DIR_INPUT) ? "in" : "out";
	osal_snprintf(path, OSAL_sizeof(path), "/sys/class/gpio/gpio%u/direction",
				  gpio_num);

	ret = gpio_write_file(path, dir_str);

	osal_pthread_mutex_unlock(&gpio_isr_mutex);
	osal_flock_unlock(g_gpio_flock);

	return ret;
}

int32_t hal_gpio_get_direction(uint32_t gpio_num,
							   hal_gpio_direction_t *direction)
{
	char path[256];
	char buffer[16];
	int32_t ret;

	if (gpio_num >= MAX_GPIO_PINS || !direction) {
		return OSAL_ERR_INVALID_PARAM;
	}

	/* 获取文件锁 */
	ret = osal_flock_lock(g_gpio_flock, OSAL_FLOCK_EXCLUSIVE);
	if (ret != OSAL_SUCCESS) {
		return ret;
	}

	/* 获取互斥锁 */
	osal_pthread_mutex_lock(&gpio_isr_mutex);

	osal_snprintf(path, OSAL_sizeof(path), "/sys/class/gpio/gpio%u/direction",
				  gpio_num);
	ret = gpio_read_file(path, buffer, OSAL_sizeof(buffer));
	if (ret != OSAL_SUCCESS) {
		osal_pthread_mutex_unlock(&gpio_isr_mutex);
		osal_flock_unlock(g_gpio_flock);
		return ret;
	}

	*direction = (osal_strncmp(buffer, "in", 2) == 0) ? HAL_GPIO_DIR_INPUT :
														HAL_GPIO_DIR_OUTPUT;

	osal_pthread_mutex_unlock(&gpio_isr_mutex);
	osal_flock_unlock(g_gpio_flock);

	return OSAL_SUCCESS;
}

int32_t hal_gpio_set_level(uint32_t gpio_num, hal_gpio_level_t level)
{
	char path[256];
	const char *level_str;
	int32_t ret;

	if (gpio_num >= MAX_GPIO_PINS) {
		return OSAL_ERR_INVALID_PARAM;
	}

	/* 获取文件锁 */
	ret = osal_flock_lock(g_gpio_flock, OSAL_FLOCK_EXCLUSIVE);
	if (ret != OSAL_SUCCESS) {
		return ret;
	}

	/* 获取互斥锁 */
	osal_pthread_mutex_lock(&gpio_isr_mutex);

	level_str = (level == HAL_GPIO_LEVEL_HIGH) ? "1" : "0";
	osal_snprintf(path, OSAL_sizeof(path), "/sys/class/gpio/gpio%u/value",
				  gpio_num);

	ret = gpio_write_file(path, level_str);

	osal_pthread_mutex_unlock(&gpio_isr_mutex);
	osal_flock_unlock(g_gpio_flock);

	return ret;
}

int32_t hal_gpio_get_level(uint32_t gpio_num, hal_gpio_level_t *level)
{
	char path[256];
	char buffer[4];
	int32_t ret;

	if (gpio_num >= MAX_GPIO_PINS || !level) {
		return OSAL_ERR_INVALID_PARAM;
	}

	/* 获取文件锁 */
	ret = osal_flock_lock(g_gpio_flock, OSAL_FLOCK_EXCLUSIVE);
	if (ret != OSAL_SUCCESS) {
		return ret;
	}

	/* 获取互斥锁 */
	osal_pthread_mutex_lock(&gpio_isr_mutex);

	osal_snprintf(path, OSAL_sizeof(path), "/sys/class/gpio/gpio%u/value",
				  gpio_num);
	ret = gpio_read_file(path, buffer, OSAL_sizeof(buffer));
	if (ret != OSAL_SUCCESS) {
		osal_pthread_mutex_unlock(&gpio_isr_mutex);
		osal_flock_unlock(g_gpio_flock);
		return ret;
	}

	*level = (buffer[0] == '1') ? HAL_GPIO_LEVEL_HIGH : HAL_GPIO_LEVEL_LOW;

	osal_pthread_mutex_unlock(&gpio_isr_mutex);
	osal_flock_unlock(g_gpio_flock);

	return OSAL_SUCCESS;
}

int32_t hal_gpio_set_interrupt(uint32_t gpio_num, hal_gpio_edge_t edge,
							   hal_gpio_isr_callback_t callback,
							   void *user_data)
{
	char path[256];
	const char *edge_str;
	int32_t ret;
	int32_t fd;

	if (gpio_num >= MAX_GPIO_PINS || !callback) {
		return OSAL_ERR_INVALID_PARAM;
	}

	/* 获取文件锁 */
	ret = osal_flock_lock(g_gpio_flock, OSAL_FLOCK_EXCLUSIVE);
	if (ret != OSAL_SUCCESS) {
		return ret;
	}

	/* 设置触发模式 */
	switch (edge) {
	case HAL_GPIO_EDGE_NONE:
		edge_str = "none";
		break;
	case HAL_GPIO_EDGE_RISING:
		edge_str = "rising";
		break;
	case HAL_GPIO_EDGE_FALLING:
		edge_str = "falling";
		break;
	case HAL_GPIO_EDGE_BOTH:
		edge_str = "both";
		break;
	default:
		osal_flock_unlock(g_gpio_flock);
		return OSAL_ERR_INVALID_PARAM;
	}

	osal_snprintf(path, OSAL_sizeof(path), "/sys/class/gpio/gpio%u/edge",
				  gpio_num);
	ret = gpio_write_file(path, edge_str);
	if (ret != OSAL_SUCCESS) {
		osal_flock_unlock(g_gpio_flock);
		return ret;
	}

	if (edge == HAL_GPIO_EDGE_NONE) {
		osal_flock_unlock(g_gpio_flock);
		return OSAL_SUCCESS;
	}

	/* 打开value文件用于poll */
	osal_snprintf(path, OSAL_sizeof(path), "/sys/class/gpio/gpio%u/value",
				  gpio_num);
	fd = osal_open(path, OSAL_O_RDONLY, 0);
	if (fd < 0) {
		int32_t err = osal_get_errno();
		osal_flock_unlock(g_gpio_flock);
		return err;
	}

	/* 初始化中断上下文 */
	osal_pthread_mutex_lock(&gpio_isr_mutex);

	/* 如果已有线程在运行，先停止 */
	if (gpio_isr_table[gpio_num].running) {
		gpio_isr_table[gpio_num].running = false;
		osal_pthread_mutex_unlock(&gpio_isr_mutex);
		osal_pthread_join(gpio_isr_table[gpio_num].thread, NULL);
		osal_pthread_mutex_lock(&gpio_isr_mutex);
		if (gpio_isr_table[gpio_num].value_fd >= 0) {
			osal_close(gpio_isr_table[gpio_num].value_fd);
		}
	}

	gpio_isr_table[gpio_num].gpio_num = gpio_num;
	gpio_isr_table[gpio_num].value_fd = fd;
	gpio_isr_table[gpio_num].callback = callback;
	gpio_isr_table[gpio_num].user_data = user_data;
	gpio_isr_table[gpio_num].enabled = true;
	gpio_isr_table[gpio_num].running = true;

	/* 创建监听线程 */
	ret = osal_pthread_create(&gpio_isr_table[gpio_num].thread, NULL,
							  gpio_isr_thread, &gpio_isr_table[gpio_num]);

	osal_pthread_mutex_unlock(&gpio_isr_mutex);
	osal_flock_unlock(g_gpio_flock);

	if (ret != OSAL_SUCCESS) {
		osal_close(fd);
		osal_memset(&gpio_isr_table[gpio_num], 0,
					OSAL_sizeof(gpio_isr_context_t));
		return ret;
	}

	return OSAL_SUCCESS;
}

int32_t hal_gpio_enable_interrupt(uint32_t gpio_num)
{
	int32_t ret;

	if (gpio_num >= MAX_GPIO_PINS) {
		return OSAL_ERR_INVALID_PARAM;
	}

	/* 获取文件锁 */
	ret = osal_flock_lock(g_gpio_flock, OSAL_FLOCK_EXCLUSIVE);
	if (ret != OSAL_SUCCESS) {
		return ret;
	}

	osal_pthread_mutex_lock(&gpio_isr_mutex);
	if (!gpio_isr_table[gpio_num].running) {
		osal_pthread_mutex_unlock(&gpio_isr_mutex);
		osal_flock_unlock(g_gpio_flock);
		return OSAL_ERR_INVALID_PARAM;
	}
	gpio_isr_table[gpio_num].enabled = true;
	osal_pthread_mutex_unlock(&gpio_isr_mutex);

	osal_flock_unlock(g_gpio_flock);

	return OSAL_SUCCESS;
}

int32_t hal_gpio_disable_interrupt(uint32_t gpio_num)
{
	int32_t ret;

	if (gpio_num >= MAX_GPIO_PINS) {
		return OSAL_ERR_INVALID_PARAM;
	}

	/* 获取文件锁 */
	ret = osal_flock_lock(g_gpio_flock, OSAL_FLOCK_EXCLUSIVE);
	if (ret != OSAL_SUCCESS) {
		return ret;
	}

	osal_pthread_mutex_lock(&gpio_isr_mutex);
	if (!gpio_isr_table[gpio_num].running) {
		osal_pthread_mutex_unlock(&gpio_isr_mutex);
		osal_flock_unlock(g_gpio_flock);
		return OSAL_ERR_INVALID_PARAM;
	}
	gpio_isr_table[gpio_num].enabled = false;
	osal_pthread_mutex_unlock(&gpio_isr_mutex);

	osal_flock_unlock(g_gpio_flock);

	return OSAL_SUCCESS;
}

/*===========================================================================
 * Initialization and cleanup functions
 *===========================================================================*/

int32_t hal_gpio_init(uint32_t gpio_num, const hal_gpio_config_t *config)
{
	int32_t ret;

	if (!config || gpio_num >= MAX_GPIO_PINS) {
		return OSAL_ERR_INVALID_PARAM;
	}

	/* 初始化GPIO模块 */
	ret = gpio_module_init();
	if (ret != OSAL_SUCCESS) {
		return ret;
	}

	/* 获取文件锁 */
	ret = osal_flock_lock(g_gpio_flock, OSAL_FLOCK_EXCLUSIVE);
	if (ret != OSAL_SUCCESS) {
		return ret;
	}

	/* 获取互斥锁 */
	osal_pthread_mutex_lock(&gpio_isr_mutex);

	/* 导出GPIO */
	ret = gpio_export(gpio_num);
	if (ret != OSAL_SUCCESS) {
		osal_pthread_mutex_unlock(&gpio_isr_mutex);
		osal_flock_unlock(g_gpio_flock);
		return ret;
	}

	/* 等待sysfs文件创建 */
	osal_msleep(100); /* 100ms */

	/* 设置方向 */
	ret = hal_gpio_set_direction(gpio_num, config->direction);
	if (ret != OSAL_SUCCESS) {
		gpio_unexport(gpio_num);
		osal_pthread_mutex_unlock(&gpio_isr_mutex);
		osal_flock_unlock(g_gpio_flock);
		return ret;
	}

	/* 如果是输出模式，设置初始电平 */
	if (config->direction == HAL_GPIO_DIR_OUTPUT) {
		ret = hal_gpio_set_level(gpio_num, config->initial_level);
		if (ret != OSAL_SUCCESS) {
			gpio_unexport(gpio_num);
			osal_pthread_mutex_unlock(&gpio_isr_mutex);
			osal_flock_unlock(g_gpio_flock);
			return ret;
		}
	}

	/* 配置中断 */
	if (config->edge != HAL_GPIO_EDGE_NONE && config->callback) {
		ret = hal_gpio_set_interrupt(gpio_num, config->edge, config->callback,
									 config->user_data);
		if (ret != OSAL_SUCCESS) {
			gpio_unexport(gpio_num);
			osal_pthread_mutex_unlock(&gpio_isr_mutex);
			osal_flock_unlock(g_gpio_flock);
			return ret;
		}
	}

	osal_pthread_mutex_unlock(&gpio_isr_mutex);
	osal_flock_unlock(g_gpio_flock);

	return OSAL_SUCCESS;
}

int32_t hal_gpio_deinit(uint32_t gpio_num)
{
	int32_t ret;

	if (gpio_num >= MAX_GPIO_PINS) {
		return OSAL_ERR_INVALID_PARAM;
	}

	/* 获取文件锁 */
	ret = osal_flock_lock(g_gpio_flock, OSAL_FLOCK_EXCLUSIVE);
	if (ret != OSAL_SUCCESS) {
		return ret;
	}

	/* 停止中断监听线程 */
	osal_pthread_mutex_lock(&gpio_isr_mutex);
	if (gpio_isr_table[gpio_num].running) {
		gpio_isr_table[gpio_num].running = false;
		osal_pthread_mutex_unlock(&gpio_isr_mutex);

		osal_pthread_join(gpio_isr_table[gpio_num].thread, NULL);

		osal_pthread_mutex_lock(&gpio_isr_mutex);
		if (gpio_isr_table[gpio_num].value_fd >= 0) {
			osal_close(gpio_isr_table[gpio_num].value_fd);
			gpio_isr_table[gpio_num].value_fd = -1;
		}
		osal_memset(&gpio_isr_table[gpio_num], 0,
					OSAL_sizeof(gpio_isr_context_t));
	}
	osal_pthread_mutex_unlock(&gpio_isr_mutex);

	/* 清理GPIO模块 */
	gpio_module_cleanup();

	/* 取消导出GPIO */
	ret = gpio_unexport(gpio_num);

	osal_flock_unlock(g_gpio_flock);

	return ret;
}
