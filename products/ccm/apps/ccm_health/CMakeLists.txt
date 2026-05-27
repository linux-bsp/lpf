# =============================================================================
# ccm_health 应用程序 CMake 配置
# =============================================================================

set(HEALTH_SRCS
    src/ccm_health.c
    src/main.c
)

add_executable(ccm_health ${HEALTH_SRCS})

target_include_directories(ccm_health
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ccm_health
    PRIVATE
        h200_am625
        ccm
)

install(TARGETS ccm_health
    RUNTIME DESTINATION bin
)
