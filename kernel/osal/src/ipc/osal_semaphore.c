// SPDX-License-Identifier: GPL-2.0

#include "osal/osal.h"

#include <linux/jiffies.h>

static void osal_sem_value_dec(osal_sem_t *sem)
{
	atomic_dec_if_positive(&sem->value);
}

int32_t osal_sem_init(osal_sem_t *sem, int32_t pshared, uint32_t value)
{
	if (!sem) {
		return OSAL_ERR_INVALID_POINTER;
	}
	if (pshared) {
		return OSAL_ERR_NOT_SUPPORTED;
	}
	if (value > INT_MAX) {
		return OSAL_ERR_INVALID_SIZE;
	}

	sema_init(&sem->sem, value);
	atomic_set(&sem->value, (int)value);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_sem_init);

int32_t osal_sem_destroy(osal_sem_t *sem)
{
	if (!sem) {
		return OSAL_ERR_INVALID_POINTER;
	}

	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_sem_destroy);

int32_t osal_sem_wait(osal_sem_t *sem)
{
	if (!sem) {
		return OSAL_ERR_INVALID_POINTER;
	}

	if (down_interruptible(&sem->sem)) {
		return OSAL_ERR_INTERRUPTED;
	}

	osal_sem_value_dec(sem);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_sem_wait);

int32_t osal_sem_try_wait(osal_sem_t *sem)
{
	if (!sem) {
		return OSAL_ERR_INVALID_POINTER;
	}

	if (down_trylock(&sem->sem)) {
		return OSAL_ERR_BUSY;
	}

	osal_sem_value_dec(sem);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_sem_try_wait);

int32_t osal_sem_timed_wait(osal_sem_t *sem, uint32_t timeout_ms)
{
	long ret;

	if (!sem) {
		return OSAL_ERR_INVALID_POINTER;
	}
	if (timeout_ms == 0) {
		return osal_sem_try_wait(sem);
	}

	ret = down_timeout(&sem->sem, msecs_to_jiffies(timeout_ms));
	if (ret == -ETIME) {
		return OSAL_ERR_TIMEOUT;
	}
	if (ret) {
		return OSAL_ERR_GENERIC;
	}

	osal_sem_value_dec(sem);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_sem_timed_wait);

int32_t osal_sem_post(osal_sem_t *sem)
{
	if (!sem) {
		return OSAL_ERR_INVALID_POINTER;
	}

	up(&sem->sem);
	atomic_inc(&sem->value);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_sem_post);

int32_t osal_sem_get_value(osal_sem_t *sem, int32_t *value)
{
	if (!sem || !value) {
		return OSAL_ERR_INVALID_POINTER;
	}

	*value = atomic_read(&sem->value);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_sem_get_value);
