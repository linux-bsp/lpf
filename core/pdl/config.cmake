# =============================================================================
# PDL 模块配置
# =============================================================================
# 功能：根据 Kconfig 配置选择需要编译的源文件
# 用法：include(config.cmake)
#       然后使用 ${PDL_SRCS} 变量
# =============================================================================

message(STATUS "Configuring PDL module...")

set(PDL_SRCS "")

# MCU 外设支持（根据 Kconfig 配置）
if(CONFIG_PDL_MCU_SUPPORT)
    list(APPEND PDL_SRCS
        "src/pdl_mcu/pdl_mcu.c"
        "src/pdl_mcu/pdl_mcu_can.c"
        "src/pdl_mcu/pdl_mcu_serial.c"
    )
    message(STATUS "  [PDL] MCU peripheral support enabled")
endif()

if(NOT PDL_SRCS)
    message(WARNING "PDL: No peripheral support enabled. Skipping PDL build.")
    message(WARNING "  Enable PDL_MCU_SUPPORT to build the current peripheral layer")
else()
    list(LENGTH PDL_SRCS PDL_FILE_COUNT)
    message(STATUS "  [PDL] Total ${PDL_FILE_COUNT} source files selected")
endif()
