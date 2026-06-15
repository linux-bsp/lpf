# ==============================================================================
# CCM Logger Application
# ==============================================================================
# This application handles logging and data recording for the CCM system.
# ==============================================================================

# 构建依赖列表（根据 Kconfig 配置）
set(LOGGER_REQUIREMENTS)

# 必须的依赖
if(CONFIG_OSAL)
    list(APPEND LOGGER_REQUIREMENTS osal)
endif()

# libccm 产品库
list(APPEND LOGGER_REQUIREMENTS libccm)

# 可选依赖（根据配置启用）
if(CONFIG_HAL)
    list(APPEND LOGGER_REQUIREMENTS hal)
endif()

if(CONFIG_PCONFIG)
    list(APPEND LOGGER_REQUIREMENTS pconfig)
endif()

if(CONFIG_PDL)
    list(APPEND LOGGER_REQUIREMENTS pdl)
endif()

if(CONFIG_ACONFIG)
    list(APPEND LOGGER_REQUIREMENTS aconfig)
endif()

# Register CCM logger application using simplified API
register_app_component(
    PRODUCT ccm
    REQUIREMENTS ${LOGGER_REQUIREMENTS}
)
