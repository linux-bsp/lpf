/************************************************************************
 * CCM 系统驱动实现
 *
 * 职责：
 * - 实现对外业务接口
 * - 管理心跳和消息处理任务
 * - 调度内部以太网通信模块
 ************************************************************************/

#include "osal.h"
#include "prl.h"
#include "prl_pmc.h"  /* PMC protocol definitions */
#include "pdl.h"
#include "pdl_ccm_internal.h"

/*
 * CCM 系统驱动上下文
 */
typedef struct
{
    pdl_ccm_config_t config;
    void *eth_handle;                 /* 以太网通信句柄 */

    /* 回调函数 */
    pdl_ccm_telemetry_callback_t tm_callback;
    void *tm_user_data;
    pdl_ccm_command_callback_t tc_callback;
    void *tc_user_data;

    /* 统计信息 */
    uint32_t rx_count;
    uint32_t tx_count;
    uint32_t error_count;
    uint32_t link_quality;

    /* 线程控制 */
    osal_thread_t rx_thread;
    osal_thread_t heartbeat_thread;
    osal_atomic_bool_t running;       /* 使用原子变量保证多线程安全 */

    /* 互斥锁保护 */
    osal_mutex_t mutex;
} ccm_driver_context_t;

/*
 * 心跳任务
 */
static void *heartbeat_task(void *arg)
{
    ccm_driver_context_t *ctx = (ccm_driver_context_t *)arg;
    uint8_t buf[256];
    size_t len;
    int32_t ret;

    LOG_INFO("PDL_CCM", "Heartbeat task started");

    while (OSAL_atomic_load_bool(&ctx->running))
    {
        /* 检查连接状态 */
        if (!ccm_eth_is_connected(ctx->eth_handle))
        {
            LOG_WARN("PDL_CCM", "Connection lost, skip heartbeat");
            OSAL_msleep(ctx->config.heartbeat_interval_ms);
            continue;
        }

        /* 编码心跳消息 - 使用新的 PRL API */
        prl_pmc_heartbeat_t hb = {
            .sender_status = PDL_CCM_STATUS_OK,
            .link_quality = ctx->link_quality,
            .packet_loss = 0,
            .rtt_ms = 0
        };
        ret = PRL_encode(PRL_DEV_TYPE_PMC, PRL_PMC_MSG_HEARTBEAT,
                        &hb, OSAL_sizeof(hb), buf, OSAL_sizeof(buf), 0);
        if (ret < 0)
        {
            LOG_ERROR("PDL_CCM", "Failed to encode heartbeat");
            OSAL_pthread_mutex_lock(&ctx->mutex);
            ctx->error_count++;
            OSAL_pthread_mutex_unlock(&ctx->mutex);
            OSAL_msleep(ctx->config.heartbeat_interval_ms);
            continue;
        }
        len = ret;  /* PRL_encode 返回编码后的长度 */

        /* 发送心跳 */
        ccm_eth_msg_t msg = {
            .msg_type = 0x01,  /* 心跳消息类型 */
            .seq_num = 0,
            .payload_len = len,
        };
        OSAL_memcpy(msg.payload, buf, len);

        ret = ccm_eth_send(ctx->eth_handle, &msg, ctx->config.send_timeout_ms);
        if (ret == OSAL_SUCCESS)
        {
            OSAL_pthread_mutex_lock(&ctx->mutex);
            ctx->tx_count++;
            OSAL_pthread_mutex_unlock(&ctx->mutex);
        }
        else
        {
            LOG_ERROR("PDL_CCM", "Failed to send heartbeat");
            OSAL_pthread_mutex_lock(&ctx->mutex);
            ctx->error_count++;
            OSAL_pthread_mutex_unlock(&ctx->mutex);
        }

        /* 延迟 */
        OSAL_msleep(ctx->config.heartbeat_interval_ms);
    }

    LOG_INFO("PDL_CCM", "Heartbeat task stopped");
    return NULL;
}

/*
 * 以太网接收任务
 */
static void *eth_rx_task(void *arg)
{
    ccm_driver_context_t *ctx = (ccm_driver_context_t *)arg;
    ccm_eth_msg_t msg;
    int32_t ret;

    LOG_INFO("PDL_CCM", "Ethernet RX task started");

    while (OSAL_atomic_load_bool(&ctx->running))
    {
        /* 接收以太网消息 */
        ret = ccm_eth_recv(ctx->eth_handle, &msg, ctx->config.recv_timeout_ms);

        if (ret == OSAL_SUCCESS)
        {
            OSAL_pthread_mutex_lock(&ctx->mutex);
            ctx->rx_count++;
            OSAL_pthread_mutex_unlock(&ctx->mutex);

            /* 处理遥测数据 */
            if (msg.msg_type == 0x02)  /* 遥测消息类型 */
            {
                uint8_t dev_type, msg_type;
                const uint8_t *payload;
                uint16_t payload_len;

                ret = PRL_decode(msg.payload, msg.payload_len,
                                &dev_type, &msg_type, &payload, &payload_len);
                if (ret == OSAL_SUCCESS && payload_len >= OSAL_sizeof(prl_pmc_telemetry_t))
                {
                    const prl_pmc_telemetry_t *tm = (const prl_pmc_telemetry_t *)payload;
                    const uint8_t *data = payload + OSAL_sizeof(prl_pmc_telemetry_t);
                    size_t data_len = payload_len - OSAL_sizeof(prl_pmc_telemetry_t);

                    if (ctx->tm_callback)
                    {
                        ctx->tm_callback(tm->tm_id, tm->tm_source, data, data_len, ctx->tm_user_data);
                    }
                }
            }
            /* 处理遥控指令 */
            else if (msg.msg_type == 0x03)  /* 遥控消息类型 */
            {
                uint8_t dev_type, msg_type;
                const uint8_t *payload;
                uint16_t payload_len;

                ret = PRL_decode(msg.payload, msg.payload_len,
                                &dev_type, &msg_type, &payload, &payload_len);
                if (ret == OSAL_SUCCESS && payload_len >= OSAL_sizeof(prl_pmc_command_t))
                {
                    const prl_pmc_command_t *tc = (const prl_pmc_command_t *)payload;
                    const uint8_t *params = payload + OSAL_sizeof(prl_pmc_command_t);
                    size_t params_len = payload_len - OSAL_sizeof(prl_pmc_command_t);

                    if (ctx->tc_callback)
                    {
                        ctx->tc_callback(tc->cmd_id, tc->target_node, tc->cmd_type, params, params_len, ctx->tc_user_data);
                    }
                }
            }
            /* 处理应答消息 */
            else if (msg.msg_type == 0xFF)  /* 应答消息类型 */
            {
                uint8_t dev_type, msg_type;
                const uint8_t *payload;
                uint16_t payload_len;

                ret = PRL_decode(msg.payload, msg.payload_len,
                                &dev_type, &msg_type, &payload, &payload_len);
                if (ret == OSAL_SUCCESS && payload_len >= OSAL_sizeof(prl_pmc_ack_t))
                {
                    const prl_pmc_ack_t *ack = (const prl_pmc_ack_t *)payload;
                    LOG_DEBUG("PDL_CCM", "Received ACK seq=%u result=%u",
                                   ack->ack_seq, ack->ack_result);
                }
            }
        }
        else if (ret == OSAL_ERR_TIMEOUT)
        {
            /* 超时是正常的，继续接收 */
            continue;
        }
        else
        {
            LOG_ERROR("PDL_CCM", "Ethernet receive error");
            OSAL_pthread_mutex_lock(&ctx->mutex);
            ctx->error_count++;
            OSAL_pthread_mutex_unlock(&ctx->mutex);
        }
    }

    LOG_INFO("PDL_CCM", "Ethernet RX task stopped");
    return NULL;
}

/*
 * 初始化 CCM 系统驱动
 */
int32_t PDL_CCM_init(const pdl_ccm_config_t *config,
                     pdl_ccm_handle_t *handle)
{
    ccm_driver_context_t *ctx;
    int32_t ret;

    if (!config || !handle)
    {
        LOG_ERROR("PDL_CCM", "Invalid parameters");
        return OSAL_ERR_INVALID_PARAM;
    }

    /* 分配上下文 */
    ctx = (ccm_driver_context_t *)OSAL_malloc(OSAL_sizeof(ccm_driver_context_t));
    if (!ctx)
    {
        LOG_ERROR("PDL_CCM", "Failed to allocate context");
        return OSAL_ERR_NO_MEMORY;
    }

    OSAL_memset(ctx, 0, OSAL_sizeof(ccm_driver_context_t));
    OSAL_memcpy(&ctx->config, config, OSAL_sizeof(pdl_ccm_config_t));
    ctx->link_quality = 100;  /* 初始链路质量 */
    OSAL_atomic_init_bool(&ctx->running, false);

    /* 创建互斥锁 */
    ret = OSAL_pthread_mutex_init(&ctx->mutex, NULL);
    if (ret != OSAL_SUCCESS)
    {
        LOG_ERROR("PDL_CCM", "Failed to create mutex");
        OSAL_free(ctx);
        return ret;
    }

    /* 初始化以太网通信 */
    ret = ccm_eth_init(config, &ctx->eth_handle);
    if (ret != OSAL_SUCCESS)
    {
        LOG_ERROR("PDL_CCM", "Failed to init ethernet");
        OSAL_pthread_mutex_destroy(&ctx->mutex);
        OSAL_free(ctx);
        return ret;
    }

    /* 启动接收线程 */
    OSAL_atomic_store_bool(&ctx->running, true);
    ret = OSAL_pthread_create(&ctx->rx_thread, NULL, eth_rx_task, ctx);
    if (ret != OSAL_SUCCESS)
    {
        LOG_ERROR("PDL_CCM", "Failed to create RX thread");
        ccm_eth_deinit(ctx->eth_handle);
        OSAL_pthread_mutex_destroy(&ctx->mutex);
        OSAL_free(ctx);
        return ret;
    }

    /* 启动心跳线程 */
    ret = OSAL_pthread_create(&ctx->heartbeat_thread, NULL, heartbeat_task, ctx);
    if (ret != OSAL_SUCCESS)
    {
        LOG_ERROR("PDL_CCM", "Failed to create heartbeat thread");
        OSAL_atomic_store_bool(&ctx->running, false);
        OSAL_pthread_join(ctx->rx_thread, NULL);
        ccm_eth_deinit(ctx->eth_handle);
        OSAL_pthread_mutex_destroy(&ctx->mutex);
        OSAL_free(ctx);
        return ret;
    }

    *handle = ctx;
    LOG_INFO("PDL_CCM", "Initialized successfully");
    return OSAL_SUCCESS;
}

/*
 * 反初始化 CCM 系统驱动
 */
int32_t PDL_CCM_deinit(pdl_ccm_handle_t handle)
{
    ccm_driver_context_t *ctx = (ccm_driver_context_t *)handle;

    if (!ctx)
    {
        return OSAL_ERR_INVALID_PARAM;
    }

    /* 停止线程 */
    OSAL_atomic_store_bool(&ctx->running, false);
    OSAL_pthread_join(ctx->rx_thread, NULL);
    OSAL_pthread_join(ctx->heartbeat_thread, NULL);

    /* 清理以太网通信 */
    ccm_eth_deinit(ctx->eth_handle);

    /* 销毁互斥锁 */
    OSAL_pthread_mutex_destroy(&ctx->mutex);

    /* 释放上下文 */
    OSAL_free(ctx);

    LOG_INFO("PDL_CCM", "Deinitialized");
    return OSAL_SUCCESS;
}

/*
 * 注册遥测数据回调函数
 */
int32_t PDL_CCM_register_telemetry_callback(pdl_ccm_handle_t handle,
                                          pdl_ccm_telemetry_callback_t callback,
                                          void *user_data)
{
    ccm_driver_context_t *ctx = (ccm_driver_context_t *)handle;

    if (!ctx)
    {
        return OSAL_ERR_INVALID_PARAM;
    }

    OSAL_pthread_mutex_lock(&ctx->mutex);
    ctx->tm_callback = callback;
    ctx->tm_user_data = user_data;
    OSAL_pthread_mutex_unlock(&ctx->mutex);

    return OSAL_SUCCESS;
}

/*
 * 注册遥控指令回调函数
 */
int32_t PDL_CCM_register_command_callback(pdl_ccm_handle_t handle,
                                         pdl_ccm_command_callback_t callback,
                                         void *user_data)
{
    ccm_driver_context_t *ctx = (ccm_driver_context_t *)handle;

    if (!ctx)
    {
        return OSAL_ERR_INVALID_PARAM;
    }

    OSAL_pthread_mutex_lock(&ctx->mutex);
    ctx->tc_callback = callback;
    ctx->tc_user_data = user_data;
    OSAL_pthread_mutex_unlock(&ctx->mutex);

    return OSAL_SUCCESS;
}

/*
 * 获取驱动统计信息
 */
int32_t PDL_CCM_GetStats(pdl_ccm_handle_t handle,
                          uint32_t *rx_count,
                          uint32_t *tx_count,
                          uint32_t *error_count)
{
    ccm_driver_context_t *ctx = (ccm_driver_context_t *)handle;

    if (!ctx)
    {
        return OSAL_ERR_INVALID_PARAM;
    }

    OSAL_pthread_mutex_lock(&ctx->mutex);
    if (rx_count) *rx_count = ctx->rx_count;
    if (tx_count) *tx_count = ctx->tx_count;
    if (error_count) *error_count = ctx->error_count;
    OSAL_pthread_mutex_unlock(&ctx->mutex);

    return OSAL_SUCCESS;
}

/*
 * 获取连接状态
 */
int32_t PDL_CCM_GetConnectionStatus(pdl_ccm_handle_t handle,
                                     bool *connected,
                                     uint32_t *link_quality)
{
    ccm_driver_context_t *ctx = (ccm_driver_context_t *)handle;

    if (!ctx)
    {
        return OSAL_ERR_INVALID_PARAM;
    }

    if (connected)
    {
        *connected = ccm_eth_is_connected(ctx->eth_handle);
    }

    if (link_quality)
    {
        OSAL_pthread_mutex_lock(&ctx->mutex);
        *link_quality = ctx->link_quality;
        OSAL_pthread_mutex_unlock(&ctx->mutex);
    }

    return OSAL_SUCCESS;
}
