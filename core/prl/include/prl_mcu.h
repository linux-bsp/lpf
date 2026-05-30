/**
 * @file prl_mcu.h
 * @brief MCU Communication Protocol
 * @details MCU 通信协议定义，支持 CAN 和串口两种传输方式
 *          可被 CCM、PMC 等多个设备复用
 */

#ifndef PRL_MCU_H
#define PRL_MCU_H

#include "prl_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MCU 协议版本 */
#define PRL_MCU_VERSION         0x0100

/* MCU 命令码定义 */
typedef enum {
    PRL_MCU_CMD_GET_VERSION     = 0x01,     /* 获取版本 */
    PRL_MCU_CMD_GET_STATUS      = 0x02,     /* 获取状态 */
    PRL_MCU_CMD_RESET           = 0x03,     /* 复位 */
    PRL_MCU_CMD_READ_REG        = 0x10,     /* 读寄存器 */
    PRL_MCU_CMD_WRITE_REG       = 0x11,     /* 写寄存器 */
    PRL_MCU_CMD_POWER_ON        = 0x20,     /* 上电 */
    PRL_MCU_CMD_POWER_OFF       = 0x21,     /* 下电 */
    PRL_MCU_CMD_CUSTOM_START    = 0x30,     /* 自定义命令起始 */
    PRL_MCU_CMD_CUSTOM_END      = 0xFE,     /* 自定义命令结束 */
} prl_mcu_cmd_t;

/* MCU 传输方式 */
typedef enum {
    PRL_MCU_TRANSPORT_CAN       = 0,        /* CAN 总线 */
    PRL_MCU_TRANSPORT_SERIAL    = 1,        /* 串口 */
    PRL_MCU_TRANSPORT_I2C       = 2,        /* I2C（预留） */
    PRL_MCU_TRANSPORT_SPI       = 3,        /* SPI（预留） */
} prl_mcu_transport_t;

/* MCU 版本信息 */
typedef struct {
    uint8_t major;              /* 主版本号 */
    uint8_t minor;              /* 次版本号 */
    uint8_t patch;              /* 补丁版本号 */
    uint8_t build;              /* 构建版本号 */
} __attribute__((packed)) prl_mcu_version_t;

/* MCU 状态信息 */
typedef struct {
    uint8_t  online;            /* 在线状态：0离线 1在线 */
    uint8_t  error_code;        /* 错误码 */
    uint16_t voltage_mv;        /* 电压（毫伏） */
    uint32_t uptime_sec;        /* 运行时间（秒） */
    int16_t  temperature;       /* 温度（0.1℃，如 256 表示 25.6℃） */
    uint16_t reserved;          /* 保留 */
} __attribute__((packed)) prl_mcu_status_t;

/* MCU 寄存器读写 */
typedef struct {
    uint8_t reg_addr;           /* 寄存器地址 */
    uint8_t value;              /* 寄存器值 */
} __attribute__((packed)) prl_mcu_register_t;

/* MCU 通用请求 */
typedef struct {
    uint8_t  cmd_code;          /* 命令码 */
    uint8_t  data_len;          /* 数据长度 */
    uint8_t  data[254];         /* 数据（最大254字节） */
} __attribute__((packed)) prl_mcu_request_t;

/* MCU 通用响应 */
typedef struct {
    uint8_t  status;            /* 状态码：0成功，非0失败 */
    uint8_t  data_len;          /* 数据长度 */
    uint8_t  data[254];         /* 数据（最大254字节） */
} __attribute__((packed)) prl_mcu_response_t;

/* ========== CAN 帧格式编解码 ========== */

/**
 * @brief 编码 CAN 请求帧
 * @details CAN 帧格式：[cmd_code][data_len][data...] (最多8字节)
 *
 * @param cmd_code 命令码
 * @param data 数据指针（可为 NULL）
 * @param data_len 数据长度（最大6字节，因为 CAN 帧限制）
 * @param frame 输出帧缓冲区（至少8字节）
 * @param frame_len 输出帧长度
 * @return PRL_OK 成功，其他值表示错误
 */
int prl_mcu_can_encode_request(uint8_t cmd_code,
                                const uint8_t *data,
                                uint8_t data_len,
                                uint8_t *frame,
                                uint8_t *frame_len);

/**
 * @brief 解码 CAN 响应帧
 * @details CAN 帧格式：[status][data_len][data...] (最多8字节)
 *
 * @param frame 输入帧缓冲区
 * @param frame_len 输入帧长度
 * @param status 输出状态码
 * @param data 输出数据缓冲区（可为 NULL）
 * @param data_size 数据缓冲区大小
 * @param actual_len 实际数据长度
 * @return PRL_OK 成功，其他值表示错误
 */
int prl_mcu_can_decode_response(const uint8_t *frame,
                                 uint8_t frame_len,
                                 uint8_t *status,
                                 uint8_t *data,
                                 uint8_t data_size,
                                 uint8_t *actual_len);

/* ========== 串口帧格式编解码 ========== */

/**
 * @brief 编码串口请求帧
 * @details 串口帧格式：[0xAA][0x55][cmd][len][data...][crc16_h][crc16_l]
 *
 * @param cmd_code 命令码
 * @param data 数据指针（可为 NULL）
 * @param data_len 数据长度（最大254字节）
 * @param enable_crc 是否启用 CRC 校验
 * @param frame 输出帧缓冲区（至少 data_len + 6 字节）
 * @param frame_size 帧缓冲区大小
 * @param actual_len 实际帧长度
 * @return PRL_OK 成功，其他值表示错误
 */
int prl_mcu_serial_encode_request(uint8_t cmd_code,
                                   const uint8_t *data,
                                   uint8_t data_len,
                                   bool enable_crc,
                                   uint8_t *frame,
                                   size_t frame_size,
                                   size_t *actual_len);

/**
 * @brief 解码串口响应帧
 * @details 串口帧格式：[0xAA][0x55][status][len][data...][crc16_h][crc16_l]
 *
 * @param frame 输入帧缓冲区
 * @param frame_len 输入帧长度
 * @param enable_crc 是否启用 CRC 校验
 * @param status 输出状态码
 * @param data 输出数据缓冲区（可为 NULL）
 * @param data_size 数据缓冲区大小
 * @param actual_len 实际数据长度
 * @return PRL_OK 成功，其他值表示错误
 */
int prl_mcu_serial_decode_response(const uint8_t *frame,
                                    size_t frame_len,
                                    bool enable_crc,
                                    uint8_t *status,
                                    uint8_t *data,
                                    size_t data_size,
                                    size_t *actual_len);

/* ========== 高层消息编解码（传输无关） ========== */

/**
 * @brief 编码版本查询请求
 * @param req 输出请求结构
 * @return PRL_OK 成功
 */
int prl_mcu_encode_get_version(prl_mcu_request_t *req);

/**
 * @brief 解码版本查询响应
 * @param resp 输入响应结构
 * @param version 输出版本信息
 * @return PRL_OK 成功，其他值表示错误
 */
int prl_mcu_decode_get_version(const prl_mcu_response_t *resp,
                                prl_mcu_version_t *version);

/**
 * @brief 编码状态查询请求
 * @param req 输出请求结构
 * @return PRL_OK 成功
 */
int prl_mcu_encode_get_status(prl_mcu_request_t *req);

/**
 * @brief 解码状态查询响应
 * @param resp 输入响应结构
 * @param status 输出状态信息
 * @return PRL_OK 成功，其他值表示错误
 */
int prl_mcu_decode_get_status(const prl_mcu_response_t *resp,
                               prl_mcu_status_t *status);

/**
 * @brief 编码复位请求
 * @param req 输出请求结构
 * @return PRL_OK 成功
 */
int prl_mcu_encode_reset(prl_mcu_request_t *req);

/**
 * @brief 编码寄存器读取请求
 * @param reg_addr 寄存器地址
 * @param req 输出请求结构
 * @return PRL_OK 成功
 */
int prl_mcu_encode_read_register(uint8_t reg_addr, prl_mcu_request_t *req);

/**
 * @brief 解码寄存器读取响应
 * @param resp 输入响应结构
 * @param reg 输出寄存器信息
 * @return PRL_OK 成功，其他值表示错误
 */
int prl_mcu_decode_read_register(const prl_mcu_response_t *resp,
                                  prl_mcu_register_t *reg);

/**
 * @brief 编码寄存器写入请求
 * @param reg_addr 寄存器地址
 * @param value 写入值
 * @param req 输出请求结构
 * @return PRL_OK 成功
 */
int prl_mcu_encode_write_register(uint8_t reg_addr,
                                   uint8_t value,
                                   prl_mcu_request_t *req);

/**
 * @brief 编码自定义命令请求
 * @param cmd_code 命令码（0x30-0xFE）
 * @param data 命令数据
 * @param data_len 数据长度
 * @param req 输出请求结构
 * @return PRL_OK 成功，其他值表示错误
 */
int prl_mcu_encode_custom_command(uint8_t cmd_code,
                                   const uint8_t *data,
                                   uint8_t data_len,
                                   prl_mcu_request_t *req);

#ifdef __cplusplus
}
#endif

#endif /* PRL_MCU_H */
