# =============================================================================
# 生成校验和脚本
# =============================================================================
# 用法: cmake -DBINARY_DIR=<dir> -DOUTPUT_FILE=<file> -P GenerateChecksums.cmake
# =============================================================================

if(NOT DEFINED BINARY_DIR)
    message(FATAL_ERROR "BINARY_DIR not defined")
endif()

if(NOT DEFINED OUTPUT_FILE)
    message(FATAL_ERROR "OUTPUT_FILE not defined")
endif()

# 查找所有二进制文件和库文件
file(GLOB_RECURSE BINARIES
    "${BINARY_DIR}/bin/*"
    "${BINARY_DIR}/lib/*.so"
    "${BINARY_DIR}/lib/*.so.*"
    "${BINARY_DIR}/lib/*.a"
)

# 过滤掉符号链接
set(REAL_BINARIES "")
foreach(FILE ${BINARIES})
    if(NOT IS_SYMLINK "${FILE}")
        list(APPEND REAL_BINARIES "${FILE}")
    endif()
endforeach()

# 生成校验和
file(WRITE "${OUTPUT_FILE}" "# SHA256 Checksums\n")
file(APPEND "${OUTPUT_FILE}" "# Generated: ${CMAKE_CURRENT_LIST_FILE}\n")
file(APPEND "${OUTPUT_FILE}" "# Date: ")

execute_process(
    COMMAND date "+%Y-%m-%d %H:%M:%S"
    OUTPUT_VARIABLE CURRENT_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
file(APPEND "${OUTPUT_FILE}" "${CURRENT_DATE}\n\n")

foreach(FILE ${REAL_BINARIES})
    file(SHA256 "${FILE}" FILE_HASH)
    file(RELATIVE_PATH REL_PATH "${BINARY_DIR}" "${FILE}")
    file(APPEND "${OUTPUT_FILE}" "${FILE_HASH}  ${REL_PATH}\n")
    message(STATUS "  ${REL_PATH}: ${FILE_HASH}")
endforeach()

list(LENGTH REAL_BINARIES NUM_FILES)
message(STATUS "Generated checksums for ${NUM_FILES} files: ${OUTPUT_FILE}")
