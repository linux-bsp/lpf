// SPDX-License-Identifier: GPL-2.0

#include "osal/osal.h"

#include <linux/jiffies.h>
#include <linux/sched.h>

int32_t osal_mutex_init(osal_mutex_t *mutex, const osal_mutex_attr_t *attr)
{
	(void)attr;

	if (!mutex)
		return OSAL_ERR_INVALID_POINTER;

	mutex_init(mutex);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_mutex_init);

int32_t osal_mutex_destroy(osal_mutex_t *mutex)
{
	if (!mutex)
		return OSAL_ERR_INVALID_POINTER;

	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_mutex_destroy);

int32_t osal_mutex_lock(osal_mutex_t *mutex)
{
	if (!mutex)
		return OSAL_ERR_INVALID_POINTER;

	mutex_lock(mutex);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_mutex_lock);

int32_t osal_mutex_try_lock(osal_mutex_t *mutex)
{
	if (!mutex)
		return OSAL_ERR_INVALID_POINTER;

	return mutex_trylock(mutex) ? OSAL_SUCCESS : -OSAL_EBUSY;
}
EXPORT_SYMBOL_GPL(osal_mutex_try_lock);

int32_t osal_mutex_timed_lock(osal_mutex_t *mutex, uint32_t timeout_ms)
{
	unsigned long timeout;

	if (!mutex)
		return OSAL_ERR_INVALID_POINTER;

	timeout = jiffies + msecs_to_jiffies(timeout_ms);
	do {
		if (mutex_trylock(mutex))
			return OSAL_SUCCESS;
		if (time_after_eq(jiffies, timeout))
			return -OSAL_ETIMEDOUT;
		schedule_timeout_uninterruptible(1);
	} while (true);
}
EXPORT_SYMBOL_GPL(osal_mutex_timed_lock);

int32_t osal_mutex_unlock(osal_mutex_t *mutex)
{
	if (!mutex)
		return OSAL_ERR_INVALID_POINTER;

	mutex_unlock(mutex);
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_mutex_unlock);

int32_t osal_mutex_attr_init(osal_mutex_attr_t *attr)
{
	if (!attr)
		return OSAL_ERR_INVALID_POINTER;

	attr->type = OSAL_MUTEX_NORMAL;
	attr->protocol = OSAL_MUTEX_PRIO_INHERIT;
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_mutex_attr_init);

int32_t osal_mutex_attr_destroy(osal_mutex_attr_t *attr)
{
	if (!attr)
		return OSAL_ERR_INVALID_POINTER;

	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_mutex_attr_destroy);

int32_t osal_mutex_attr_set_type(osal_mutex_attr_t *attr, int32_t type)
{
	if (!attr)
		return OSAL_ERR_INVALID_POINTER;
	if (type != OSAL_MUTEX_NORMAL && type != OSAL_MUTEX_RECURSIVE)
		return OSAL_ERR_INVALID_PARAM;

	attr->type = type;
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_mutex_attr_set_type);

int32_t osal_mutex_attr_set_protocol(osal_mutex_attr_t *attr, int32_t protocol)
{
	if (!attr)
		return OSAL_ERR_INVALID_POINTER;

	attr->protocol = protocol;
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_mutex_attr_set_protocol);

int32_t osal_mutex_attr_get_type(const osal_mutex_attr_t *attr, int32_t *type)
{
	if (!attr || !type)
		return OSAL_ERR_INVALID_POINTER;

	*type = attr->type;
	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_mutex_attr_get_type);
