# =============================================================================
# 验证校验和脚本
# =============================================================================
# 用法: cmake -DBINARY_DIR=<dir> -DCHECKSUM_FILE=<file> -P VerifyChecksums.cmake
# =============================================================================

if(NOT DEFINED BINARY_DIR)
    message(FATAL_ERROR "BINARY_DIR not defined")
endif()

if(NOT DEFINED CHECKSUM_FILE)
    message(FATAL_ERROR "CHECKSUM_FILE not defined")
endif()

if(NOT EXISTS "${CHECKSUM_FILE}")
    message(FATAL_ERROR "Checksum file not found: ${CHECKSUM_FILE}")
endif()

# 读取校验和文件
file(STRINGS "${CHECKSUM_FILE}" CHECKSUM_LINES)

set(TOTAL 0)
set(PASSED 0)
set(FAILED 0)
set(MISSING 0)

message(STATUS "Verifying checksums from: ${CHECKSUM_FILE}")

foreach(LINE ${CHECKSUM_LINES})
    # 跳过注释和空行
    if(LINE MATCHES "^#" OR LINE STREQUAL "")
        continue()
    endif()

    # 解析行: <hash>  <file>
    if(LINE MATCHES "^([0-9a-f]+)  (.+)$")
        set(EXPECTED_HASH "${CMAKE_MATCH_1}")
        set(REL_PATH "${CMAKE_MATCH_2}")
        set(FILE_PATH "${BINARY_DIR}/${REL_PATH}")

        math(EXPR TOTAL "${TOTAL} + 1")

        if(NOT EXISTS "${FILE_PATH}")
            message(WARNING "  MISSING: ${REL_PATH}")
            math(EXPR MISSING "${MISSING} + 1")
            continue()
        endif()

        file(SHA256 "${FILE_PATH}" ACTUAL_HASH)

        if(EXPECTED_HASH STREQUAL ACTUAL_HASH)
            message(STATUS "  OK: ${REL_PATH}")
            math(EXPR PASSED "${PASSED} + 1")
        else()
            message(WARNING "  FAILED: ${REL_PATH}")
            message(WARNING "    Expected: ${EXPECTED_HASH}")
            message(WARNING "    Actual:   ${ACTUAL_HASH}")
            math(EXPR FAILED "${FAILED} + 1")
        endif()
    endif()
endforeach()

message(STATUS "")
message(STATUS "Verification Summary:")
message(STATUS "  Total:   ${TOTAL}")
message(STATUS "  Passed:  ${PASSED}")
message(STATUS "  Failed:  ${FAILED}")
message(STATUS "  Missing: ${MISSING}")

if(FAILED GREATER 0 OR MISSING GREATER 0)
    message(FATAL_ERROR "Checksum verification failed!")
endif()

message(STATUS "All checksums verified successfully!")
