# =============================================================================
# ccm_comm 应用程序 CMake 配置
# =============================================================================

set(COMM_SRCS
    src/ccm_comm.c
    src/main.c
)

add_executable(ccm_comm ${COMM_SRCS})

target_include_directories(ccm_comm
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ccm_comm
    PRIVATE
        h200_am625
        ccm
)

install(TARGETS ccm_comm
    RUNTIME DESTINATION bin
)
