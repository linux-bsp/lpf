/**
 * @file prl_device.h
 * @brief Protocol Layer Device Messages
 * @details 统一设备协议消息定义，所有设备共用相同的报文头和编解码逻辑
 */

#ifndef PRL_DEVICE_H
#define PRL_DEVICE_H

#include "prl_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========== MCU 消息类型 ========== */
typedef enum {
    PRL_MCU_MSG_GET_VERSION     = 0x01,     /* 获取版本信息 */
    PRL_MCU_MSG_GET_STATUS      = 0x02,     /* 获取状态 */
    PRL_MCU_MSG_RESET           = 0x03,     /* 复位 */
    PRL_MCU_MSG_HEARTBEAT       = 0x04,     /* 心跳 */
    PRL_MCU_MSG_SET_CONFIG      = 0x05,     /* 设置配置 */
    PRL_MCU_MSG_GET_CONFIG      = 0x06,     /* 获取配置 */
    PRL_MCU_MSG_POWER_ON        = 0x20,     /* 上电 */
    PRL_MCU_MSG_POWER_OFF       = 0x21,     /* 下电 */
    PRL_MCU_MSG_CUSTOM          = 0xFF,     /* 自定义命令 */
} prl_mcu_msg_type_t;

/* MCU 版本信息 */
typedef struct {
    uint8_t  major;             /* 主版本号 */
    uint8_t  minor;             /* 次版本号 */
    uint8_t  patch;             /* 补丁版本号 */
    uint8_t  reserved;          /* 保留 */
    uint32_t build_time;        /* 编译时间戳 */
    char     git_hash[8];       /* Git 提交哈希（前8字节） */
} prl_mcu_version_t;

/* MCU 状态信息 */
typedef struct {
    uint8_t  state;             /* 运行状态 */
    uint8_t  error_code;        /* 错误码 */
    uint16_t uptime;            /* 运行时间（秒） */
    uint32_t cpu_usage;         /* CPU 使用率（千分比） */
    uint32_t mem_usage;         /* 内存使用率（千分比） */
} prl_mcu_status_t;

/* ========== CCM 消息类型 ========== */
typedef enum {
    PRL_CCM_MSG_HEARTBEAT       = 0x01,     /* 心跳 */
    PRL_CCM_MSG_TELEMETRY       = 0x02,     /* 遥测数据 */
    PRL_CCM_MSG_COMMAND         = 0x03,     /* 遥控指令 */
    PRL_CCM_MSG_TIME_SYNC       = 0x04,     /* 时间同步 */
    PRL_CCM_MSG_ORBIT_DATA      = 0x05,     /* 轨道数据 */
    PRL_CCM_MSG_ATTITUDE_DATA   = 0x06,     /* 姿态数据 */
    PRL_CCM_MSG_POWER_STATUS    = 0x07,     /* 电源状态 */
    PRL_CCM_MSG_THERMAL_STATUS  = 0x08,     /* 热控状态 */
    PRL_CCM_MSG_ACK             = 0xFF,     /* 应答 */
} prl_ccm_msg_type_t;

/* CCM 状态信息 */
typedef struct {
    uint8_t  link_status;       /* 链路状态 */
    uint8_t  signal_strength;   /* 信号强度 */
    uint16_t error_count;       /* 错误计数 */
    uint32_t tx_bytes;          /* 发送字节数 */
    uint32_t rx_bytes;          /* 接收字节数 */
} prl_ccm_status_t;

/* ========== PMC 消息类型 ========== */
typedef enum {
    PRL_PMC_MSG_HEARTBEAT       = 0x01,     /* 心跳 */
    PRL_PMC_MSG_TELEMETRY       = 0x02,     /* 遥测数据 */
    PRL_PMC_MSG_COMMAND         = 0x03,     /* 遥控指令 */
    PRL_PMC_MSG_FIRMWARE_UPDATE = 0x04,     /* 固件升级 */
    PRL_PMC_MSG_NODE_MANAGE     = 0x05,     /* 节点管理 */
    PRL_PMC_MSG_POWER_CONTROL   = 0x06,     /* 电源控制 */
    PRL_PMC_MSG_STATUS_QUERY    = 0x07,     /* 状态查询 */
    PRL_PMC_MSG_ACK             = 0xFF,     /* 应答 */
} prl_pmc_msg_type_t;

/* PMC 遥测数据 */
typedef struct {
    uint32_t timestamp;         /* 时间戳 */
    int16_t  temperature;       /* 温度（0.1°C） */
    uint16_t voltage;           /* 电压（mV） */
    uint16_t current;           /* 电流（mA） */
    uint16_t power;             /* 功率（mW） */
} prl_pmc_telemetry_t;

/* ========== GSC 消息类型 ========== */
typedef enum {
    PRL_GSC_MSG_HEARTBEAT       = 0x01,     /* 心跳 */
    PRL_GSC_MSG_TELEMETRY       = 0x02,     /* 遥测数据 */
    PRL_GSC_MSG_COMMAND         = 0x03,     /* 遥控指令 */
    PRL_GSC_MSG_FILE_TRANSFER   = 0x04,     /* 文件传输 */
    PRL_GSC_MSG_DATABASE_SYNC   = 0x05,     /* 数据库同步 */
    PRL_GSC_MSG_LOG_UPLOAD      = 0x06,     /* 日志上传 */
    PRL_GSC_MSG_ACK             = 0xFF,     /* 应答 */
} prl_gsc_msg_type_t;

/* GSC 状态信息 */
typedef struct {
    uint8_t  connection_status; /* 连接状态 */
    uint8_t  command_queue_len; /* 指令队列长度 */
    uint16_t last_error;        /* 最后错误码 */
    uint32_t uptime;            /* 运行时间（秒） */
} prl_gsc_status_t;

/* ========== SATELLITE 消息类型 ========== */
typedef enum {
    PRL_SAT_MSG_HEARTBEAT       = 0x01,     /* 心跳 */
    PRL_SAT_MSG_TELEMETRY       = 0x02,     /* 遥测数据 */
    PRL_SAT_MSG_TELECOMMAND     = 0x03,     /* 遥控指令 */
    PRL_SAT_MSG_TIME_SYNC       = 0x04,     /* 时间同步 */
    PRL_SAT_MSG_ORBIT_DATA      = 0x05,     /* 轨道数据 */
    PRL_SAT_MSG_ATTITUDE_DATA   = 0x06,     /* 姿态数据 */
    PRL_SAT_MSG_POWER_STATUS    = 0x07,     /* 电源状态 */
    PRL_SAT_MSG_THERMAL_STATUS  = 0x08,     /* 热控状态 */
    PRL_SAT_MSG_ACK             = 0xFF,     /* 应答 */
} prl_sat_msg_type_t;

/* ========== POWER 消息类型 ========== */
typedef enum {
    PRL_POWER_MSG_HEARTBEAT     = 0x01,     /* 心跳 */
    PRL_POWER_MSG_POWER_ON      = 0x02,     /* 上电 */
    PRL_POWER_MSG_POWER_OFF     = 0x03,     /* 下电 */
    PRL_POWER_MSG_VOLTAGE_QUERY = 0x04,     /* 电压查询 */
    PRL_POWER_MSG_CURRENT_QUERY = 0x05,     /* 电流查询 */
    PRL_POWER_MSG_TEMP_QUERY    = 0x06,     /* 温度查询 */
    PRL_POWER_MSG_STATUS_REPORT = 0x07,     /* 状态上报 */
    PRL_POWER_MSG_ALARM         = 0x08,     /* 告警 */
    PRL_POWER_MSG_ACK           = 0xFF,     /* 应答 */
} prl_power_msg_type_t;

/* 电源状态 */
typedef struct {
    uint8_t  enabled;           /* 使能状态 */
    uint8_t  fault;             /* 故障标志 */
    uint16_t voltage;           /* 输出电压（mV） */
    uint16_t current;           /* 输出电流（mA） */
    uint16_t temperature;       /* 温度（0.1°C） */
} prl_power_status_t;

/* ========== BMC 消息类型 ========== */
typedef enum {
    PRL_BMC_MSG_GET_SENSOR      = 0x01,     /* 获取传感器数据 */
    PRL_BMC_MSG_GET_FAN_SPEED   = 0x02,     /* 获取风扇转速 */
    PRL_BMC_MSG_SET_FAN_SPEED   = 0x03,     /* 设置风扇转速 */
    PRL_BMC_MSG_GET_POWER       = 0x04,     /* 获取电源状态 */
    PRL_BMC_MSG_RESET_SYSTEM    = 0x05,     /* 系统复位 */
} prl_bmc_msg_type_t;

/* ========== 内部编解码接口（仅供 PRL 内部使用） ========== */

/**
 * @brief 编码设备消息（内部函数）
 * @param dev_type 设备类型
 * @param msg_type 消息类型
 * @param payload 负载数据
 * @param payload_len 负载长度
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @param flags 标志位
 * @return 成功返回编码后的总长度，失败返回负数错误码
 * @note 内部函数，外部请使用 PRL_Encode()
 */
int prl_device_encode(uint8_t dev_type, uint8_t msg_type,
                      const void *payload, uint16_t payload_len,
                      uint8_t *buffer, size_t buffer_size, uint8_t flags);

/**
 * @brief 解码设备消息（内部函数）
 * @param packet 报文数据
 * @param packet_len 报文长度
 * @param dev_type 输出：设备类型
 * @param msg_type 输出：消息类型
 * @param payload 输出：负载数据指针（指向 packet 内部，零拷贝）
 * @param payload_len 输出：负载长度
 * @return 成功返回 PRL_OK，失败返回负数错误码
 * @note 内部函数，外部请使用 PRL_Decode()
 */
int prl_device_decode(const uint8_t *packet, size_t packet_len,
                      uint8_t *dev_type, uint8_t *msg_type,
                      const uint8_t **payload, uint16_t *payload_len);

/**
 * @brief 验证设备类型是否有效（内部函数）
 * @param dev_type 设备类型
 * @return 有效返回 true，无效返回 false
 * @note 内部函数，外部请使用 PRL_IsDeviceTypeValid()
 */
bool prl_device_type_valid(uint8_t dev_type);

/**
 * @brief 获取设备类型名称（内部函数）
 * @param dev_type 设备类型
 * @return 设备类型名称字符串
 * @note 内部函数，外部请使用 PRL_GetDeviceTypeName()
 */
const char *prl_device_type_name(uint8_t dev_type);

#ifdef __cplusplus
}
#endif

#endif /* PRL_DEVICE_H */
