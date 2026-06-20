add_executable(pdi_basic_test
    test_pdi_basic.c
    "${SDK_PATH}/user/pdi/src/pdi_discovery.c"
    "${SDK_PATH}/user/pdi/src/pdi_led.c"
    "${SDK_PATH}/user/pdi/src/pdi_mcu.c"
    "${SDK_PATH}/user/pdi/src/pdi_syscall.c"
)

target_include_directories(pdi_basic_test PRIVATE
    "${SDK_PATH}/uapi"
    "${SDK_PATH}/user/pdi/include"
    "${SDK_PATH}/user/pdi/include/pdi"
    "${SDK_PATH}/user/pdi/src"
)

add_test(NAME pdi_basic COMMAND pdi_basic_test)

add_executable(pdi_ioctl_mock_test
    test_pdi_ioctl_mock.c
    "${SDK_PATH}/user/pdi/src/pdi_discovery.c"
    "${SDK_PATH}/user/pdi/src/pdi_led.c"
    "${SDK_PATH}/user/pdi/src/pdi_mcu.c"
    "${SDK_PATH}/user/pdi/src/pdi_syscall.c"
)

target_include_directories(pdi_ioctl_mock_test PRIVATE
    "${SDK_PATH}/uapi"
    "${SDK_PATH}/user/pdi/include"
    "${SDK_PATH}/user/pdi/include/pdi"
    "${SDK_PATH}/user/pdi/src"
)

add_test(NAME pdi_ioctl_mock COMMAND pdi_ioctl_mock_test)
