/************************************************************************
 * Kernel OSAL thread API
 ************************************************************************/

#ifndef OSAL_THREAD_H
#define OSAL_THREAD_H

#include <linux/sched.h>

#include "osal_types.h"

typedef struct osal_thread_s *osal_thread_t;

typedef struct {
	osal_size_t stack_size;
	int32_t detach_state;
	int32_t sched_policy;
	int32_t sched_priority;
	char name[OS_MAX_API_NAME + 1];
} osal_thread_attr_t;

typedef struct {
	int32_t sched_priority;
} osal_sched_param_t;

#define OSAL_THREAD_CREATE_JOINABLE 0
#define OSAL_THREAD_CREATE_DETACHED 1
#define OSAL_SCHED_OTHER SCHED_NORMAL
#define OSAL_SCHED_FIFO SCHED_FIFO
#define OSAL_SCHED_RR SCHED_RR

int32_t osal_thread_create(osal_thread_t *thread,
			   const osal_thread_attr_t *attr,
			   void *(*start_routine)(void *), void *arg);
int32_t osal_thread_join(osal_thread_t thread, void **retval);
int32_t osal_thread_detach(osal_thread_t thread);
bool osal_thread_equal(osal_thread_t thread1, osal_thread_t thread2);
osal_thread_t osal_thread_self(void);
void osal_thread_exit(void *retval);
int32_t osal_thread_cancel(osal_thread_t thread);
int32_t osal_thread_should_stop(void);
int32_t osal_thread_attr_init(osal_thread_attr_t *attr);
int32_t osal_thread_attr_destroy(osal_thread_attr_t *attr);
int32_t osal_thread_attr_set_stack_size(osal_thread_attr_t *attr,
					osal_size_t stacksize);
int32_t osal_thread_attr_get_stack_size(const osal_thread_attr_t *attr,
					osal_size_t *stacksize);
int32_t osal_thread_attr_set_detach_state(osal_thread_attr_t *attr,
					  int32_t detachstate);
int32_t osal_thread_attr_get_detach_state(const osal_thread_attr_t *attr,
					  int32_t *detachstate);
int32_t osal_thread_attr_set_sched_policy(osal_thread_attr_t *attr,
					  int32_t policy);
int32_t osal_thread_attr_set_sched_param(osal_thread_attr_t *attr,
					 const osal_sched_param_t *param);
int32_t osal_thread_attr_set_name(osal_thread_attr_t *attr, const char *name);

#endif /* OSAL_THREAD_H */
