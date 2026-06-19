// SPDX-License-Identifier: GPL-2.0

#ifndef OSAL_SEMAPHORE_H
#define OSAL_SEMAPHORE_H

#include <linux/atomic.h>
#include <linux/semaphore.h>

#include "osal_types.h"

typedef struct {
	struct semaphore sem;
	atomic_t value;
} osal_sem_t;

int32_t osal_sem_init(osal_sem_t *sem, int32_t pshared, uint32_t value);
int32_t osal_sem_destroy(osal_sem_t *sem);
int32_t osal_sem_wait(osal_sem_t *sem);
int32_t osal_sem_try_wait(osal_sem_t *sem);
int32_t osal_sem_timed_wait(osal_sem_t *sem, uint32_t timeout_ms);
int32_t osal_sem_post(osal_sem_t *sem);
int32_t osal_sem_get_value(osal_sem_t *sem, int32_t *value);

#endif /* OSAL_SEMAPHORE_H */
