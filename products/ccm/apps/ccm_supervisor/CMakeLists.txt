# =============================================================================
# ccm_supervisor 应用程序 CMake 配置
# =============================================================================

set(SUPERVISOR_SRCS
    src/ccm_supervisor.c
    src/main.c
)

add_executable(ccm_supervisor ${SUPERVISOR_SRCS})

target_include_directories(ccm_supervisor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ccm_supervisor
    PRIVATE
        h200_am625
        ccm
)

install(TARGETS ccm_supervisor
    RUNTIME DESTINATION bin
)
