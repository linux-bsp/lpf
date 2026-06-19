// SPDX-License-Identifier: GPL-2.0

#ifndef OSAL_RWLOCK_H
#define OSAL_RWLOCK_H

#include <linux/atomic.h>
#include <linux/rwsem.h>

#include "osal_types.h"

typedef struct {
	struct rw_semaphore sem;
	atomic_t writer;
} osal_rwlock_t;

typedef struct {
	uint32_t reserved;
} osal_rwlock_attr_t;

int32_t osal_rwlock_init(osal_rwlock_t *rwlock,
			 const osal_rwlock_attr_t *attr);
int32_t osal_rwlock_destroy(osal_rwlock_t *rwlock);
int32_t osal_rwlock_read_lock(osal_rwlock_t *rwlock);
int32_t osal_rwlock_write_lock(osal_rwlock_t *rwlock);
int32_t osal_rwlock_try_read_lock(osal_rwlock_t *rwlock);
int32_t osal_rwlock_try_write_lock(osal_rwlock_t *rwlock);
int32_t osal_rwlock_unlock(osal_rwlock_t *rwlock);

#endif /* OSAL_RWLOCK_H */
