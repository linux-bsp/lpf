#include "osal.h"

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    LOG_INFO("DEMO", "=================================");
    LOG_INFO("DEMO", "  Sample Product Demo Application");
    LOG_INFO("DEMO", "=================================");

    // 打印版本信息
    print_version_info();

    LOG_INFO("DEMO", "Demo application running...");
    LOG_INFO("DEMO", "This is a sample application for demonstration purposes.");

    return 0;
}
