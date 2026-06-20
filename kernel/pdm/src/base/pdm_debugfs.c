// SPDX-License-Identifier: GPL-2.0

#include "pdm_debugfs.h"

#include <linux/debugfs.h>
#include <linux/errno.h>
#include <linux/fs.h>

#define PDM_DEBUGFS_ROOT_NAME "pdm"

static struct dentry *g_pdm_debugfs_root;
static osal_mutex_t g_pdm_debugfs_lock;
static bool g_pdm_debugfs_lock_ready;
static uint32_t g_pdm_debugfs_users;
static bool g_pdm_debugfs_available = true;

static int pdm_debugfs_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t pdm_debugfs_write(struct file *file,
				 const char __user *buffer, size_t count,
				 loff_t *ppos)
{
	pdm_debugfs_entry_t *entry = file->private_data;
	char *command;
	int32_t status;
	int ret;

	if (!entry || !entry->write)
		return -EOPNOTSUPP;

	if (count == 0)
		return 0;

	if (count > PDM_DEBUGFS_WRITE_MAX_SIZE)
		return -E2BIG;

	command = osal_malloc(count + 1);
	if (!command)
		return -ENOMEM;

	status = osal_copy_from_user(command, buffer, count);
	if (status != OSAL_SUCCESS) {
		osal_free(command);
		return -EFAULT;
	}
	command[count] = '\0';

	ret = entry->write(command, count, entry->data);
	osal_free(command);
	if (ret)
		return ret;

	if (ppos)
		*ppos += count;

	return count;
}

static const struct file_operations pdm_debugfs_fops = {
	.owner = THIS_MODULE,
	.open = pdm_debugfs_open,
	.write = pdm_debugfs_write,
};

int pdm_debugfs_root_init(void)
{
	int ret;

	if (!g_pdm_debugfs_available)
		return -ENODEV;

	if (!g_pdm_debugfs_lock_ready) {
		ret = osal_mutex_init(&g_pdm_debugfs_lock, NULL);
		if (ret != OSAL_SUCCESS)
			return -ret;
		g_pdm_debugfs_lock_ready = true;
	}

	osal_mutex_lock(&g_pdm_debugfs_lock);
	if (!g_pdm_debugfs_root) {
		g_pdm_debugfs_root = debugfs_create_dir(PDM_DEBUGFS_ROOT_NAME,
							NULL);
		if (IS_ERR_OR_NULL(g_pdm_debugfs_root)) {
			ret = g_pdm_debugfs_root ? PTR_ERR(g_pdm_debugfs_root) :
						   -ENOMEM;
			g_pdm_debugfs_root = NULL;
			g_pdm_debugfs_available = false;
			osal_mutex_unlock(&g_pdm_debugfs_lock);
			return ret;
		}
	}

	g_pdm_debugfs_users++;
	osal_mutex_unlock(&g_pdm_debugfs_lock);

	return 0;
}

void pdm_debugfs_root_deinit(void)
{
	if (!g_pdm_debugfs_lock_ready)
		return;

	osal_mutex_lock(&g_pdm_debugfs_lock);
	if (g_pdm_debugfs_users > 0)
		g_pdm_debugfs_users--;
	if (g_pdm_debugfs_users == 0 && g_pdm_debugfs_root) {
		debugfs_remove_recursive(g_pdm_debugfs_root);
		g_pdm_debugfs_root = NULL;
	}
	osal_mutex_unlock(&g_pdm_debugfs_lock);
}

int pdm_debugfs_register(pdm_debugfs_entry_t *entry, const char *name,
			 pdm_debugfs_write_t write, void *data)
{
	int ret;

	if (!entry || !name || !write)
		return -EINVAL;

	osal_memset(entry, 0, sizeof(*entry));

	ret = pdm_debugfs_root_init();
	if (ret == -ENODEV)
		return 0;
	if (ret)
		return ret;

	entry->name = name;
	entry->write = write;
	entry->data = data;
	entry->entry = debugfs_create_file(name, 0200, g_pdm_debugfs_root,
					   entry, &pdm_debugfs_fops);
	if (IS_ERR_OR_NULL(entry->entry)) {
		ret = entry->entry ? PTR_ERR(entry->entry) : -ENOMEM;
		pdm_debugfs_root_deinit();
		osal_memset(entry, 0, sizeof(*entry));
		return ret;
	}

	LOG_INFO(name, "/sys/kernel/debug/%s/%s ready",
		 PDM_DEBUGFS_ROOT_NAME, name);
	return 0;
}

void pdm_debugfs_unregister(pdm_debugfs_entry_t *entry)
{
	if (!entry || !entry->entry)
		return;

	debugfs_remove(entry->entry);
	osal_memset(entry, 0, sizeof(*entry));
	pdm_debugfs_root_deinit();
}
