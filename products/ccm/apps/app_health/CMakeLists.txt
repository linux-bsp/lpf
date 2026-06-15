# ==============================================================================
# CCM Health Application
# ==============================================================================
# This application monitors system health and diagnostics for the CCM system.
# ==============================================================================

# 构建依赖列表（根据 Kconfig 配置）
set(HEALTH_REQUIREMENTS)

# 必须的依赖
if(CONFIG_OSAL)
    list(APPEND HEALTH_REQUIREMENTS osal)
endif()

# libccm 产品库
list(APPEND HEALTH_REQUIREMENTS libccm)

# 可选依赖（根据配置启用）
if(CONFIG_HAL)
    list(APPEND HEALTH_REQUIREMENTS hal)
endif()

if(CONFIG_PCONFIG)
    list(APPEND HEALTH_REQUIREMENTS pconfig)
endif()

if(CONFIG_PDL)
    list(APPEND HEALTH_REQUIREMENTS pdl)
endif()

if(CONFIG_ACONFIG)
    list(APPEND HEALTH_REQUIREMENTS aconfig)
endif()

# Register CCM health application using simplified API
register_app_component(
    PRODUCT ccm
    REQUIREMENTS ${HEALTH_REQUIREMENTS}
)
