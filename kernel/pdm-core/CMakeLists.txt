add_library(pdm_core INTERFACE)
target_include_directories(pdm_core INTERFACE
    ${SDK_PATH}/kernel/include
    ${SDK_PATH}/kernel/include/lpf
    ${SDK_PATH}/kernel/pdm-core/core
    ${SDK_PATH}/kernel/pdm-core/protocol
    ${SDK_PATH}/kernel/pdm-core/compat
    ${SDK_PATH}/kernel/pdm-core/soc
    ${SDK_PATH}/kernel/pdm-core/include
    ${SDK_PATH}/kernel/pdm-core/config/src
    ${SDK_PATH}/kernel/pdm-core/hw
    ${SDK_PATH}/kernel/pdm-core/peripheral/mcu
    ${SDK_PATH}/kernel/pdm-core/peripheral/led
)
