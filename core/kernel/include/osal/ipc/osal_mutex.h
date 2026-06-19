/************************************************************************
 * Kernel OSAL mutex API
 ************************************************************************/

#ifndef OSAL_MUTEX_H
#define OSAL_MUTEX_H

#include <linux/mutex.h>

#include "osal_types.h"

#define OSAL_MUTEX_NORMAL 0
#define OSAL_MUTEX_RECURSIVE 1
#define OSAL_MUTEX_PRIO_INHERIT 0

typedef struct mutex osal_mutex_t;

typedef struct {
	int32_t type;
	int32_t protocol;
} osal_mutex_attr_t;

int32_t osal_mutex_init(osal_mutex_t *mutex, const osal_mutex_attr_t *attr);
int32_t osal_mutex_destroy(osal_mutex_t *mutex);
int32_t osal_mutex_lock(osal_mutex_t *mutex);
int32_t osal_mutex_try_lock(osal_mutex_t *mutex);
int32_t osal_mutex_timed_lock(osal_mutex_t *mutex, uint32_t timeout_ms);
int32_t osal_mutex_unlock(osal_mutex_t *mutex);
int32_t osal_mutex_attr_init(osal_mutex_attr_t *attr);
int32_t osal_mutex_attr_destroy(osal_mutex_attr_t *attr);
int32_t osal_mutex_attr_set_type(osal_mutex_attr_t *attr, int32_t type);
int32_t osal_mutex_attr_set_protocol(osal_mutex_attr_t *attr, int32_t protocol);
int32_t osal_mutex_attr_get_type(const osal_mutex_attr_t *attr, int32_t *type);

#endif /* OSAL_MUTEX_H */
