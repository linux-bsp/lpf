#include "ccm_logger.h"

int main(int argc, char *argv[])
{
    int32_t ret;

    (void)argc;
    (void)argv;

    LOG_INFO("MAIN", "CCM Logger进程启动");

    /* 初始化 */
    ret = CCM_Logger_Init();
    if (ret != OSAL_SUCCESS) {
        LOG_ERROR("MAIN", "初始化失败: %d", ret);
        return 1;
    }

    /* 运行主循环 */
    ret = CCM_Logger_Run();

    /* 清理 */
    CCM_Logger_Cleanup();

    LOG_INFO("MAIN", "CCM Logger进程退出: %d", ret);
    return (ret == OSAL_SUCCESS) ? 0 : 1;
}
