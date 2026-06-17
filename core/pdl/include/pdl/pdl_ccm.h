/************************************************************************
 * CCM 系统驱动对外API
 *
 * 功能：
 * - 封装以太网通信协议（PMC-CCM 协议）
 * - 处理与 CCM 的命令请求和响应
 * - 管理心跳和状态上报
 * - 提供统一的 CCM 交互接口
 *
 * 使用场景：
 * - PMC 产品中使用，用于与 CCM 通信
 ************************************************************************/

#ifndef PDL_CCM_H
#define PDL_CCM_H


/* CCM 状态码 */
typedef enum
{
	PDL_CCM_STATUS_OK = 0x00,
	PDL_CCM_STATUS_ERROR = 0x01,
	PDL_CCM_STATUS_BUSY = 0x02,
	PDL_CCM_STATUS_TIMEOUT = 0x03,
	PDL_CCM_STATUS_DISCONNECTED = 0x04
} pdl_ccm_status_t;

/* CCM 电源操作 */
typedef enum
{
	PDL_CCM_POWER_OFF = 0x00,
	PDL_CCM_POWER_ON = 0x01,
	PDL_CCM_POWER_QUERY = 0x02
} pdl_ccm_power_op_t;

/* CCM 节点操作 */
typedef enum
{
	PDL_CCM_NODE_QUERY = 0x00,
	PDL_CCM_NODE_START = 0x01,
	PDL_CCM_NODE_STOP = 0x02,
	PDL_CCM_NODE_RESTART = 0x03
} pdl_ccm_node_op_t;

/*
 * CCM 系统驱动句柄
 */
typedef void* pdl_ccm_handle_t;

/*
 * CCM 系统驱动配置
 *
 * 说明：直接嵌入网络配置，避免重复定义
 */
typedef struct
{
	/* 网络配置 */
	const char *ccm_ip;              /* CCM IP 地址 */
	uint16_t ccm_port;               /* CCM 端口 */
	uint32_t connect_timeout_ms;     /* 连接超时(ms) */
	uint32_t send_timeout_ms;        /* 发送超时(ms) */
	uint32_t recv_timeout_ms;        /* 接收超时(ms) */

	/* 业务配置 */
	uint32_t heartbeat_interval_ms;  /* 心跳间隔(ms) */
	uint32_t cmd_timeout_ms;         /* 命令超时(ms) */
	uint8_t max_retries;             /* 最大重试次数 */
} pdl_ccm_config_t;

/*
 * 遥测数据回调函数类型
 */
typedef void (*pdl_ccm_telemetry_callback_t)(uint32_t tm_id,
                                              uint32_t tm_source,
                                              const uint8_t *data,
                                              size_t len,
                                              void *user_data);

/*
 * 遥控指令回调函数类型
 */
typedef void (*pdl_ccm_command_callback_t)(uint32_t tc_id,
                                            uint32_t tc_target,
                                            uint32_t tc_action,
                                            const uint8_t *params,
                                            size_t params_len,
                                            void *user_data);

/**
 * @brief 初始化 CCM 系统驱动（旧接口，保持向后兼容）
 *
 * @param[in] config 配置参数
 * @param[out] handle 驱动句柄
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_GENERIC 失败
 */
int32_t PDL_CCM_init(const pdl_ccm_config_t *config,
                     pdl_ccm_handle_t *handle);

/**
 * @brief 初始化 CCM 系统驱动（从 PCONFIG 获取配置）
 *
 * @param[in] index CCM设备索引（从 PCONFIG 获取配置）
 * @param[out] handle 驱动句柄
 *
 * @return OSAL_SUCCESS 成功
 * @return OSAL_ERR_GENERIC 失败
 *
 * @note 函数内部会：
 *       1. 调用 PCONFIG_GetBoard() 获取平台配置
 *       2. 调用 PCONFIG_HW_GetCCM(platform, index) 获取 CCM 配置
 *       3. 检查配置是否启用
 *       4. 根据接口类型初始化通信（Ethernet/CAN）
 */
int32_t PDL_CCM_init_from_pconfig(uint32_t index, pdl_ccm_handle_t *handle);

/**
 * @brief 反初始化 CCM 系统驱动
 *
 * @param[in] handle 驱动句柄
 *
 * @return OSAL_SUCCESS 成功
 */
int32_t PDL_CCM_deinit(pdl_ccm_handle_t handle);

/**
 * @brief 注册遥测数据回调函数
 *
 * @param[in] handle 驱动句柄
 * @param[in] callback 回调函数
 * @param[in] user_data 用户数据
 *
 * @return OSAL_SUCCESS 成功
 */
int32_t PDL_CCM_register_telemetry_callback(pdl_ccm_handle_t handle,
                                          pdl_ccm_telemetry_callback_t callback,
                                          void *user_data);

/**
 * @brief 注册遥控指令回调函数
 *
 * @param[in] handle 驱动句柄
 * @param[in] callback 回调函数
 * @param[in] user_data 用户数据
 *
 * @return OSAL_SUCCESS 成功
 */
int32_t PDL_CCM_register_command_callback(pdl_ccm_handle_t handle,
                                         pdl_ccm_command_callback_t callback,
                                         void *user_data);

/**
 * @brief 获取驱动统计信息
 *
 * @param[in] handle 驱动句柄
 * @param[out] rx_count 接收计数
 * @param[out] tx_count 发送计数
 * @param[out] error_count 错误计数
 *
 * @return OSAL_SUCCESS 成功
 */
int32_t PDL_CCM_GetStats(pdl_ccm_handle_t handle,
                          uint32_t *rx_count,
                          uint32_t *tx_count,
                          uint32_t *error_count);

/**
 * @brief 获取连接状态
 *
 * @param[in] handle 驱动句柄
 * @param[out] connected 是否连接
 * @param[out] link_quality 链路质量（0-100）
 *
 * @return OSAL_SUCCESS 成功
 */
int32_t PDL_CCM_GetConnectionStatus(pdl_ccm_handle_t handle,
                                     bool *connected,
                                     uint32_t *link_quality);

#endif /* PDL_CCM_H */
