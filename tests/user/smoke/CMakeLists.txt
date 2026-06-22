add_executable(pdm_mock_runtime_smoke
    test_lpf_mock_runtime_smoke.c
    "${SDK_PATH}/user/pdi/src/pdi_discovery.c"
    "${SDK_PATH}/user/pdi/src/pdi_led.c"
    "${SDK_PATH}/user/pdi/src/pdi_mcu.c"
    "${SDK_PATH}/user/pdi/src/pdi_syscall.c"
)

target_include_directories(pdm_mock_runtime_smoke PRIVATE
    "${SDK_PATH}/uapi"
    "${SDK_PATH}/user/pdi/include"
    "${SDK_PATH}/user/pdi/include/pdi"
    "${SDK_PATH}/user/pdi/src"
)
