# =============================================================================
# 版本信息生成模块
# =============================================================================
# 功能：
# 1. 从 git 获取版本信息（hash、branch、dirty 状态）
# 2. 生成构建时间戳
# 3. 配置 version.h 头文件
# =============================================================================

# 获取 git 信息
find_package(Git QUIET)

if(GIT_FOUND)
    # 获取 git hash（短格式）
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short=8 HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    # 获取 git branch
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    # 检查是否有未提交的修改
    execute_process(
        COMMAND ${GIT_EXECUTABLE} diff-index --quiet HEAD --
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        RESULT_VARIABLE GIT_DIRTY_RESULT
        ERROR_QUIET
    )

    if(GIT_DIRTY_RESULT EQUAL 0)
        set(GIT_DIRTY "")
    else()
        set(GIT_DIRTY "-dirty")
    endif()
else()
    set(GIT_HASH "unknown")
    set(GIT_BRANCH "unknown")
    set(GIT_DIRTY "")
endif()

# 获取构建时间
string(TIMESTAMP BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP BUILD_TIME "%H:%M:%S")
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S UTC" UTC)

# 获取 defconfig 名称（如果有）
if(DEFINED DEFCONFIG)
    set(DEFCONFIG_NAME "${DEFCONFIG}")
else()
    set(DEFCONFIG_NAME "custom")
endif()

# 配置版本头文件
configure_file(
    ${CMAKE_SOURCE_DIR}/include/version.h.in
    ${CMAKE_BINARY_DIR}/include/version.h
    @ONLY
)

# 添加到包含路径
include_directories(${CMAKE_BINARY_DIR}/include)

# 输出版本信息
message(STATUS "")
message(STATUS "=== Version Information ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Git Hash: ${GIT_HASH}${GIT_DIRTY}")
message(STATUS "Git Branch: ${GIT_BRANCH}")
message(STATUS "Build Date: ${BUILD_DATE} ${BUILD_TIME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Defconfig: ${DEFCONFIG_NAME}")
message(STATUS "")
