# =============================================================================
# 构建产物校验和生成模块
# =============================================================================
# 功能：
# 1. 为所有二进制文件生成 SHA256 校验和
# 2. 生成校验和清单文件
# 3. 提供校验和验证功能
# =============================================================================

# 添加自定义目标：生成校验和
add_custom_target(checksums
    COMMAND ${CMAKE_COMMAND} -E echo "Generating checksums..."
    COMMAND ${CMAKE_COMMAND}
        -DBINARY_DIR=${CMAKE_BINARY_DIR}
        -DOUTPUT_FILE=${CMAKE_BINARY_DIR}/checksums.txt
        -P ${CMAKE_SOURCE_DIR}/cmake/GenerateChecksums.cmake
    COMMENT "Generating SHA256 checksums for all binaries"
    VERBATIM
)

# 添加自定义目标：验证校验和
add_custom_target(verify-checksums
    COMMAND ${CMAKE_COMMAND}
        -DBINARY_DIR=${CMAKE_BINARY_DIR}
        -DCHECKSUM_FILE=${CMAKE_BINARY_DIR}/checksums.txt
        -P ${CMAKE_SOURCE_DIR}/cmake/VerifyChecksums.cmake
    COMMENT "Verifying SHA256 checksums"
    VERBATIM
)

# 安装时生成校验和
install(CODE "
    message(STATUS \"Generating checksums for installed files...\")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            -DBINARY_DIR=\${CMAKE_INSTALL_PREFIX}
            -DOUTPUT_FILE=\${CMAKE_INSTALL_PREFIX}/checksums.txt
            -P ${CMAKE_SOURCE_DIR}/cmake/GenerateChecksums.cmake
    )
")
