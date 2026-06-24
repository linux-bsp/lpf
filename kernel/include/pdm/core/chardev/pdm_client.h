// SPDX-License-Identifier: GPL-2.0
/**
 * @file pdm_client.h
 * @brief PDM per-instance character device helper
 */

#ifndef PDM_CLIENT_H
#define PDM_CLIENT_H

#include <linux/atomic.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/types.h>

#include "pdm/core/device/pdm_device.h"

#define PDM_CLIENT_NAME_LEN 64U
#define PDM_CLIENT_NODE_LEN 64U
#define PDM_CLIENT_MINORS (MINORMASK + 1)

struct pdm_client {
	struct pdm_device *pdm_dev;
	struct device dev;
	struct cdev cdev;
	const struct file_operations *fops;
	void (*release)(struct pdm_client *client);
	atomic_t open_count;
	int minor;
	bool registered;
	char name[PDM_CLIENT_NAME_LEN];
	char nodename[PDM_CLIENT_NODE_LEN];
};

int pdm_client_init(void);
void pdm_client_exit(void);
int pdm_client_register(struct pdm_client *client, struct pdm_device *pdm_dev,
			const char *name, const char *nodename,
			const struct file_operations *fops,
			void (*release)(struct pdm_client *client));
void pdm_client_unregister(struct pdm_client *client);
u32 pdm_client_open_count(const struct pdm_client *client);
struct pdm_client *pdm_client_from_file(struct file *filp);
int pdm_client_default_open(struct inode *inode, struct file *filp);
int pdm_client_default_release(struct inode *inode, struct file *filp);

#endif /* PDM_CLIENT_H */
