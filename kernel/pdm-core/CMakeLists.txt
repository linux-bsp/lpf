add_library(pdm_core INTERFACE)
target_include_directories(pdm_core INTERFACE
    ${SDK_PATH}/kernel/include
    ${SDK_PATH}/kernel/pdm-core/core
    ${SDK_PATH}/kernel/pdm-core/bus
    ${SDK_PATH}/kernel/pdm-core/compat
)

# 已删除的目录（不再需要）：
# ${SDK_PATH}/kernel/include/lpf          - 已重命名为 pdm
# ${SDK_PATH}/kernel/pdm-core/protocol    - 已删除：无使用者
# ${SDK_PATH}/kernel/pdm-core/soc         - 已删除：无使用者
# ${SDK_PATH}/kernel/pdm-core/include     - 已删除：内部头文件
# ${SDK_PATH}/kernel/pdm-core/config/src  - 已删除：pdm_configs 模块
# ${SDK_PATH}/kernel/pdm-core/hw          - 已删除：无使用者
# ${SDK_PATH}/kernel/pdm-core/peripheral/mcu - 已删除：service 已删除
# ${SDK_PATH}/kernel/pdm-core/peripheral/led - 已删除：service 已删除
