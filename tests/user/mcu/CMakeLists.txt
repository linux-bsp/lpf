set(PDM_MCU_TRANSPORT_REGISTRY_SOURCES
    test_lpf_mcu_transport_registry.c
    "${SDK_PATH}/kernel/pdm-core/peripheral/mcu/pdm_mcu_transport.c"
)

add_executable(pdm_mcu_transport_registry_all_test
    ${PDM_MCU_TRANSPORT_REGISTRY_SOURCES}
)

target_compile_definitions(pdm_mcu_transport_registry_all_test PRIVATE
    CONFIG_PDM_MCU_TRANSPORT_CAN
    CONFIG_PDM_MCU_TRANSPORT_UART
    EXPECT_CAN_TRANSPORT=1
    EXPECT_UART_TRANSPORT=1
)

target_include_directories(pdm_mcu_transport_registry_all_test PRIVATE
    "${SDK_PATH}/kernel/include"
    "${SDK_PATH}/kernel/include/lpf"
    "${SDK_PATH}/kernel/pdm-core/peripheral/mcu"
    "${SDK_PATH}/user/osal/include/osal"
)

add_test(NAME pdm_mcu_transport_registry_all
         COMMAND pdm_mcu_transport_registry_all_test)
