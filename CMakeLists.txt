cmake_minimum_required(VERSION 3.16)

# 检查是否指定了产品
if(NOT DEFINED PRODUCT)
    message(FATAL_ERROR "Please specify PRODUCT, e.g., cmake -DPRODUCT=ccm_product")
endif()

# 产品目录
set(PRODUCT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/products/${PRODUCT}")

# 检查产品目录是否存在
if(NOT EXISTS ${PRODUCT_DIR})
    message(FATAL_ERROR "Product directory not found: ${PRODUCT_DIR}")
endif()

# 检查产品的 CMakeLists.txt 是否存在
if(NOT EXISTS ${PRODUCT_DIR}/CMakeLists.txt)
    message(FATAL_ERROR "Product CMakeLists.txt not found: ${PRODUCT_DIR}/CMakeLists.txt")
endif()

# 如果指定了配置文件，复制到产品目录
if(DEFINED CONFIG)
    set(CONFIG_FILE "${PRODUCT_DIR}/configs/${CONFIG}_defconfig")
    if(NOT EXISTS ${CONFIG_FILE})
        message(FATAL_ERROR "Config file not found: ${CONFIG_FILE}")
    endif()
    message(STATUS "Loading config: ${CONFIG}")
    file(COPY ${CONFIG_FILE} DESTINATION ${PRODUCT_DIR})
    file(RENAME ${PRODUCT_DIR}/${CONFIG}_defconfig ${PRODUCT_DIR}/.config.mk)
endif()

message(STATUS "Building product: ${PRODUCT}")

# 直接包含产品的 CMakeLists.txt，而不是作为子目录
# 这样可以避免嵌套 project() 调用的问题
set(CMAKE_CURRENT_SOURCE_DIR ${PRODUCT_DIR})
set(CMAKE_CURRENT_BINARY_DIR ${CMAKE_BINARY_DIR}/${PRODUCT})
include(${PRODUCT_DIR}/CMakeLists.txt)
