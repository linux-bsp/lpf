// SPDX-License-Identifier: GPL-2.0

#include "osal/osal.h"

#include <linux/module.h>

uint16_t osal_crc16_ccitt_update(uint16_t crc, const uint8_t *data, size_t len)
{
	size_t i;
	uint8_t bit;

	if (!data) {
		return crc;
	}

	for (i = 0; i < len; i++) {
		crc ^= (uint16_t)data[i] << 8;
		for (bit = 0; bit < 8; bit++) {
			if (crc & 0x8000) {
				crc = (uint16_t)((crc << 1) ^ 0x1021);
			}
			else {
				crc <<= 1;
			}
		}
	}

	return crc;
}
EXPORT_SYMBOL_GPL(osal_crc16_ccitt_update);

uint16_t osal_crc16_ccitt(const uint8_t *data, size_t len)
{
	if (!data) {
		return 0xFFFF;
	}

	return osal_crc16_ccitt_update(0xFFFF, data, len);
}
EXPORT_SYMBOL_GPL(osal_crc16_ccitt);

uint16_t osal_crc16_modbus(const uint8_t *data, size_t len)
{
	uint16_t crc = 0xFFFF;
	size_t i;
	uint8_t bit;

	if (!data) {
		return crc;
	}

	for (i = 0; i < len; i++) {
		crc ^= data[i];
		for (bit = 0; bit < 8; bit++) {
			if (crc & 0x0001) {
				crc = (crc >> 1) ^ 0xA001;
			}
			else {
				crc >>= 1;
			}
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

	if (!data) {
		return crc;
	}

	for (i = 0; i < len; i++) {
		crc ^= data[i];
		for (bit = 0; bit < 8; bit++) {
			if (crc & 1U) {
				crc = (crc >> 1) ^ 0xEDB88320U;
			}
			else {
				crc >>= 1;
			}
		}
	}

	return ~crc;
}
EXPORT_SYMBOL_GPL(osal_crc32);
