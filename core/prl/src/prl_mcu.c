/**
 * @file prl_mcu.c
 * @brief MCU Communication Protocol Implementation
 * @details MCU 通信协议实现，支持 CAN 和串口两种传输方式
 */

#include "prl_mcu.h"
#include "osal_string.h"

/* 串口帧头定义 */
#define SERIAL_FRAME_HEADER_0       0xAA
#define SERIAL_FRAME_HEADER_1       0x55
#define SERIAL_FRAME_HEADER_SIZE    2
#define SERIAL_FRAME_CRC_SIZE       2
#define SERIAL_FRAME_OVERHEAD       (SERIAL_FRAME_HEADER_SIZE + 2 + SERIAL_FRAME_CRC_SIZE)

/* CAN 帧最大长度 */
#define CAN_FRAME_MAX_LEN           8
#define CAN_FRAME_DATA_MAX_LEN      6   /* 8 - cmd(1) - len(1) */

/* ========== CRC16 计算（MODBUS 标准） ========== */

/**
 * @brief 计算 CRC16 校验（MODBUS 标准）
 * @details 多项式：0xA001，初始值：0xFFFF
 */
static uint16_t prl_mcu_calc_crc16(const uint8_t *data, size_t len)
{
    uint16_t crc = 0xFFFF;
    size_t i;
    int j;

    for (i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                crc = (crc >> 1) ^ 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}

/* ========== CAN 帧格式编解码 ========== */

int prl_mcu_can_encode_request(uint8_t cmd_code,
                                const uint8_t *data,
                                uint8_t data_len,
                                uint8_t *frame,
                                uint8_t *frame_len)
{
    uint8_t pos = 0;
    uint8_t copy_len;

    if (!frame || !frame_len)
    {
        return PRL_ERR_INVALID_PARAM;
    }

    /* 检查数据长度（CAN 帧最多 8 字节，留 2 字节给头） */
    if (data_len > CAN_FRAME_DATA_MAX_LEN)
    {
        return PRL_ERR_INVALID_LENGTH;
    }

    /* 封装 CAN 帧：[cmd_code][data_len][data...] */
    frame[pos++] = cmd_code;
    frame[pos++] = data_len;

    if (data && data_len > 0)
    {
        copy_len = (data_len > CAN_FRAME_DATA_MAX_LEN) ? CAN_FRAME_DATA_MAX_LEN : data_len;
        OSAL_Memcpy(&frame[pos], data, copy_len);
        pos += copy_len;
    }

    *frame_len = pos;
    return PRL_OK;
}

int prl_mcu_can_decode_response(const uint8_t *frame,
                                 uint8_t frame_len,
                                 uint8_t *status,
                                 uint8_t *data,
                                 uint8_t data_size,
                                 uint8_t *actual_len)
{
    uint8_t resp_data_len;
    uint8_t copy_len;

    if (!frame || !status)
    {
        return PRL_ERR_INVALID_PARAM;
    }

    /* 最小帧长度检查 */
    if (frame_len < 2)
    {
        return PRL_ERR_INVALID_LENGTH;
    }

    /* 解析响应：[status][data_len][data...] */
    *status = frame[0];
    resp_data_len = frame[1];

    /* 检查数据长度一致性 */
    if (frame_len < (uint8_t)(2 + resp_data_len))
    {
        return PRL_ERR_INVALID_LENGTH;
    }

    /* 复制数据 */
    if (data && resp_data_len > 0)
    {
        copy_len = (resp_data_len < data_size) ? resp_data_len : data_size;
        copy_len = (copy_len < (frame_len - 2)) ? copy_len : (frame_len - 2);
        OSAL_Memcpy(data, &frame[2], copy_len);

        if (actual_len)
        {
            *actual_len = copy_len;
        }
    }
    else if (actual_len)
    {
        *actual_len = 0;
    }

    return PRL_OK;
}

/* ========== 串口帧格式编解码 ========== */

int prl_mcu_serial_encode_request(uint8_t cmd_code,
                                   const uint8_t *data,
                                   uint8_t data_len,
                                   bool enable_crc,
                                   uint8_t *frame,
                                   size_t frame_size,
                                   size_t *actual_len)
{
    size_t required_size;
    size_t pos = 0;
    uint16_t crc;

    if (!frame || !actual_len)
    {
        return PRL_ERR_INVALID_PARAM;
    }

    /* 计算所需缓冲区大小 */
    required_size = SERIAL_FRAME_OVERHEAD + data_len;
    if (frame_size < required_size)
    {
        return PRL_ERR_BUFFER_TOO_SMALL;
    }

    /* 帧头 */
    frame[pos++] = SERIAL_FRAME_HEADER_0;
    frame[pos++] = SERIAL_FRAME_HEADER_1;

    /* 命令和长度 */
    frame[pos++] = cmd_code;
    frame[pos++] = data_len;

    /* 数据 */
    if (data && data_len > 0)
    {
        OSAL_Memcpy(&frame[pos], data, data_len);
        pos += data_len;
    }

    /* CRC 校验 */
    if (enable_crc)
    {
        /* CRC 计算范围：从 cmd 开始到数据结束（不包括帧头） */
        crc = prl_mcu_calc_crc16(&frame[SERIAL_FRAME_HEADER_SIZE],
                                  pos - SERIAL_FRAME_HEADER_SIZE);
        frame[pos++] = (uint8_t)(crc >> 8);
        frame[pos++] = (uint8_t)(crc & 0xFF);
    }
    else
    {
        frame[pos++] = 0;
        frame[pos++] = 0;
    }

    *actual_len = pos;
    return PRL_OK;
}

int prl_mcu_serial_decode_response(const uint8_t *frame,
                                    size_t frame_len,
                                    bool enable_crc,
                                    uint8_t *status,
                                    uint8_t *data,
                                    size_t data_size,
                                    size_t *actual_len)
{
    uint16_t crc_recv;
    uint16_t crc_calc;
    uint8_t resp_data_len;
    size_t copy_len;

    if (!frame || !status)
    {
        return PRL_ERR_INVALID_PARAM;
    }

    /* 最小帧长度检查 */
    if (frame_len < SERIAL_FRAME_OVERHEAD)
    {
        return PRL_ERR_INVALID_LENGTH;
    }

    /* 帧头检查 */
    if (frame[0] != SERIAL_FRAME_HEADER_0 || frame[1] != SERIAL_FRAME_HEADER_1)
    {
        return PRL_ERR_INVALID_MAGIC;
    }

    /* CRC 校验 */
    if (enable_crc)
    {
        crc_recv = ((uint16_t)frame[frame_len - 2] << 8) | frame[frame_len - 1];
        crc_calc = prl_mcu_calc_crc16(&frame[SERIAL_FRAME_HEADER_SIZE],
                                       frame_len - SERIAL_FRAME_OVERHEAD);
        if (crc_recv != crc_calc)
        {
            return PRL_ERR_CRC_FAILED;
        }
    }

    /* 解析状态和数据 */
    *status = frame[2];
    resp_data_len = frame[3];

    /* 检查数据长度一致性 */
    if (frame_len < (SERIAL_FRAME_OVERHEAD + resp_data_len))
    {
        return PRL_ERR_INVALID_LENGTH;
    }

    /* 复制数据 */
    if (data && resp_data_len > 0)
    {
        copy_len = (resp_data_len < data_size) ? resp_data_len : data_size;
        OSAL_Memcpy(data, &frame[4], copy_len);
        if (actual_len)
        {
            *actual_len = copy_len;
        }
    }
    else if (actual_len)
    {
        *actual_len = 0;
    }

    return PRL_OK;
}

/* ========== 高层消息编解码（传输无关） ========== */

int prl_mcu_encode_get_version(prl_mcu_request_t *req)
{
    if (!req)
    {
        return PRL_ERR_INVALID_PARAM;
    }

    req->cmd_code = PRL_MCU_CMD_GET_VERSION;
    req->data_len = 0;

    return PRL_OK;
}

int prl_mcu_decode_get_version(const prl_mcu_response_t *resp,
                                prl_mcu_version_t *version)
{
    if (!resp || !version)
    {
        return PRL_ERR_INVALID_PARAM;
    }

    if (resp->status != 0)
    {
        return PRL_ERR_DECODE_FAILED;
    }

    if (resp->data_len < sizeof(prl_mcu_version_t))
    {
        return PRL_ERR_INVALID_LENGTH;
    }

    OSAL_Memcpy(version, resp->data, sizeof(prl_mcu_version_t));

    return PRL_OK;
}

int prl_mcu_encode_get_status(prl_mcu_request_t *req)
{
    if (!req)
    {
        return PRL_ERR_INVALID_PARAM;
    }

    req->cmd_code = PRL_MCU_CMD_GET_STATUS;
    req->data_len = 0;

    return PRL_OK;
}

int prl_mcu_decode_get_status(const prl_mcu_response_t *resp,
                               prl_mcu_status_t *status)
{
    if (!resp || !status)
    {
        return PRL_ERR_INVALID_PARAM;
    }

    if (resp->status != 0)
    {
        return PRL_ERR_DECODE_FAILED;
    }

    if (resp->data_len < sizeof(prl_mcu_status_t))
    {
        return PRL_ERR_INVALID_LENGTH;
    }

    OSAL_Memcpy(status, resp->data, sizeof(prl_mcu_status_t));

    return PRL_OK;
}

int prl_mcu_encode_reset(prl_mcu_request_t *req)
{
    if (!req)
    {
        return PRL_ERR_INVALID_PARAM;
    }

    req->cmd_code = PRL_MCU_CMD_RESET;
    req->data_len = 0;

    return PRL_OK;
}

int prl_mcu_encode_read_register(uint8_t reg_addr, prl_mcu_request_t *req)
{
    if (!req)
    {
        return PRL_ERR_INVALID_PARAM;
    }

    req->cmd_code = PRL_MCU_CMD_READ_REG;
    req->data_len = 1;
    req->data[0] = reg_addr;

    return PRL_OK;
}

int prl_mcu_decode_read_register(const prl_mcu_response_t *resp,
                                  prl_mcu_register_t *reg)
{
    if (!resp || !reg)
    {
        return PRL_ERR_INVALID_PARAM;
    }

    if (resp->status != 0)
    {
        return PRL_ERR_DECODE_FAILED;
    }

    if (resp->data_len < sizeof(prl_mcu_register_t))
    {
        return PRL_ERR_INVALID_LENGTH;
    }

    OSAL_Memcpy(reg, resp->data, sizeof(prl_mcu_register_t));

    return PRL_OK;
}

int prl_mcu_encode_write_register(uint8_t reg_addr,
                                   uint8_t value,
                                   prl_mcu_request_t *req)
{
    if (!req)
    {
        return PRL_ERR_INVALID_PARAM;
    }

    req->cmd_code = PRL_MCU_CMD_WRITE_REG;
    req->data_len = 2;
    req->data[0] = reg_addr;
    req->data[1] = value;

    return PRL_OK;
}

int prl_mcu_encode_custom_command(uint8_t cmd_code,
                                   const uint8_t *data,
                                   uint8_t data_len,
                                   prl_mcu_request_t *req)
{
    if (!req)
    {
        return PRL_ERR_INVALID_PARAM;
    }

    /* 检查命令码范围 */
    if (cmd_code < PRL_MCU_CMD_CUSTOM_START || cmd_code > PRL_MCU_CMD_CUSTOM_END)
    {
        return PRL_ERR_INVALID_PARAM;
    }

    /* 检查数据长度 */
    if (data_len > sizeof(req->data))
    {
        return PRL_ERR_INVALID_LENGTH;
    }

    req->cmd_code = cmd_code;
    req->data_len = data_len;

    if (data && data_len > 0)
    {
        OSAL_Memcpy(req->data, data, data_len);
    }

    return PRL_OK;
}
