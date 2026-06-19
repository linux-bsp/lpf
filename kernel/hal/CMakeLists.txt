add_library(hal INTERFACE)
target_include_directories(hal INTERFACE
    ${SDK_PATH}/kernel/include
    ${SDK_PATH}/kernel/include/hal
    ${SDK_PATH}/kernel/include/hal/config
)
