# =============================================================================
# ccm_logger 应用程序 CMake 配置
# =============================================================================

set(LOGGER_SRCS
    src/ccm_logger.c
    src/main.c
)

add_executable(ccm_logger ${LOGGER_SRCS})

target_include_directories(ccm_logger
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ccm_logger
    PRIVATE
        h200_am625
        ccm
)

install(TARGETS ccm_logger
    RUNTIME DESTINATION bin
)
