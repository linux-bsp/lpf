# =============================================================================
# 系统测试 CMake 配置
# =============================================================================

# 收集源文件
set(SYSTEM_TEST_SRCS
    test_system_core.c
)

if(CONFIG_TEST_SYSTEM_OSAL)
    list(APPEND SYSTEM_TEST_SRCS osal/test_system_osal.c)
endif()

if(CONFIG_TEST_SYSTEM_HAL)
    list(APPEND SYSTEM_TEST_SRCS hal/test_system_hal.c)
endif()

if(CONFIG_TEST_SYSTEM_PCL)
    list(APPEND SYSTEM_TEST_SRCS pcl/test_system_pcl.c)
endif()

if(CONFIG_TEST_SYSTEM_PDL)
    list(APPEND SYSTEM_TEST_SRCS pdl/test_system_pdl.c)
endif()

if(CONFIG_TEST_SYSTEM_ACL)
    list(APPEND SYSTEM_TEST_SRCS acl/test_system_acl.c)
endif()

if(CONFIG_TEST_INTEGRATION)
    list(APPEND SYSTEM_TEST_SRCS integration/test_system_integration.c)
endif()

# 创建可执行文件
add_executable(ems-system-test ${SYSTEM_TEST_SRCS})

target_include_directories(ems-system-test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/tests/include
)

# 链接库（CMake 自动处理传递依赖）
target_link_libraries(ems-system-test
    PRIVATE
        testcore
        acl
        pdl
        pcl
        hal
        osal
)

install(TARGETS ems-system-test
    RUNTIME DESTINATION bin
)
