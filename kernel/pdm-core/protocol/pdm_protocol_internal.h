// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_PROTOCOL_INTERNAL_H
#define PDM_PROTOCOL_INTERNAL_H

#include "pdm/protocol/pdm_protocol.h"

uint16_t pdm_protocol_crc16(const uint8_t *data, uint16_t len);
uint32_t pdm_protocol_get_next_seq(void);
uint32_t pdm_protocol_get_timestamp(void);
void pdm_protocol_reset_sequence(uint32_t seq);
uint32_t pdm_protocol_get_current_sequence(void);
void pdm_protocol_init_header(pdm_protocol_header_t *hdr, uint8_t dev_type,
			      uint8_t msg_type, uint16_t payload_len,
			      uint8_t flags);
int pdm_protocol_validate_header(const pdm_protocol_header_t *hdr,
				 uint8_t expected_dev_type);
void pdm_protocol_set_packet_crc(uint8_t *packet, size_t total_len);
bool pdm_protocol_verify_packet_crc(const uint8_t *packet, size_t total_len);
bool pdm_protocol_is_device_type_valid(uint8_t dev_type);
const char *pdm_protocol_get_device_type_name(uint8_t dev_type);
const char *pdm_protocol_get_error_string(int32_t error_code);
void pdm_protocol_get_version(uint8_t *major, uint8_t *minor);

#endif /* PDM_PROTOCOL_INTERNAL_H */
