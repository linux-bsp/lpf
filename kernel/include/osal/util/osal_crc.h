/************************************************************************
 * Kernel OSAL CRC API
 ************************************************************************/

#ifndef OSAL_CRC_H
#define OSAL_CRC_H

#include "osal_types.h"

uint16_t osal_crc16_ccitt(const uint8_t *data, size_t len);
uint16_t osal_crc16_ccitt_update(uint16_t crc, const uint8_t *data, size_t len);
uint16_t osal_crc16_modbus(const uint8_t *data, size_t len);
uint32_t osal_crc32(const uint8_t *data, size_t len);

#endif /* OSAL_CRC_H */
