add_library(hal INTERFACE)
target_include_directories(hal INTERFACE
    ${SDK_PATH}/core/kernel/include
    ${SDK_PATH}/core/kernel/include/hal
    ${SDK_PATH}/core/kernel/include/hal/config
)
