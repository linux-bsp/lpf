/************************************************************************
 * Watchdog应用 - 主程序
 *
 * 这是一个看门狗喂狗应用，展示如何使用PDL层的Watchdog接口：
 * - 初始化看门狗服务
 * - 启动自动喂狗
 * - 监控看门狗状态
 * - 优雅退出
 ************************************************************************/

#include "osal.h"
#include "pdl_watchdog.h"

/* 应用版本 */
#define APP_VERSION "1.0.0"

/* 统计信息打印间隔（毫秒） */
#define STATS_INTERVAL_MS  30000  /* 30秒打印一次统计 */

/* 全局运行标志 */
static volatile bool g_running = true;

/* 看门狗句柄 */
static watchdog_handle_t g_watchdog_handle = NULL;

/* 统计线程句柄 */
static osal_thread_t g_stats_thread = 0;

/**
 * @brief 信号处理函数
 *
 * 捕获SIGINT(Ctrl+C)和SIGTERM信号，实现优雅退出
 */
static void signal_handler(int32_t sig)
{
    if (sig == OS_SIGNAL_INT || sig == OS_SIGNAL_TERM)
    {
        OSAL_Printf("\n[Main] 收到退出信号，正在关闭应用...\n");
        g_running = false;
    }
}

/**
 * @brief 统计任务
 *
 * 定期打印看门狗统计信息
 */
static void *stats_task(void *arg)
{
    (void)arg;

    LOG_INFO("Stats", "统计线程启动");

    while (g_running)
    {
        OSAL_msleep(STATS_INTERVAL_MS);

        if (!g_running)
        {
            break;
        }

        /* 获取看门狗状态 */
        watchdog_status_t status;
        int32_t ret = PDL_WATCHDOG_GetStatus(g_watchdog_handle, &status);
        if (ret == OSAL_SUCCESS)
        {
            OSAL_Printf("\n========== Watchdog 状态 ==========\n");
            OSAL_Printf("  启用状态: %s\n", status.enabled ? "已启用" : "未启用");
            OSAL_Printf("  运行状态: %s\n", status.running ? "运行中" : "已停止");
            OSAL_Printf("  工作模式: %s\n", status.mode == WATCHDOG_MODE_AUTO ? "自动" : "手动");
            OSAL_Printf("  超时时间: %u 秒\n", status.timeout_sec);
            OSAL_Printf("  喂狗间隔: %u 毫秒\n", status.kick_interval_ms);
            OSAL_Printf("  喂狗次数: %u\n", status.kick_count);
            OSAL_Printf("===================================\n\n");
        }
    }

    LOG_INFO("Stats", "统计线程退出");
    return NULL;
}

/**
 * @brief 主函数
 */
int32_t main(int32_t argc, char *argv[])
{
    (void)argc;
    (void)argv;

    OSAL_Printf("========================================\n");
    OSAL_Printf("Watchdog应用 v%s\n", APP_VERSION);
    OSAL_Printf("========================================\n\n");

    /* 注册信号处理 */
    OSAL_SignalRegister(OS_SIGNAL_INT, signal_handler);
    OSAL_SignalRegister(OS_SIGNAL_TERM, signal_handler);

    /* 初始化看门狗服务 */
    watchdog_config_t config = {
        .name = "system_watchdog",
        .device = "/dev/watchdog",
        .timeout_sec = 60,              /* 60秒超时 */
        .mode = WATCHDOG_MODE_AUTO,     /* 自动模式 */
        .kick_interval_ms = 5000,       /* 5秒喂一次狗 */
        .enable_on_init = true          /* 初始化时启用 */
    };

    LOG_INFO("Main", "初始化看门狗服务: %s", config.name);
    int32_t ret = PDL_WATCHDOG_Init(&config, &g_watchdog_handle);
    if (ret != OSAL_SUCCESS)
    {
        LOG_ERROR("Main", "看门狗服务初始化失败: %d", ret);
        LOG_ERROR("Main", "请确保:");
        LOG_ERROR("Main", "  1. 看门狗设备存在: %s", config.device);
        LOG_ERROR("Main", "  2. 有足够的权限访问设备（可能需要root权限）");
        LOG_ERROR("Main", "  3. 看门狗驱动已加载");
        return OSAL_ERR_GENERIC;
    }

    LOG_INFO("Main", "看门狗服务初始化成功");

    /* 启动自动喂狗服务 */
    ret = PDL_WATCHDOG_Start(g_watchdog_handle);
    if (ret != OSAL_SUCCESS)
    {
        LOG_ERROR("Main", "启动自动喂狗服务失败");
        PDL_WATCHDOG_Deinit(g_watchdog_handle);
        return OSAL_ERR_GENERIC;
    }

    LOG_INFO("Main", "自动喂狗服务已启动");

    /* 创建统计线程 */
    ret = OSAL_ThreadCreate(&g_stats_thread, stats_task, NULL);
    if (ret != OSAL_SUCCESS)
    {
        LOG_ERROR("Main", "创建统计线程失败");
        PDL_WATCHDOG_Stop(g_watchdog_handle);
        PDL_WATCHDOG_Deinit(g_watchdog_handle);
        return OSAL_ERR_GENERIC;
    }

    LOG_INFO("Main", "应用启动成功，按Ctrl+C退出");
    OSAL_Printf("\n提示：\n");
    OSAL_Printf("  - 看门狗超时时间: %u 秒\n", config.timeout_sec);
    OSAL_Printf("  - 喂狗间隔: %u 毫秒\n", config.kick_interval_ms);
    OSAL_Printf("  - 统计信息间隔: %u 毫秒\n", STATS_INTERVAL_MS);
    OSAL_Printf("  - 工作模式: 自动（PDL内部线程自动喂狗）\n\n");

    /* 等待统计线程退出 */
    OSAL_ThreadJoin(g_stats_thread);

    /* 停止自动喂狗服务 */
    LOG_INFO("Main", "停止自动喂狗服务");
    PDL_WATCHDOG_Stop(g_watchdog_handle);

    /* 清理资源 */
    LOG_INFO("Main", "关闭看门狗服务");
    PDL_WATCHDOG_Deinit(g_watchdog_handle);

    OSAL_Printf("\n应用已退出\n");
    return OSAL_SUCCESS;
}
