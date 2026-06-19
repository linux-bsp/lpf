// SPDX-License-Identifier: GPL-2.0

#ifndef OSAL_USERCOPY_H
#define OSAL_USERCOPY_H

#include <linux/compiler_types.h>

#include "osal_types.h"

int32_t osal_copy_to_user(void __user *to, const void *from, uint32_t size);
int32_t osal_copy_from_user(void *to, const void __user *from, uint32_t size);

#endif /* OSAL_USERCOPY_H */
