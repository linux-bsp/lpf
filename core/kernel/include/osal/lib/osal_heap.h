/************************************************************************
 * Kernel OSAL heap API
 ************************************************************************/

#ifndef OSAL_HEAP_H
#define OSAL_HEAP_H

#include "osal_types.h"

void *osal_malloc(uint32_t size);
void *osal_zalloc(uint32_t size);
void osal_free(void *ptr);
void *osal_realloc(void *ptr, uint32_t new_size);
int32_t osal_heap_get_info(uint32_t *free_bytes, uint32_t *total_bytes);
int32_t osal_heap_set_threshold(uint32_t percent);
int32_t osal_heap_check_threshold(bool *exceeded);
int32_t osal_heap_get_stats(uint32_t *current_bytes, uint32_t *peak_bytes);

#endif /* OSAL_HEAP_H */
