// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_DEBUGFS_H
#define PDM_DEBUGFS_H

#include <linux/dcache.h>
#include <linux/types.h>

#include "osal.h"

#define PDM_DEBUGFS_WRITE_MAX_SIZE 256U

typedef int (*pdm_debugfs_write_t)(char *command, size_t count, void *data);

typedef struct {
	const char *name;
	pdm_debugfs_write_t write;
	void *data;
	struct dentry *entry;
} pdm_debugfs_entry_t;

int pdm_debugfs_root_init(void);
void pdm_debugfs_root_deinit(void);
int pdm_debugfs_register(pdm_debugfs_entry_t *entry, const char *name,
			 pdm_debugfs_write_t write, void *data);
void pdm_debugfs_unregister(pdm_debugfs_entry_t *entry);

#endif /* PDM_DEBUGFS_H */
