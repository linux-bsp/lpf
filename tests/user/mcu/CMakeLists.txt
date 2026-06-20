set(LPF_MCU_TRANSPORT_REGISTRY_SOURCES
    test_lpf_mcu_transport_registry.c
    "${SDK_PATH}/kernel/lpf-runtime/transport/mcu/lpf_mcu_transport.c"
)

add_executable(lpf_mcu_transport_registry_all_test
    ${LPF_MCU_TRANSPORT_REGISTRY_SOURCES}
)

target_compile_definitions(lpf_mcu_transport_registry_all_test PRIVATE
    CONFIG_LPF_MCU_TRANSPORT_CAN
    CONFIG_LPF_MCU_TRANSPORT_UART
    EXPECT_CAN_TRANSPORT=1
    EXPECT_UART_TRANSPORT=1
)

target_include_directories(lpf_mcu_transport_registry_all_test PRIVATE
    "${SDK_PATH}/kernel/include"
    "${SDK_PATH}/kernel/include/lpf"
    "${SDK_PATH}/user/osal/include/osal"
)

add_test(NAME lpf_mcu_transport_registry_all
         COMMAND lpf_mcu_transport_registry_all_test)
