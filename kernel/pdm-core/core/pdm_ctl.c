// SPDX-License-Identifier: GPL-2.0

#include "pdm_ctl_internal.h"

#include <linux/errno.h>
#include <linux/fs.h>

#include "pdm/core/pdm_chrdev.h"
#include "pdm/core/pdm_core.h"
#include "pdm/pdm_ctl.h"

#define PDM_CTL_VERSION_MAJOR 0x01
#define PDM_CTL_VERSION_MINOR 0x00
#define PDM_CTL_VERSION_PATCH 0x00

static pdm_chrdev_t g_lpf_ctl_chrdev;

static uint32_t pdm_ctl_device_count(void)
{
	uint32_t count = 0;
	int32_t ret;

	ret = pdm_device_list(NULL, &count);
	return (ret == OSAL_SUCCESS) ? count : 0;
}

static uint32_t pdm_ctl_device_type(pdm_device_type_t type)
{
	switch (type) {
	case PDM_DEVICE_TYPE_MCU:
		return PDM_CTL_DEVICE_TYPE_MCU;
	case PDM_DEVICE_TYPE_LED:
		return PDM_CTL_DEVICE_TYPE_LED;
	default:
		return PDM_CTL_DEVICE_TYPE_INVALID;
	}
}

static uint32_t pdm_ctl_device_state(pdm_device_state_t state)
{
	switch (state) {
	case PDM_DEVICE_STATE_REGISTERED:
		return PDM_CTL_DEVICE_STATE_REGISTERED;
	case PDM_DEVICE_STATE_BOUND:
		return PDM_CTL_DEVICE_STATE_BOUND;
	case PDM_DEVICE_STATE_ERROR:
		return PDM_CTL_DEVICE_STATE_ERROR;
	default:
		return PDM_CTL_DEVICE_STATE_ERROR;
	}
}

static void pdm_ctl_fill_device_info(const pdm_device_info_t *src,
				     struct pdm_ctl_device_info *dst)
{
	osal_memset(dst, 0, sizeof(*dst));
	dst->type = pdm_ctl_device_type(src->type);
	dst->index = src->index;
	dst->state = pdm_ctl_device_state(src->state);
	dst->last_error = src->last_error;
	dst->error_count = src->error_count;
	dst->capabilities = src->capabilities;
	osal_strncpy(dst->name, src->name, sizeof(dst->name) - 1U);
	dst->name[sizeof(dst->name) - 1U] = '\0';
	osal_strncpy(dst->driver_name, src->driver_name,
		     sizeof(dst->driver_name) - 1U);
	dst->driver_name[sizeof(dst->driver_name) - 1U] = '\0';
}

static long pdm_ctl_ioctl_get_info(unsigned long arg)
{
	struct pdm_ctl_info info;

	osal_memset(&info, 0, sizeof(info));
	info.abi_version = PDM_CTL_ABI_VERSION;
	info.module_version_major = PDM_CTL_VERSION_MAJOR;
	info.module_version_minor = PDM_CTL_VERSION_MINOR;
	info.module_version_patch = PDM_CTL_VERSION_PATCH;
	info.open_count = pdm_chrdev_open_count(&g_lpf_ctl_chrdev);
	info.device_count = pdm_ctl_device_count();

	if (osal_copy_to_user((void __user *)arg, &info, sizeof(info)) !=
	    OSAL_SUCCESS)
		return -EFAULT;

	return 0;
}

static long pdm_ctl_ioctl_get_device(unsigned long arg)
{
	struct pdm_ctl_device_query query;
	pdm_device_info_t *infos;
	uint32_t count;
	int32_t ret;

	if (osal_copy_from_user(&query, (void __user *)arg, sizeof(query)) !=
	    OSAL_SUCCESS)
		return -EFAULT;

	if (query.match_index >= pdm_ctl_device_count())
		return -ENOENT;

	infos = osal_zalloc(sizeof(*infos) * (query.match_index + 1U));
	if (!infos)
		return -ENOMEM;

	count = query.match_index + 1U;
	ret = pdm_device_list(infos, &count);
	if (ret != OSAL_SUCCESS && ret != OSAL_ERR_RESOURCE_LIMIT) {
		osal_free(infos);
		return -EIO;
	}
	if (count <= query.match_index) {
		osal_free(infos);
		return -ENOENT;
	}

	pdm_ctl_fill_device_info(&infos[query.match_index], &query.info);
	osal_free(infos);

	if (osal_copy_to_user((void __user *)arg, &query, sizeof(query)) !=
	    OSAL_SUCCESS)
		return -EFAULT;

	return 0;
}

static long pdm_ctl_ioctl_get_device_by_name(unsigned long arg)
{
	struct pdm_ctl_device_name_query query;
	pdm_device_info_t info;
	int32_t ret;

	if (osal_copy_from_user(&query, (void __user *)arg, sizeof(query)) !=
	    OSAL_SUCCESS)
		return -EFAULT;

	query.name[sizeof(query.name) - 1U] = '\0';
	ret = pdm_device_get_info_by_name(query.name, &info);
	if (ret != OSAL_SUCCESS)
		return -ENOENT;

	pdm_ctl_fill_device_info(&info, &query.info);
	if (osal_copy_to_user((void __user *)arg, &query, sizeof(query)) !=
	    OSAL_SUCCESS)
		return -EFAULT;

	return 0;
}

static long pdm_ctl_ioctl_get_device_by_capability(unsigned long arg)
{
	struct pdm_ctl_device_query query;
	pdm_device_info_t info;
	int32_t ret;

	if (osal_copy_from_user(&query, (void __user *)arg, sizeof(query)) !=
	    OSAL_SUCCESS)
		return -EFAULT;

	ret = pdm_device_get_info_by_capability(query.required_capabilities,
						query.match_index, &info);
	if (ret != OSAL_SUCCESS)
		return -ENOENT;

	pdm_ctl_fill_device_info(&info, &query.info);
	if (osal_copy_to_user((void __user *)arg, &query, sizeof(query)) !=
	    OSAL_SUCCESS)
		return -EFAULT;

	return 0;
}

static long pdm_ctl_ioctl(struct file *file, unsigned int cmd,
			  unsigned long arg)
{
	(void)file;

	switch (cmd) {
	case PDM_CTL_IOC_GET_INFO:
		return pdm_ctl_ioctl_get_info(arg);
	case PDM_CTL_IOC_GET_DEVICE:
		return pdm_ctl_ioctl_get_device(arg);
	case PDM_CTL_IOC_GET_DEVICE_BY_NAME:
		return pdm_ctl_ioctl_get_device_by_name(arg);
	case PDM_CTL_IOC_GET_DEVICE_BY_CAPABILITY:
		return pdm_ctl_ioctl_get_device_by_capability(arg);
	default:
		return -ENOTTY;
	}
}

#ifdef CONFIG_COMPAT
static long pdm_ctl_compat_ioctl(struct file *file, unsigned int cmd,
				 unsigned long arg)
{
	return pdm_ctl_ioctl(file, cmd, arg);
}
#endif

static int pdm_ctl_open(struct inode *inode, struct file *file)
{
	(void)inode;

	return pdm_chrdev_open(file);
}

static int pdm_ctl_release(struct inode *inode, struct file *file)
{
	(void)inode;

	return pdm_chrdev_release(file);
}

static const struct file_operations g_lpf_ctl_fops = {
	.owner = THIS_MODULE,
	.open = pdm_ctl_open,
	.release = pdm_ctl_release,
	.unlocked_ioctl = pdm_ctl_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = pdm_ctl_compat_ioctl,
#endif
};

int32_t pdm_ctl_chrdev_register(void)
{
	int ret;

	ret = pdm_chrdev_register(&g_lpf_ctl_chrdev, PDM_CTL_DEVICE_NAME,
				  &g_lpf_ctl_fops);
	if (ret == 0)
		return OSAL_SUCCESS;

	return ret < 0 ? -ret : ret;
}

void pdm_ctl_chrdev_unregister(void)
{
	pdm_chrdev_unregister(&g_lpf_ctl_chrdev);
}
