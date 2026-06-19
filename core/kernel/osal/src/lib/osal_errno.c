// SPDX-License-Identifier: GPL-2.0

#include "osal/osal.h"

#include <linux/module.h>

const char *osal_get_status_name(int32_t status_code)
{
	if (status_code < 0)
		status_code = -status_code;

	switch (status_code) {
	case OSAL_SUCCESS:
		return "SUCCESS";
	case OSAL_EINVAL:
		return "EINVAL";
	case OSAL_EFAULT:
		return "EFAULT";
	case OSAL_ENOMEM:
		return "ENOMEM";
	case OSAL_ENOENT:
		return "ENOENT";
	case OSAL_EPERM:
		return "EPERM";
	case OSAL_EINTR:
		return "EINTR";
	case OSAL_EAGAIN:
		return "EAGAIN";
	case OSAL_EEXIST:
		return "EEXIST";
	case OSAL_ENODEV:
		return "ENODEV";
	case OSAL_ETIMEDOUT:
		return "ETIMEDOUT";
	case OSAL_EBUSY:
		return "EBUSY";
	case OSAL_EMFILE:
		return "EMFILE";
	case OSAL_EPROTO:
		return "EPROTO";
	case OSAL_EOPNOTSUPP:
		return "EOPNOTSUPP";
	case OSAL_ECANCELED:
		return "ECANCELED";
	case OSAL_ENOSYS:
		return "ENOSYS";
	case OSAL_EIO:
		return "EIO";
	default:
		return "UNKNOWN";
	}
}
EXPORT_SYMBOL_GPL(osal_get_status_name);
