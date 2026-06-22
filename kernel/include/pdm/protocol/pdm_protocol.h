/**
 * @file pdm_protocol.h
 * @brief PDM standard peripheral protocol entry point
 */

#ifndef PDM_PROTOCOL_H
#define PDM_PROTOCOL_H

#include "osal.h"

#define PDM_PROTOCOL_MAGIC 0xAA55
#define PDM_PROTOCOL_VERSION 0x01
#define PDM_PROTOCOL_VERSION_MAJOR 0x00
#define PDM_PROTOCOL_VERSION_MINOR 0x01
#define PDM_PROTOCOL_HEADER_SIZE 20

#ifdef CONFIG_PDM_PROTOCOL_MAX_PAYLOAD_SIZE
#define PDM_PROTOCOL_MAX_PAYLOAD_SIZE CONFIG_PDM_PROTOCOL_MAX_PAYLOAD_SIZE
#else
#define PDM_PROTOCOL_MAX_PAYLOAD_SIZE 1024
#endif

#define PDM_PROTOCOL_MAX_PACKET_SIZE \
	(PDM_PROTOCOL_HEADER_SIZE + PDM_PROTOCOL_MAX_PAYLOAD_SIZE)

typedef enum {
	PDM_PROTOCOL_DEV_TYPE_INVALID = 0x00,
	PDM_PROTOCOL_DEV_TYPE_MCU = 0x01,
	PDM_PROTOCOL_DEV_TYPE_FPGA = 0x02,
} pdm_protocol_dev_type_t;

typedef struct {
	uint16_t magic;
	uint8_t version;
	uint8_t dev_type;
	uint8_t msg_type;
	uint8_t flags;
	uint16_t length;
	uint32_t seq;
	uint32_t timestamp;
	uint16_t crc16;
	uint16_t reserved;
} __attribute__((packed)) pdm_protocol_header_t;

#include "pdm/protocol/pdm_protocol_mcu.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint8_t dev_type;
	uint8_t msg_type;
	uint8_t flags;
	const void *payload;
	uint16_t payload_len;
	uint8_t *buffer;
	size_t buffer_size;
} pdm_protocol_encode_ctx_t;

typedef struct {
	const uint8_t *buffer;
	size_t buffer_len;
	uint8_t dev_type;
	uint8_t msg_type;
	uint8_t flags;
	void *payload;
	size_t payload_size;
	uint32_t payload_len;
} pdm_protocol_decode_ctx_t;

int pdm_protocol_encode(pdm_protocol_encode_ctx_t *ctx);
int pdm_protocol_decode(pdm_protocol_decode_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* PDM_PROTOCOL_H */
