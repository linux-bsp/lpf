// SPDX-License-Identifier: GPL-2.0

#include "pdm_chrdev.h"

#include <linux/container_of.h>
#include <linux/errno.h>

int pdm_chrdev_open(pdm_chrdev_t *chrdev)
{
	int open_count;

	if (!chrdev)
		return -EINVAL;

	osal_mutex_lock(&chrdev->lock);
	open_count = (int)osal_atomic_inc(&chrdev->open_count);
	osal_mutex_unlock(&chrdev->lock);

	LOG_INFO(chrdev->name, "open count=%d", open_count);
	return 0;
}

int pdm_chrdev_release(pdm_chrdev_t *chrdev)
{
	int open_count;

	if (!chrdev)
		return -EINVAL;

	osal_mutex_lock(&chrdev->lock);
	if (osal_atomic_load(&chrdev->open_count) > 0)
		osal_atomic_dec(&chrdev->open_count);
	open_count = (int)osal_atomic_load(&chrdev->open_count);
	osal_mutex_unlock(&chrdev->lock);

	LOG_INFO(chrdev->name, "release count=%d", open_count);
	return 0;
}

int pdm_chrdev_register(pdm_chrdev_t *chrdev, const char *name,
			const struct file_operations *fops)
{
	return pdm_chrdev_register_instance(chrdev, name, NULL, 0, fops);
}

int pdm_chrdev_register_instance(pdm_chrdev_t *chrdev, const char *name,
				 const char *nodename, uint32_t index,
				 const struct file_operations *fops)
{
	int ret;

	if (!chrdev || !name || !fops)
		return -EINVAL;
	if (chrdev->registered)
		return -EBUSY;

	ret = osal_mutex_init(&chrdev->lock, NULL);
	if (ret != OSAL_SUCCESS)
		return -ret;

	osal_strncpy(chrdev->name, name, sizeof(chrdev->name));
	chrdev->name[sizeof(chrdev->name) - 1U] = '\0';
	if (nodename) {
		osal_strncpy(chrdev->nodename, nodename,
			     sizeof(chrdev->nodename));
		chrdev->nodename[sizeof(chrdev->nodename) - 1U] = '\0';
	}
	chrdev->fops = fops;
	chrdev->index = index;
	osal_atomic_init(&chrdev->open_count, 0);

	chrdev->miscdev.minor = MISC_DYNAMIC_MINOR;
	chrdev->miscdev.name = chrdev->name;
	chrdev->miscdev.fops = fops;
	chrdev->miscdev.nodename = nodename ? chrdev->nodename : NULL;
	chrdev->miscdev.mode = 0666;

	ret = misc_register(&chrdev->miscdev);
	if (ret) {
		osal_mutex_destroy(&chrdev->lock);
		return ret;
	}

	chrdev->registered = true;
	LOG_INFO(chrdev->name, "/dev/%s ready",
		 chrdev->miscdev.nodename ? chrdev->miscdev.nodename :
					    chrdev->name);
	return 0;
}

void pdm_chrdev_unregister(pdm_chrdev_t *chrdev)
{
	if (!chrdev)
		return;
	if (!chrdev->registered)
		return;

	misc_deregister(&chrdev->miscdev);
	osal_mutex_destroy(&chrdev->lock);
	osal_memset(chrdev, 0, sizeof(*chrdev));
}

pdm_chrdev_t *pdm_chrdev_from_file(struct file *file)
{
	struct miscdevice *miscdev;

	if (!file || !file->private_data)
		return NULL;

	miscdev = file->private_data;
	return container_of(miscdev, pdm_chrdev_t, miscdev);
}

uint32_t pdm_chrdev_open_count(const pdm_chrdev_t *chrdev)
{
	if (!chrdev)
		return 0;

	return osal_atomic_load(&chrdev->open_count);
}

uint32_t pdm_chrdev_index(const pdm_chrdev_t *chrdev)
{
	if (!chrdev)
		return 0;

	return chrdev->index;
}
