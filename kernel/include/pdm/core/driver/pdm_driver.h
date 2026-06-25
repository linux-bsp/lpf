// SPDX-License-Identifier: GPL-2.0
/**
 * @file pdm_driver.h
 * @brief Common helpers for PDM drivers (LED, MCU, etc.)
 *
 * This header provides a unified framework to reduce code duplication across
 * PDM drivers. It handles common patterns like device lifecycle management
 * and state protection.
 */

#ifndef PDM_DRIVER_HELPER_H
#define PDM_DRIVER_HELPER_H

#include <linux/mutex.h>
#include <linux/types.h>

#include "pdm/core/chardev/pdm_client.h"
#include "pdm/core/device/pdm_device.h"

/**
 * struct pdm_driver_instance - Base structure for driver instances
 * @client: Embedded PDM client for chardev registration
 * @pdm_dev: Associated PDM device
 * @lock: Mutex for protecting device state
 * @online: Device is bound and operational
 *
 * PDM drivers should embed this structure and add their specific fields.
 */
struct pdm_driver_instance {
	struct pdm_client client;
	struct pdm_device *pdm_dev;
	struct mutex lock;
	bool online;
};

/**
 * pdm_driver_claim() - Acquire device lock and verify it's online
 * @inst: Driver instance
 *
 * Returns: 0 on success, -ENODEV if device is offline
 */
static inline int pdm_driver_claim(struct pdm_driver_instance *inst)
{
	mutex_lock(&inst->lock);
	if (!inst->online) {
		mutex_unlock(&inst->lock);
		return -ENODEV;
	}
	return 0;
}

/**
 * pdm_driver_release() - Release device lock
 * @inst: Driver instance
 */
static inline void pdm_driver_release(struct pdm_driver_instance *inst)
{
	mutex_unlock(&inst->lock);
}

/**
 * pdm_driver_init() - Initialize common driver instance fields
 * @inst: Driver instance to initialize
 * @pdm_dev: PDM device to associate with
 */
static inline void pdm_driver_init(struct pdm_driver_instance *inst,
				   struct pdm_device *pdm_dev)
{
	inst->pdm_dev = pdm_dev;
	mutex_init(&inst->lock);
	inst->online = true;
}

/**
 * pdm_driver_shutdown() - Mark driver offline for cleanup
 * @inst: Driver instance
 *
 * Returns with the mutex held so caller can perform cleanup, then unlock manually.
 */
static inline void pdm_driver_shutdown(struct pdm_driver_instance *inst)
{
	mutex_lock(&inst->lock);
	inst->online = false;
}

#endif /* PDM_DRIVER_HELPER_H */
