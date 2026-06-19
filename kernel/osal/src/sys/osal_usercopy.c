// SPDX-License-Identifier: GPL-2.0

#include "osal/osal.h"

#include <linux/uaccess.h>

int32_t osal_copy_to_user(void __user *to, const void *from, uint32_t size)
{
	if (!to || !from)
		return OSAL_ERR_INVALID_POINTER;

	if (copy_to_user(to, from, size))
		return OSAL_ERR_BAD_ADDRESS;

	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_copy_to_user);

int32_t osal_copy_from_user(void *to, const void __user *from, uint32_t size)
{
	if (!to || !from)
		return OSAL_ERR_INVALID_POINTER;

	if (copy_from_user(to, from, size))
		return OSAL_ERR_BAD_ADDRESS;

	return OSAL_SUCCESS;
}
EXPORT_SYMBOL_GPL(osal_copy_from_user);
