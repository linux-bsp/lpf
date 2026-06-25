// SPDX-License-Identifier: GPL-2.0
/**
 * @file pdm_instance.h
 * @brief PDM driver instance base structure (decoupled from character devices)
 *
 * This provides the core instance management without forcing character device
 * dependency. Drivers that need /dev nodes should use pdm_cdev_instance instead.
 */

#ifndef PDM_INSTANCE_H
#define PDM_INSTANCE_H

#include <linux/mutex.h>
#include <linux/types.h>

struct pdm_device;

/**
 * struct pdm_instance - Base structure for PDM driver instances
 * @pdm_dev: Associated PDM device
 * @lock: Mutex for protecting device state
 * @online: Device is bound and operational
 *
 * This is the minimal base structure for all PDM driver instances.
 * It provides lifecycle management and concurrency control without
 * imposing any user-space interface requirements.
 */
struct pdm_instance {
	struct pdm_device *pdm_dev;
	struct mutex lock;
	bool online;
};

/**
 * pdm_instance_init() - Initialize a PDM instance
 * @inst: Instance to initialize
 * @pdm_dev: PDM device to associate with
 */
static inline void pdm_instance_init(struct pdm_instance *inst,
				     struct pdm_device *pdm_dev)
{
	if (!inst)
		return;

	inst->pdm_dev = pdm_dev;
	mutex_init(&inst->lock);
	inst->online = true;
}

/**
 * pdm_instance_claim() - Acquire device lock and verify it's online
 * @inst: Driver instance
 *
 * Returns: 0 on success, -ENODEV if device is offline
 */
static inline int pdm_instance_claim(struct pdm_instance *inst)
{
	if (!inst)
		return -EINVAL;

	mutex_lock(&inst->lock);
	if (!inst->online) {
		mutex_unlock(&inst->lock);
		return -ENODEV;
	}
	return 0;
}

/**
 * pdm_instance_release() - Release device lock
 * @inst: Driver instance
 */
static inline void pdm_instance_release(struct pdm_instance *inst)
{
	if (inst)
		mutex_unlock(&inst->lock);
}

/**
 * pdm_instance_shutdown() - Mark instance offline for cleanup
 * @inst: Driver instance
 *
 * Returns with the mutex held so caller can perform cleanup, then unlock manually.
 */
static inline void pdm_instance_shutdown(struct pdm_instance *inst)
{
	if (!inst)
		return;

	mutex_lock(&inst->lock);
	inst->online = false;
}

#endif /* PDM_INSTANCE_H */
