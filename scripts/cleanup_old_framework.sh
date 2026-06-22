#!/bin/bash
# 清理旧框架代码和 CONFIG_PDM_NEW_BUS 条件编译

set -e

WORKSPACE_ROOT="/home/wanguo/workspase/lpf"

echo "==================================================================="
echo "清理旧框架代码"
echo "==================================================================="

# 1. 清理 pdm_core.c - 移除条件编译，只保留新总线代码
echo "处理 kernel/pdm-core/core/pdm_core.c ..."

cat > "${WORKSPACE_ROOT}/kernel/pdm-core/core/pdm_core.c.new" << 'EOF'
// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>

#include "pdm/core/pdm_bus.h"
#include "pdm/pdm_errno.h"
#include "pdm/runtime/pdm_runtime.h"

/**
 * @brief PDM Core 模块初始化
 */
static int __init pdm_core_module_init(void)
{
	int ret;

	LOG_INFO("PDM_CORE", "Initializing PDM Core with Linux bus_type");

	/* 初始化 Linux bus_type */
	ret = pdm_bus_init();
	if (ret) {
		LOG_ERR("PDM_CORE", "Failed to initialize PDM bus: %d", ret);
		return ret;
	}

#ifdef CONFIG_PDM_RUNTIME
	/* 初始化运行时系统 */
	pdm_runtime_print_version();
	ret = pdm_runtime_init();
	if (ret != OSAL_SUCCESS) {
		LOG_ERR("PDM_CORE", "Failed to initialize runtime: %d", ret);
		pdm_bus_exit();
		return pdm_status_to_errno(ret);
	}
#endif

	LOG_INFO("PDM_CORE", "PDM Core initialized successfully");
	return 0;
}

/**
 * @brief PDM Core 模块退出
 */
static void __exit pdm_core_module_exit(void)
{
#ifdef CONFIG_PDM_RUNTIME
	pdm_runtime_exit();
#endif

	pdm_bus_exit();

	LOG_INFO("PDM_CORE", "PDM Core exited");
}

module_init(pdm_core_module_init);
module_exit(pdm_core_module_exit);

MODULE_AUTHOR("PDM");
MODULE_DESCRIPTION("PDM core device model with Linux bus_type");
MODULE_LICENSE("GPL");
MODULE_SOFTDEP("pre: osal can can_raw");
EOF

mv "${WORKSPACE_ROOT}/kernel/pdm-core/core/pdm_core.c.new" "${WORKSPACE_ROOT}/kernel/pdm-core/core/pdm_core.c"

echo "✓ pdm_core.c 清理完成"

# 2. 删除旧的伪总线文件
echo "删除旧的伪总线实现文件 ..."

OLD_FILES=(
    "kernel/pdm-core/core/pdm_driver.c"
    "kernel/include/pdm/core/pdm_driver.h"
)

for file in "${OLD_FILES[@]}"; do
    if [ -f "${WORKSPACE_ROOT}/${file}" ]; then
        rm -f "${WORKSPACE_ROOT}/${file}"
        echo "  删除: ${file}"
    fi
done

echo "✓ 旧框架文件删除完成"

echo ""
echo "==================================================================="
echo "清理完成！"
echo "==================================================================="
echo ""
echo "已清理的内容："
echo "  1. pdm_core.c - 移除条件编译，只使用新总线"
echo "  2. 删除旧的伪总线实现文件"
echo ""
echo "剩余工作："
echo "  - MCU 和 LED 驱动中的旧代码需要手动清理"
echo "  - 移除 CONFIG_PDM_NEW_BUS 配置选项"
echo ""
