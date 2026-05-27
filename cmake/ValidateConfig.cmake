# =============================================================================
# 配置验证模块
# =============================================================================
# 功能：验证 Kconfig 配置项之间的依赖关系，避免无效配置
# =============================================================================

function(validate_config)
    message(STATUS "Validating configuration...")

    set(VALIDATION_ERRORS "")

    # =============================================================================
    # 1. 核心模块依赖关系验证
    # =============================================================================

    # ACL 依赖 PCL
    if(CONFIG_ACL AND NOT CONFIG_PCL)
        list(APPEND VALIDATION_ERRORS "ACL requires PCL to be enabled")
    endif()

    # ACL 依赖 HAL
    if(CONFIG_ACL AND NOT CONFIG_HAL)
        list(APPEND VALIDATION_ERRORS "ACL requires HAL to be enabled")
    endif()

    # PCL 依赖 HAL
    if(CONFIG_PCL AND NOT CONFIG_HAL)
        list(APPEND VALIDATION_ERRORS "PCL requires HAL to be enabled")
    endif()

    # HAL 依赖 OSAL
    if(CONFIG_HAL AND NOT CONFIG_OSAL)
        list(APPEND VALIDATION_ERRORS "HAL requires OSAL to be enabled")
    endif()

    # PCL 依赖 OSAL
    if(CONFIG_PCL AND NOT CONFIG_OSAL)
        list(APPEND VALIDATION_ERRORS "PCL requires OSAL to be enabled")
    endif()

    # ACL 依赖 OSAL
    if(CONFIG_ACL AND NOT CONFIG_OSAL)
        list(APPEND VALIDATION_ERRORS "ACL requires OSAL to be enabled")
    endif()

    # =============================================================================
    # 2. OSAL 子模块依赖验证
    # =============================================================================

    # 如果启用了 OSAL，至少需要启用一些基础功能
    if(CONFIG_OSAL)
        if(NOT CONFIG_OSAL_IPC AND NOT CONFIG_OSAL_THREAD AND NOT CONFIG_OSAL_FILE)
            list(APPEND VALIDATION_ERRORS "OSAL is enabled but no subsystems (IPC/THREAD/FILE) are enabled")
        endif()
    endif()

    # =============================================================================
    # 3. HAL 驱动依赖验证
    # =============================================================================

    # 如果启用了 HAL，至少需要启用一个驱动
    if(CONFIG_HAL)
        if(NOT CONFIG_HAL_CAN AND NOT CONFIG_HAL_UART AND NOT CONFIG_HAL_I2C AND
           NOT CONFIG_HAL_SPI AND NOT CONFIG_HAL_GPIO AND NOT CONFIG_HAL_WATCHDOG)
            list(APPEND VALIDATION_ERRORS "HAL is enabled but no drivers are enabled")
        endif()
    endif()

    # =============================================================================
    # 4. 库类型验证
    # =============================================================================

    # 至少需要构建一种类型的库
    if(NOT CONFIG_LIBRARY_BUILD_TYPE_STATIC AND NOT CONFIG_LIBRARY_BUILD_TYPE_DYNAMIC)
        list(APPEND VALIDATION_ERRORS "At least one library type (static or dynamic) must be enabled")
    endif()

    # =============================================================================
    # 5. 架构和操作系统验证
    # =============================================================================

    # 必须选择一个架构
    if(NOT CONFIG_ARCH_X86_64 AND NOT CONFIG_ARCH_ARM64)
        list(APPEND VALIDATION_ERRORS "No architecture selected (x86_64 or ARM64)")
    endif()

    # 不能同时选择多个架构
    if(CONFIG_ARCH_X86_64 AND CONFIG_ARCH_ARM64)
        list(APPEND VALIDATION_ERRORS "Cannot select multiple architectures")
    endif()

    # 必须选择一个操作系统
    if(NOT CONFIG_OS_LINUX)
        list(APPEND VALIDATION_ERRORS "No operating system selected")
    endif()

    # =============================================================================
    # 6. 优化级别验证
    # =============================================================================

    # 必须选择一个优化级别
    if(NOT CONFIG_OPT_O0 AND NOT CONFIG_OPT_O2 AND NOT CONFIG_OPT_O3 AND NOT CONFIG_OPT_OS)
        list(APPEND VALIDATION_ERRORS "No optimization level selected")
    endif()

    # 不能同时选择多个优化级别
    set(OPT_COUNT 0)
    if(CONFIG_OPT_O0)
        math(EXPR OPT_COUNT "${OPT_COUNT} + 1")
    endif()
    if(CONFIG_OPT_O2)
        math(EXPR OPT_COUNT "${OPT_COUNT} + 1")
    endif()
    if(CONFIG_OPT_O3)
        math(EXPR OPT_COUNT "${OPT_COUNT} + 1")
    endif()
    if(CONFIG_OPT_OS)
        math(EXPR OPT_COUNT "${OPT_COUNT} + 1")
    endif()
    if(OPT_COUNT GREATER 1)
        list(APPEND VALIDATION_ERRORS "Multiple optimization levels selected")
    endif()

    # =============================================================================
    # 7. 报告验证结果
    # =============================================================================

    list(LENGTH VALIDATION_ERRORS ERROR_COUNT)

    if(ERROR_COUNT GREATER 0)
        message(STATUS "")
        message(STATUS "=== Configuration Validation Failed ===")
        message(STATUS "Found ${ERROR_COUNT} error(s):")
        message(STATUS "")
        foreach(ERROR ${VALIDATION_ERRORS})
            message(STATUS "  ❌ ${ERROR}")
        endforeach()
        message(STATUS "")
        message(FATAL_ERROR "Configuration validation failed. Please fix the errors above.")
    else()
        message(STATUS "Configuration validation passed ✓")
    endif()
endfunction()
