cmake_minimum_required(VERSION 3.10)

project(EMS VERSION 1.0.0 LANGUAGES C)

# 设置C标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ============================================================================
# Kconfig 集成
# ============================================================================
# 检查 Kconfig 配置是否存在
set(KCONFIG_HEADER "${CMAKE_SOURCE_DIR}/include/generated/autoconf.h")
if(NOT EXISTS ${KCONFIG_HEADER})
    message(WARNING "Kconfig not configured. Run 'make menuconfig' or 'make defconfig' first.")
    message(WARNING "Using default CMake configuration...")
endif()

# 包含生成的配置目录
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/generated)

# Kconfig 辅助函数：检查选项是否设置
function(kconfig_is_set option result_var)
    if(EXISTS ${KCONFIG_HEADER})
        file(STRINGS ${KCONFIG_HEADER} CONFIG_LINES)
        if("${CONFIG_LINES}" MATCHES "#define ${option}")
            set(${result_var} TRUE PARENT_SCOPE)
        else()
            set(${result_var} FALSE PARENT_SCOPE)
        endif()
    else()
        set(${result_var} FALSE PARENT_SCOPE)
    endif()
endfunction()

# Kconfig 辅助函数：获取选项值
function(kconfig_get_value option result_var)
    if(EXISTS ${KCONFIG_HEADER})
        file(STRINGS ${KCONFIG_HEADER} CONFIG_LINES)
        foreach(line ${CONFIG_LINES})
            if("${line}" MATCHES "#define ${option} (.+)")
                set(${result_var} ${CMAKE_MATCH_1} PARENT_SCOPE)
                return()
            endif()
        endforeach()
    endif()
    set(${result_var} "" PARENT_SCOPE)
endfunction()

# ============================================================================
# 产品线和项目选择（优先使用 Kconfig，回退到 CMake 选项）
# ============================================================================
# 从 Kconfig 读取产品配置
kconfig_is_set(CONFIG_PRODUCT_PMC KCONFIG_PMC)
kconfig_is_set(CONFIG_PRODUCT_SAMPLES KCONFIG_SAMPLES)
kconfig_is_set(CONFIG_PROJECT_H200_AM625 KCONFIG_H200)

if(KCONFIG_PMC)
    set(PRODUCT "pmc")
    if(KCONFIG_H200)
        set(PRODUCT_PROJECT "h200_am625")
    endif()
elseif(KCONFIG_SAMPLES)
    set(PRODUCT "samples")
    set(PRODUCT_PROJECT "project_demo")
else()
    # 回退到 CMake 选项（兼容旧构建方式）
    set(PRODUCT "pmc" CACHE STRING "Product line to build")
    set_property(CACHE PRODUCT PROPERTY STRINGS
        "pmc"
        "samples"
    )

    # 根据产品线设置可用的项目列表
    if(PRODUCT STREQUAL "pmc")
        set(PRODUCT_PROJECT "h200_am625" CACHE STRING "Project to build")
        set_property(CACHE PRODUCT_PROJECT PROPERTY STRINGS
            "h200_am625"
        )
    elseif(PRODUCT STREQUAL "samples")
        set(PRODUCT_PROJECT "project_demo" CACHE STRING "Project to build")
        set_property(CACHE PRODUCT_PROJECT PROPERTY STRINGS
            "project_demo"
        )
    endif()
endif()

message(STATUS "Selected product: ${PRODUCT}")
message(STATUS "Selected project: ${PRODUCT_PROJECT}")

# ============================================================================
# 构建选项（优先使用 Kconfig，回退到 CMake 选项）
# ============================================================================
# 从 Kconfig 读取构建选项
kconfig_is_set(CONFIG_BUILD_TESTING KCONFIG_BUILD_TESTING)
kconfig_is_set(CONFIG_BUILD_SHARED_LIBS KCONFIG_BUILD_SHARED)
kconfig_is_set(CONFIG_INSTALL_HEADERS KCONFIG_INSTALL_HEADERS)

if(EXISTS ${KCONFIG_HEADER})
    # 使用 Kconfig 配置
    set(BUILD_TESTING ${KCONFIG_BUILD_TESTING})
    set(BUILD_SHARED_LIBS ${KCONFIG_BUILD_SHARED})
    set(INSTALL_HEADERS ${KCONFIG_INSTALL_HEADERS})
    message(STATUS "Using Kconfig build options")
else()
    # 回退到 CMake 选项
    option(BUILD_TESTING "Build unit tests" ON)
    option(BUILD_SHARED_LIBS "Build shared libraries instead of static" ON)
    option(INSTALL_HEADERS "Install header files" ON)
    message(STATUS "Using CMake build options (Kconfig not found)")
endif()

# ============================================================================
# 安装路径配置（Buildroot 兼容）
# ============================================================================
# 使用 GNUInstallDirs 提供标准安装路径变量
include(GNUInstallDirs)

# 默认安装路径：/usr/local（可通过 -DCMAKE_INSTALL_PREFIX 覆盖）
# Buildroot 通常设置为 /usr
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Install prefix" FORCE)
endif()

# Buildroot 兼容说明：
# - CMAKE_INSTALL_PREFIX: 安装前缀（Buildroot 通常设置为 /usr）
# - CMAKE_INSTALL_LIBDIR: 库文件目录（相对路径，如 lib 或 lib64）
# - CMAKE_INSTALL_BINDIR: 可执行文件目录（相对路径，如 bin）
# - CMAKE_INSTALL_INCLUDEDIR: 头文件目录（相对路径，如 include）
# - DESTDIR: 安装时的目标根目录（make install DESTDIR=/staging）

# ============================================================================
# 编译选项（优先使用 Kconfig，回退到默认值）
# ============================================================================
# 基础编译标志
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -D_POSIX_C_SOURCE=200809L")

# macOS平台特定定义
if(APPLE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_DARWIN_C_SOURCE")
endif()

# 从 Kconfig 读取构建类型和优化级别
kconfig_is_set(CONFIG_BUILD_TYPE_DEBUG KCONFIG_DEBUG)
kconfig_is_set(CONFIG_OPT_O0 KCONFIG_OPT_O0)
kconfig_is_set(CONFIG_OPT_O2 KCONFIG_OPT_O2)
kconfig_is_set(CONFIG_OPT_O3 KCONFIG_OPT_O3)
kconfig_is_set(CONFIG_OPT_OS KCONFIG_OPT_OS)

if(EXISTS ${KCONFIG_HEADER})
    # 使用 Kconfig 配置
    if(KCONFIG_DEBUG)
        set(CMAKE_BUILD_TYPE Debug)
        message(STATUS "Build type: Debug (from Kconfig)")
    else()
        set(CMAKE_BUILD_TYPE Release)
        message(STATUS "Build type: Release (from Kconfig)")
    endif()

    # 设置优化级别
    if(KCONFIG_OPT_O0)
        set(CMAKE_C_FLAGS_RELEASE "-O0")
        set(CMAKE_C_FLAGS_DEBUG "-g -O0")
    elseif(KCONFIG_OPT_O2)
        set(CMAKE_C_FLAGS_RELEASE "-O2")
        set(CMAKE_C_FLAGS_DEBUG "-g -O0")
    elseif(KCONFIG_OPT_O3)
        set(CMAKE_C_FLAGS_RELEASE "-O3")
        set(CMAKE_C_FLAGS_DEBUG "-g -O0")
    elseif(KCONFIG_OPT_OS)
        set(CMAKE_C_FLAGS_RELEASE "-Os")
        set(CMAKE_C_FLAGS_DEBUG "-g -O0")
    endif()
else()
    # 回退到默认值
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type (Debug/Release)" FORCE)
    endif()

    # 构建类型特定的标志（如果 Buildroot 未设置 CMAKE_C_FLAGS_RELEASE）
    if(NOT CMAKE_C_FLAGS_DEBUG)
        set(CMAKE_C_FLAGS_DEBUG "-g -O0")
    endif()
    if(NOT CMAKE_C_FLAGS_RELEASE)
        set(CMAKE_C_FLAGS_RELEASE "-O2")
    endif()
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C flags (Release): ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "C flags (Debug): ${CMAKE_C_FLAGS_DEBUG}")

# ============================================================================
# 输出目录配置
# ============================================================================
# 使用更简洁的输出目录结构
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ============================================================================
# 查找依赖库
# ============================================================================
find_package(Threads REQUIRED)

# ============================================================================
# 模块化构建
# ============================================================================
# 依赖层次：OSAL → HAL → PCL → PDL → ACL → Projects
#
# 每个模块提供两个库：
# 1. <module>_public_api - 接口库（仅头文件），供上层使用
# 2. <module> - 实现库（静态库），包含实际代码
#
# 上层模块：
# - PUBLIC依赖下层的接口库（继承头文件路径）
# - PRIVATE依赖下层的实现库（运行时链接）

# 1. OSAL层（操作系统抽象层）
add_subdirectory(core/osal)

# 为后续模块创建命名空间别名
add_library(ems::osal_public_api ALIAS osal_public_api)
add_library(ems::osal ALIAS osal)

# 2. HAL层（硬件抽象层）
add_subdirectory(core/hal)
add_library(ems::hal_public_api ALIAS hal_public_api)
add_library(ems::hal ALIAS hal)

# 3. PCL层（外设配置层）
add_subdirectory(core/pcl)
add_library(ems::pcl_public_api ALIAS pcl_public_api)
add_library(ems::pcl ALIAS pcl)

# 4. PDL层（外设驱动层）
add_subdirectory(core/pdl)
add_library(ems::pdl_public_api ALIAS pdl_public_api)
add_library(ems::pdl ALIAS pdl)

# 5. 确定项目路径（用于ACL配置）
set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/products/${PRODUCT}/${PRODUCT_PROJECT})

if(NOT EXISTS ${PROJECT_DIR}/CMakeLists.txt)
    message(FATAL_ERROR "Project '${PRODUCT}/${PRODUCT_PROJECT}' not found at ${PROJECT_DIR}")
endif()

# 6. ACL层（应用配置层）
# 项目需要在其acl/目录下提供配置源文件
file(GLOB ACL_CONFIG_SOURCES "${PROJECT_DIR}/acl/*.c")
if(ACL_CONFIG_SOURCES)
    message(STATUS "Found ACL config sources in ${PROJECT_DIR}/acl/")
endif()
add_subdirectory(core/acl)
add_library(ems::acl ALIAS acl)

# 7. 项目层（Products）
message(STATUS "Building from: ${PROJECT_DIR}")
add_subdirectory(${PROJECT_DIR} ${CMAKE_CURRENT_BINARY_DIR}/project)

# ============================================================================
# 测试框架（可选）
# ============================================================================
if(BUILD_TESTING)
    # 统一测试框架（包含所有测试模块和运行器）
    add_subdirectory(tests)
endif()

# ============================================================================
# 打印配置信息
# ============================================================================
message(STATUS "")
message(STATUS "========================================")
message(STATUS "EMS v${PROJECT_VERSION}")
message(STATUS "========================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "System: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER}")
if(CMAKE_TOOLCHAIN_FILE)
message(STATUS "Toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Binaries: ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}")
message(STATUS "  Libraries: ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
message(STATUS "  Headers: ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}")
message(STATUS "")
message(STATUS "Build options:")
message(STATUS "  PROJECT_NAME: ${PROJECT_NAME}")
message(STATUS "  BUILD_TESTING: ${BUILD_TESTING}")
message(STATUS "  BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}")
message(STATUS "  INSTALL_HEADERS: ${INSTALL_HEADERS}")
message(STATUS "")
message(STATUS "Output: ${CMAKE_BINARY_DIR}")
message(STATUS "========================================")
message(STATUS "")
