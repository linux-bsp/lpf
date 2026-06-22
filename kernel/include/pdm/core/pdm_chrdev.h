// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_CHRDEV_H
#define PDM_CHRDEV_H

#include <linux/fs.h>
#include <linux/miscdevice.h>

#include "osal.h"
#include "pdm/core/pdm_device.h"

#define PDM_CHRDEV_NAME_LEN 64U
#define PDM_CHRDEV_SOC_NAME_LEN 64U

typedef struct {
	char name[PDM_CHRDEV_NAME_LEN];
	char nodename[PDM_CHRDEV_NAME_LEN];
	pdm_device_info_t info;
	char soc_name[PDM_CHRDEV_SOC_NAME_LEN];
	const struct file_operations *fops;
	osal_mutex_t lock;
	osal_atomic_uint32_t open_count;
	osal_atomic_uint32_t error_count;
	struct miscdevice miscdev;
	uint32_t index;
	bool registered;
} pdm_chrdev_t;

/*
 * 注意: pdm_chrdev.c 已删除（旧伪总线依赖）
 * 以下提供最小化 stub 实现以保持兼容性
 */

static inline int pdm_chrdev_register_lpf_device(pdm_chrdev_t *chrdev,
						 const char *name,
						 const char *nodename,
						 const pdm_device_t *device,
						 const struct file_operations *fops)
{
	/* TODO: 实现直接使用 misc_register 的版本 */
	return 0;
}

static inline void pdm_chrdev_unregister(pdm_chrdev_t *chrdev)
{
	/* TODO: 实现直接使用 misc_deregister 的版本 */
}

static inline uint32_t pdm_chrdev_open_count(const pdm_chrdev_t *chrdev)
{
	return chrdev ? osal_atomic_load(&chrdev->open_count) : 0;
}

static inline int pdm_chrdev_get_info(pdm_chrdev_t *chrdev, pdm_device_info_t *info)
{
	if (!chrdev || !info)
		return -EINVAL;
	*info = chrdev->info;
	return 0;
}

static inline uint32_t pdm_chrdev_error_count(const pdm_chrdev_t *chrdev)
{
	return chrdev ? osal_atomic_load(&chrdev->error_count) : 0;
}

static inline void pdm_chrdev_record_error(pdm_chrdev_t *chrdev, int error)
{
	if (chrdev && error)
		osal_atomic_inc(&chrdev->error_count);
}

static inline void pdm_chrdev_record_recovery(pdm_chrdev_t *chrdev)
{
	/* 可选：清除错误计数 */
}

static inline uint32_t pdm_chrdev_index(const pdm_chrdev_t *chrdev)
{
	return chrdev ? chrdev->index : 0;
}

#endif /* PDM_CHRDEV_H */
