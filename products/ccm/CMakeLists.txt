# ============================================================================
# CCM Product CMakeLists.txt
# ============================================================================
# CCM (Central Control Module) product build configuration.
# This file is included by the root CMakeLists.txt when CONFIG_PRODUCT_CCM=y
# ============================================================================

# Register SDK core components
if(CONFIG_OSAL)
    add_subdirectory(${SDK_PATH}/components/osal ${CMAKE_CURRENT_BINARY_DIR}/osal)
endif()

if(CONFIG_HAL)
    add_subdirectory(${SDK_PATH}/components/hal ${CMAKE_CURRENT_BINARY_DIR}/hal)
endif()

if(CONFIG_PCL)
    add_subdirectory(${SDK_PATH}/components/pcl ${CMAKE_CURRENT_BINARY_DIR}/pcl)
endif()

if(CONFIG_PDL)
    add_subdirectory(${SDK_PATH}/components/pdl ${CMAKE_CURRENT_BINARY_DIR}/pdl)
endif()

if(CONFIG_ACL)
    add_subdirectory(${SDK_PATH}/components/acl ${CMAKE_CURRENT_BINARY_DIR}/acl)
endif()

# Register product-specific components
if(CONFIG_LIBCCM)
    add_subdirectory(components/libccm)
endif()

if(CONFIG_H200_AM625)
    add_subdirectory(components/h200_am625)
endif()

# Register applications
if(CONFIG_APP_COLLECTOR)
    add_subdirectory(apps/app_collector)
endif()

if(CONFIG_APP_COMM)
    add_subdirectory(apps/app_comm)
endif()

if(CONFIG_APP_HEALTH)
    add_subdirectory(apps/app_health)
endif()

if(CONFIG_APP_LOGGER)
    add_subdirectory(apps/app_logger)
endif()

if(CONFIG_APP_SUPERVISOR)
    add_subdirectory(apps/app_supervisor)
endif()
