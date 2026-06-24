// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_COMPAT_I2C_H
#define PDM_COMPAT_I2C_H

#include "pdm/compat/pdm_compat_features.h"

#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>

typedef int (*pdm_compat_i2c_probe_t)(struct i2c_client *client);
typedef void (*pdm_compat_i2c_remove_t)(struct i2c_client *client);

struct pdm_compat_i2c_driver {
	struct i2c_driver driver;
	pdm_compat_i2c_probe_t probe;
	pdm_compat_i2c_remove_t remove;
};

static inline struct pdm_compat_i2c_driver *
pdm_compat_i2c_from_client(struct i2c_client *client)
{
	struct device_driver *dev_driver;
	struct i2c_driver *i2c_driver;

	if (!client) {
		return NULL;
	}

	dev_driver = client->dev.driver;
	if (!dev_driver) {
		return NULL;
	}

	i2c_driver = container_of(dev_driver, struct i2c_driver, driver);
	return container_of(i2c_driver, struct pdm_compat_i2c_driver, driver);
}

static inline int pdm_compat_i2c_probe(struct i2c_client *client)
{
	struct pdm_compat_i2c_driver *driver;

	driver = pdm_compat_i2c_from_client(client);
	if (!driver || !driver->probe) {
		return -ENODEV;
	}

	return driver->probe(client);
}

static inline void pdm_compat_i2c_remove_device(struct i2c_client *client)
{
	struct pdm_compat_i2c_driver *driver;

	driver = pdm_compat_i2c_from_client(client);
	if (driver && driver->remove) {
		driver->remove(client);
	}
}

static inline void pdm_compat_i2c_remove_void(struct i2c_client *client)
{
	pdm_compat_i2c_remove_device(client);
}

static inline int pdm_compat_i2c_remove_int(struct i2c_client *client)
{
	pdm_compat_i2c_remove_device(client);
	return 0;
}

static inline int
pdm_compat_i2c_driver_register(struct pdm_compat_i2c_driver *driver)
{
	if (!driver) {
		return -EINVAL;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
	driver->driver.probe = pdm_compat_i2c_probe;
	driver->driver.remove = pdm_compat_i2c_remove_void;
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
	driver->driver.probe_new = pdm_compat_i2c_probe;
	driver->driver.remove = pdm_compat_i2c_remove_void;
#else
	driver->driver.probe_new = pdm_compat_i2c_probe;
	driver->driver.remove = pdm_compat_i2c_remove_int;
#endif

	return i2c_register_driver(THIS_MODULE, &driver->driver);
}

static inline void
pdm_compat_i2c_driver_unregister(struct pdm_compat_i2c_driver *driver)
{
	if (driver) {
		i2c_del_driver(&driver->driver);
	}
}

#endif /* PDM_COMPAT_I2C_H */
