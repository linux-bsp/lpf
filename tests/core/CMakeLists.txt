# =============================================================================
# 测试框架核心库 CMake 配置
# =============================================================================

file(GLOB TESTCORE_SRCS
    "*.c"
)

add_library(testcore SHARED ${TESTCORE_SRCS})

set_target_properties(testcore PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
    OUTPUT_NAME testcore
)

target_include_directories(testcore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/tests/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(testcore
    PUBLIC
        osal
)

install(TARGETS testcore
    LIBRARY DESTINATION lib
)
