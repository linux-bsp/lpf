// SPDX-License-Identifier: GPL-2.0

#include "osal/osal.h"

int32_t osal_rwlock_init(osal_rwlock_t *rwlock,
			 const osal_rwlock_attr_t *attr)
{
	(void)attr;

	if (!rwlock) {
		return OSAL_ERR_INVALID_POINTER;
	}

	init_rwsem(&rwlock->sem);
	atomic_set(&rwlock->writer, 0);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_rwlock_init);

int32_t osal_rwlock_destroy(osal_rwlock_t *rwlock)
{
	if (!rwlock) {
		return OSAL_ERR_INVALID_POINTER;
	}

	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_rwlock_destroy);

int32_t osal_rwlock_read_lock(osal_rwlock_t *rwlock)
{
	if (!rwlock) {
		return OSAL_ERR_INVALID_POINTER;
	}

	down_read(&rwlock->sem);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_rwlock_read_lock);

int32_t osal_rwlock_write_lock(osal_rwlock_t *rwlock)
{
	if (!rwlock) {
		return OSAL_ERR_INVALID_POINTER;
	}

	down_write(&rwlock->sem);
	atomic_set(&rwlock->writer, 1);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_rwlock_write_lock);

int32_t osal_rwlock_try_read_lock(osal_rwlock_t *rwlock)
{
	if (!rwlock) {
		return OSAL_ERR_INVALID_POINTER;
	}

	if (!down_read_trylock(&rwlock->sem)) {
		return OSAL_ERR_BUSY;
	}

	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_rwlock_try_read_lock);

int32_t osal_rwlock_try_write_lock(osal_rwlock_t *rwlock)
{
	if (!rwlock) {
		return OSAL_ERR_INVALID_POINTER;
	}

	if (!down_write_trylock(&rwlock->sem)) {
		return OSAL_ERR_BUSY;
	}

	atomic_set(&rwlock->writer, 1);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_rwlock_try_write_lock);

int32_t osal_rwlock_unlock(osal_rwlock_t *rwlock)
{
	if (!rwlock) {
		return OSAL_ERR_INVALID_POINTER;
	}

	if (atomic_xchg(&rwlock->writer, 0)) {
		up_write(&rwlock->sem);
	}
	else {
		up_read(&rwlock->sem);
	}

	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_rwlock_unlock);
