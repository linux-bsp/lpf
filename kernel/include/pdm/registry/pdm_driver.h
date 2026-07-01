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

#include <linux/atomic.h>
#include <linux/types.h>


/**
 * pdm_driver_count_inc() - Increment device count for a driver type
 * @count: Pointer to atomic device counter
 */
static inline void pdm_driver_count_inc(atomic_t *count)
{
	atomic_inc(count);
}

/**
 * pdm_driver_count_dec() - Decrement device count for a driver type
 * @count: Pointer to atomic device counter
 */
static inline void pdm_driver_count_dec(atomic_t *count)
{
	atomic_dec_if_positive(count);
}

/**
 * pdm_driver_count_get() - Get current device count
 * @count: Pointer to atomic device counter
 *
 * Returns: Current device count as u32
 */
static inline u32 pdm_driver_count_get(atomic_t *count)
{
	return (u32)atomic_read(count);
}

#endif /* PDM_DRIVER_HELPER_H */
