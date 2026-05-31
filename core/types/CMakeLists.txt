################# Add include #################
# Types 模块只包含头文件，不包含源文件
###############################################

############## Add source files ###############
# 无源文件（仅头文件库）
###############################################

###### Add required/dependent components ######
# Types 只依赖 OSAL（用于 osal_types.h）
list(APPEND ADD_REQUIREMENTS osal)
###############################################

#### Add compile option for this component ####
# 无特殊编译选项
###############################################

# 注册组件（INTERFACE 库，仅头文件）
list(APPEND ADD_INCLUDE "include")
get_filename_component(PARENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)
list(APPEND ADD_INCLUDE "${PARENT_DIR}")

# Types 是纯头文件库，使用 INTERFACE 模式
register_component(INTERFACE)
