add_library(pdm INTERFACE)
target_include_directories(pdm INTERFACE
    ${SDK_PATH}/kernel/include
    ${SDK_PATH}/kernel/pdm/core
    ${SDK_PATH}/kernel/pdm/core/bus
    ${SDK_PATH}/kernel/pdm/core/device
    ${SDK_PATH}/kernel/pdm/core/driver
    ${SDK_PATH}/kernel/pdm/core/chardev
    ${SDK_PATH}/kernel/pdm/core/diag
)
