# =============================================================================
# CCM Products CMake 配置
# =============================================================================

# -----------------------------------------------------------------------------
# 库
# -----------------------------------------------------------------------------

# libccm
add_subdirectory(libs/libccm)

# libh200_am625
if(CONFIG_PROJECT_H200_AM625)
    add_subdirectory(h200_am625)
endif()

# -----------------------------------------------------------------------------
# 应用程序
# -----------------------------------------------------------------------------

# ccm_collector
if(CONFIG_BUILD_CCM_COLLECTOR)
    add_subdirectory(apps/ccm_collector)
endif()

# ccm_health
if(CONFIG_BUILD_CCM_HEALTH)
    add_subdirectory(apps/ccm_health)
endif()

# ccm_logger
if(CONFIG_BUILD_CCM_LOGGER)
    add_subdirectory(apps/ccm_logger)
endif()

# ccm_supervisor
if(CONFIG_BUILD_CCM_SUPERVISOR)
    add_subdirectory(apps/ccm_supervisor)
endif()

# ccm_comm
if(CONFIG_BUILD_CCM_COMM)
    add_subdirectory(apps/ccm_comm)
endif()
