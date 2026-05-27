# =============================================================================
# Tests CMake 配置
# =============================================================================

# 测试框架核心库
if(CONFIG_BUILD_TESTING)
    add_subdirectory(core)
endif()

# 单元测试
if(CONFIG_TEST_UNIT)
    add_subdirectory(unit)
endif()

# 性能测试
if(CONFIG_TEST_PERFORMANCE)
    add_subdirectory(performance)
endif()

# 压力测试
if(CONFIG_TEST_STRESS)
    add_subdirectory(stress)
endif()

# 系统测试
if(CONFIG_TEST_SYSTEM)
    add_subdirectory(system)
endif()
