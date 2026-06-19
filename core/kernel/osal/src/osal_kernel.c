// SPDX-License-Identifier: GPL-2.0

#include "osal/osal.h"

#include <linux/module.h>

const char *osal_kernel_version(void)
{
	return "kernel-osal-0.1";
}
EXPORT_SYMBOL_GPL(osal_kernel_version);

uint16_t osal_crc16_ccitt_update(uint16_t crc, const uint8_t *data, size_t len)
{
	size_t i;
	uint8_t bit;

	if (!data)
		return crc;

	for (i = 0; i < len; i++) {
		crc ^= (uint16_t)data[i] << 8;
		for (bit = 0; bit < 8; bit++) {
			if (crc & 0x8000)
				crc = (uint16_t)((crc << 1) ^ 0x1021);
			else
				crc <<= 1;
		}
	}

	return crc;
}
EXPORT_SYMBOL_GPL(osal_crc16_ccitt_update);

uint16_t osal_crc16_ccitt(const uint8_t *data, size_t len)
{
	if (!data)
		return 0xFFFF;

	return osal_crc16_ccitt_update(0xFFFF, data, len);
}
EXPORT_SYMBOL_GPL(osal_crc16_ccitt);

uint16_t osal_crc16_modbus(const uint8_t *data, size_t len)
{
	uint16_t crc = 0xFFFF;
	size_t i;
	uint8_t bit;

	if (!data)
		return crc;

	for (i = 0; i < len; i++) {
		crc ^= data[i];
		for (bit = 0; bit < 8; bit++) {
			if (crc & 0x0001)
				crc = (crc >> 1) ^ 0xA001;
			else
				crc >>= 1;
		}
	}

	return crc;
}
EXPORT_SYMBOL_GPL(osal_crc16_modbus);

uint32_t osal_crc32(const uint8_t *data, size_t len)
{
	uint32_t crc = 0xFFFFFFFFU;
	size_t i;
	uint8_t bit;

	if (!data)
		return crc;

	for (i = 0; i < len; i++) {
		crc ^= data[i];
		for (bit = 0; bit < 8; bit++) {
			if (crc & 1U)
				crc = (crc >> 1) ^ 0xEDB88320U;
			else
				crc >>= 1;
		}
	}

	return ~crc;
}
EXPORT_SYMBOL_GPL(osal_crc32);

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

MODULE_AUTHOR("ES-Middleware");
MODULE_DESCRIPTION("ES-Middleware OSAL kernel module");
MODULE_LICENSE("GPL");
