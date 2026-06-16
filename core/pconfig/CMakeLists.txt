################# Add include #################
# 公共头文件目录（对外暴露）
list(APPEND ADD_INCLUDE "include")
list(APPEND ADD_INCLUDE "include/pconfig")

# 私有头文件目录（仅内部使用）
list(APPEND ADD_PRIV_INCLUDE "src")

# 添加平台配置头文件路径（如果有）
if(PCONFIG_EXTRA_INCLUDE)
    list(APPEND ADD_INCLUDE ${PCONFIG_EXTRA_INCLUDE})
endif()
###############################################

############## Add source files ###############
# PCONFIG 核心源文件
list(APPEND ADD_SRCS
    "src/pconfig_api.c"
    "src/pconfig_register.c"
)

# 添加平台配置源文件（如果有）
if(PCONFIG_EXTRA_SRCS)
    list(APPEND ADD_SRCS ${PCONFIG_EXTRA_SRCS})
endif()
###############################################

###### Add required/dependent components ######
# PCONFIG 定义所有硬件配置类型，PDL 使用这些类型
# 依赖关系：PDL→PCONFIG（使用配置类型 + 查询配置）
# 无循环依赖：PCONFIG 不再依赖 PDL

# OSAL 是必须的（Kconfig: select OSAL）
if(CONFIG_OSAL)
    list(APPEND ADD_REQUIREMENTS osal)
else()
    message(FATAL_ERROR "PCONFIG requires OSAL but CONFIG_OSAL is not enabled")
endif()

# HAL 是 Kconfig 强制依赖的（Kconfig: select HAL）
if(CONFIG_HAL)
    list(APPEND ADD_REQUIREMENTS hal)
endif()

# PCONFIG 不再依赖 PDL（配置类型已在 PCONFIG 中定义）
###############################################

#### Add compile option for this component ####
# 平台配置通过 CONFIG_PROJECT_NAME 由产品层提供
###############################################

# 注册组件
if(CONFIG_PCONFIG_BUILD_SHARED)
    register_component(SHARED)
else()
    register_component()
endif()
