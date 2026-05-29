################# Add include #################
# 使用标准的 core 组件 include 路径
# 添加 PDL 头文件路径（只使用类型定义，不链接库）
list(APPEND ADD_INCLUDE "${CMAKE_SOURCE_DIR}/core/pdl/include")

# 添加平台配置头文件路径（如果有）
if(PCL_EXTRA_INCLUDE)
    list(APPEND ADD_INCLUDE ${PCL_EXTRA_INCLUDE})
endif()
###############################################

############## Add source files ###############
# PCL 核心源文件
list(APPEND ADD_SRCS
    "src/pcl_api.c"
    "src/pcl_register.c"
)

# 添加平台配置源文件（如果有）
if(PCL_EXTRA_SRCS)
    list(APPEND ADD_SRCS ${PCL_EXTRA_SRCS})
endif()
###############################################

###### Add required/dependent components ######
# PCL 依赖 OSAL 和 HAL
# 注意：PCL 只使用 PDL 的头文件（类型定义），不需要链接 PDL 库
# 因此不添加 PDL 到依赖列表，避免循环依赖
list(APPEND ADD_REQUIREMENTS osal hal)
###############################################

#### Add compile option for this component ####
# 平台配置通过 CONFIG_PROJECT_NAME 由产品层提供
###############################################

# 注册组件（使用公共函数）
list(APPEND ADD_INCLUDE "include")
get_filename_component(PARENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)
list(APPEND ADD_INCLUDE "${PARENT_DIR}")

if(CONFIG_PCL_BUILD_SHARED)
    register_component(SHARED)
else()
    register_component()
endif()
