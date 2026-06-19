// SPDX-License-Identifier: GPL-2.0

#include "pdm_proc.h"

#include <linux/errno.h>

#define PDM_PROC_ROOT_NAME "pdm"

static struct proc_dir_entry *g_pdm_proc_root;
static osal_mutex_t g_pdm_proc_lock;
static bool g_pdm_proc_lock_ready;
static uint32_t g_pdm_proc_users;

int pdm_proc_root_init(void)
{
	int ret;

	if (!g_pdm_proc_lock_ready) {
		ret = osal_mutex_init(&g_pdm_proc_lock, NULL);
		if (ret != OSAL_SUCCESS)
			return -ret;
		g_pdm_proc_lock_ready = true;
	}

	osal_mutex_lock(&g_pdm_proc_lock);
	if (!g_pdm_proc_root) {
		g_pdm_proc_root = proc_mkdir(PDM_PROC_ROOT_NAME, NULL);
		if (!g_pdm_proc_root) {
			osal_mutex_unlock(&g_pdm_proc_lock);
			return -ENOMEM;
		}
	}
	g_pdm_proc_users++;
	osal_mutex_unlock(&g_pdm_proc_lock);

	return 0;
}

void pdm_proc_root_deinit(void)
{
	if (!g_pdm_proc_lock_ready)
		return;

	osal_mutex_lock(&g_pdm_proc_lock);
	if (g_pdm_proc_users > 0)
		g_pdm_proc_users--;
	if (g_pdm_proc_users == 0 && g_pdm_proc_root) {
		proc_remove(g_pdm_proc_root);
		g_pdm_proc_root = NULL;
	}
	osal_mutex_unlock(&g_pdm_proc_lock);
}

int pdm_proc_register(pdm_proc_entry_t *entry, const char *name,
		      pdm_proc_show_t show, void *data)
{
	int ret;

	if (!entry || !name || !show)
		return -EINVAL;

	ret = pdm_proc_root_init();
	if (ret)
		return ret;

	osal_memset(entry, 0, sizeof(*entry));
	entry->name = name;
	entry->show = show;
	entry->data = data;
	entry->entry = proc_create_single_data(name, 0444, g_pdm_proc_root,
					       show, data);
	if (!entry->entry) {
		pdm_proc_root_deinit();
		osal_memset(entry, 0, sizeof(*entry));
		return -ENOMEM;
	}

	LOG_INFO(name, "/proc/%s/%s ready", PDM_PROC_ROOT_NAME, name);
	return 0;
}

void pdm_proc_unregister(pdm_proc_entry_t *entry)
{
	if (!entry || !entry->entry)
		return;

	proc_remove(entry->entry);
	osal_memset(entry, 0, sizeof(*entry));
	pdm_proc_root_deinit();
}
