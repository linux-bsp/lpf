# ==============================================================================
# Compiler Flags Configuration from Kconfig
# ==============================================================================
# This module applies compiler flags based on Kconfig configuration.
# It works with the build wrapper layer to provide consistent compilation.
#
# IMPORTANT: Do not clear CMAKE_C_FLAGS - append to existing flags instead
# ==============================================================================

########## set C flags from Kconfig configuration #########

# Use a temporary list to collect flags, then append to CMAKE_C_FLAGS
set(KCONFIG_C_FLAGS)

# Add warning flags from Kconfig
if(CONFIG_COMPILER_WALL)
    list(APPEND KCONFIG_C_FLAGS -Wall)
endif()

if(CONFIG_COMPILER_WEXTRA)
    list(APPEND KCONFIG_C_FLAGS -Wextra)
endif()

if(CONFIG_COMPILER_PEDANTIC)
    list(APPEND KCONFIG_C_FLAGS -Wpedantic)
endif()

if(CONFIG_COMPILER_WARNINGS_AS_ERRORS)
    list(APPEND KCONFIG_C_FLAGS -Werror)
endif()

# Add optimization level
if(CONFIG_COMPILER_OPTIMIZATION_LEVEL)
    list(APPEND KCONFIG_C_FLAGS ${CONFIG_COMPILER_OPTIMIZATION_LEVEL})
else()
    if(CONFIG_BUILD_TYPE_DEBUG)
        list(APPEND KCONFIG_C_FLAGS -O0)
        # Disable _FORTIFY_SOURCE when using -O0 (it requires optimization)
        # This is common in Buildroot/Yocto environments where toolchain adds it by default
        list(APPEND KCONFIG_C_FLAGS -U_FORTIFY_SOURCE)
    elseif(CONFIG_BUILD_TYPE_RELEASE OR CONFIG_BUILD_TYPE_RELWITHDEBINFO)
        list(APPEND KCONFIG_C_FLAGS -O2)
    elseif(CONFIG_BUILD_TYPE_MINSIZEREL)
        list(APPEND KCONFIG_C_FLAGS -Os)
    endif()
endif()

# Add debug information
if(CONFIG_COMPILER_DEBUG_LEVEL)
    list(APPEND KCONFIG_C_FLAGS ${CONFIG_COMPILER_DEBUG_LEVEL})
endif()

# Add sanitizers
if(CONFIG_COMPILER_SANITIZER_ADDRESS)
    list(APPEND KCONFIG_C_FLAGS -fsanitize=address)
    list(APPEND CMAKE_C_LINK_FLAGS -fsanitize=address)
endif()

if(CONFIG_COMPILER_SANITIZER_THREAD)
    list(APPEND KCONFIG_C_FLAGS -fsanitize=thread)
    list(APPEND CMAKE_C_LINK_FLAGS -fsanitize=thread)
endif()

if(CONFIG_COMPILER_SANITIZER_UNDEFINED)
    list(APPEND KCONFIG_C_FLAGS -fsanitize=undefined)
    list(APPEND CMAKE_C_LINK_FLAGS -fsanitize=undefined)
endif()

# Add coverage
if(CONFIG_COMPILER_COVERAGE)
    list(APPEND KCONFIG_C_FLAGS -fprofile-arcs -ftest-coverage)
    list(APPEND CMAKE_C_LINK_FLAGS --coverage)
endif()

# Add LTO
if(CONFIG_COMPILER_LTO)
    list(APPEND KCONFIG_C_FLAGS -flto)
    list(APPEND CMAKE_C_LINK_FLAGS -flto)
endif()

# Add position independent code
if(CONFIG_POSITION_INDEPENDENT_CODE)
    list(APPEND KCONFIG_C_FLAGS -fPIC)
endif()

# Add stack protector
if(CONFIG_STACK_PROTECTOR)
    list(APPEND KCONFIG_C_FLAGS -fstack-protector-strong)
endif()

# Add custom C flags (split by spaces)
if(CONFIG_COMPILER_CUSTOM_C_FLAGS)
    separate_arguments(CUSTOM_C_FLAGS_LIST UNIX_COMMAND "${CONFIG_COMPILER_CUSTOM_C_FLAGS}")
    list(APPEND KCONFIG_C_FLAGS ${CUSTOM_C_FLAGS_LIST})
endif()

################################


###### set CXX(cpp) flags from Kconfig configuration ######

# Use a temporary list to collect flags, then append to CMAKE_CXX_FLAGS
set(KCONFIG_CXX_FLAGS)

# Add warning flags from Kconfig
if(CONFIG_COMPILER_WALL)
    list(APPEND KCONFIG_CXX_FLAGS -Wall)
endif()

if(CONFIG_COMPILER_WEXTRA)
    list(APPEND KCONFIG_CXX_FLAGS -Wextra)
endif()

if(CONFIG_COMPILER_PEDANTIC)
    list(APPEND KCONFIG_CXX_FLAGS -Wpedantic)
endif()

if(CONFIG_COMPILER_WARNINGS_AS_ERRORS)
    list(APPEND KCONFIG_CXX_FLAGS -Werror)
endif()

# Add optimization level
if(CONFIG_COMPILER_OPTIMIZATION_LEVEL)
    list(APPEND KCONFIG_CXX_FLAGS ${CONFIG_COMPILER_OPTIMIZATION_LEVEL})
else()
    if(CONFIG_BUILD_TYPE_DEBUG)
        list(APPEND KCONFIG_CXX_FLAGS -O0)
        # Disable _FORTIFY_SOURCE when using -O0 (it requires optimization)
        # This is common in Buildroot/Yocto environments where toolchain adds it by default
        list(APPEND KCONFIG_CXX_FLAGS -U_FORTIFY_SOURCE)
    elseif(CONFIG_BUILD_TYPE_RELEASE OR CONFIG_BUILD_TYPE_RELWITHDEBINFO)
        list(APPEND KCONFIG_CXX_FLAGS -O2)
    elseif(CONFIG_BUILD_TYPE_MINSIZEREL)
        list(APPEND KCONFIG_CXX_FLAGS -Os)
    endif()
endif()

# Add debug information
if(CONFIG_COMPILER_DEBUG_LEVEL)
    list(APPEND KCONFIG_CXX_FLAGS ${CONFIG_COMPILER_DEBUG_LEVEL})
endif()

# Add sanitizers
if(CONFIG_COMPILER_SANITIZER_ADDRESS)
    list(APPEND KCONFIG_CXX_FLAGS -fsanitize=address)
endif()

if(CONFIG_COMPILER_SANITIZER_THREAD)
    list(APPEND KCONFIG_CXX_FLAGS -fsanitize=thread)
endif()

if(CONFIG_COMPILER_SANITIZER_UNDEFINED)
    list(APPEND KCONFIG_CXX_FLAGS -fsanitize=undefined)
endif()

# Add coverage
if(CONFIG_COMPILER_COVERAGE)
    list(APPEND KCONFIG_CXX_FLAGS -fprofile-arcs -ftest-coverage)
endif()

# Add LTO
if(CONFIG_COMPILER_LTO)
    list(APPEND KCONFIG_CXX_FLAGS -flto)
endif()

# Add position independent code
if(CONFIG_POSITION_INDEPENDENT_CODE)
    list(APPEND KCONFIG_CXX_FLAGS -fPIC)
endif()

# Add stack protector
if(CONFIG_STACK_PROTECTOR)
    list(APPEND KCONFIG_CXX_FLAGS -fstack-protector-strong)
endif()

# Add custom C++ flags (split by spaces)
if(CONFIG_COMPILER_CUSTOM_CXX_FLAGS)
    separate_arguments(CUSTOM_CXX_FLAGS_LIST UNIX_COMMAND "${CONFIG_COMPILER_CUSTOM_CXX_FLAGS}")
    list(APPEND KCONFIG_CXX_FLAGS ${CUSTOM_CXX_FLAGS_LIST})
endif()

################################


###### set linker flags ######

# Add custom linker flags
if(CONFIG_COMPILER_CUSTOM_LINK_FLAGS)
    separate_arguments(CUSTOM_LINK_FLAGS_LIST UNIX_COMMAND "${CONFIG_COMPILER_CUSTOM_LINK_FLAGS}")
    list(APPEND CMAKE_C_LINK_FLAGS ${CUSTOM_LINK_FLAGS_LIST})
endif()

set(CMAKE_CXX_LINK_FLAGS ${CMAKE_C_LINK_FLAGS})

################################


# ==============================================================================
# Apply collected flags to CMAKE variables
# ==============================================================================

# Convert lists to strings and append to existing CMAKE_*_FLAGS
if(KCONFIG_C_FLAGS)
    string(REPLACE ";" " " KCONFIG_C_FLAGS_STR "${KCONFIG_C_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${KCONFIG_C_FLAGS_STR}")
    string(STRIP "${CMAKE_C_FLAGS}" CMAKE_C_FLAGS)
    message(STATUS "Kconfig C flags: ${KCONFIG_C_FLAGS_STR}")
endif()

if(KCONFIG_CXX_FLAGS)
    string(REPLACE ";" " " KCONFIG_CXX_FLAGS_STR "${KCONFIG_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KCONFIG_CXX_FLAGS_STR}")
    string(STRIP "${CMAKE_CXX_FLAGS}" CMAKE_CXX_FLAGS)
    message(STATUS "Kconfig C++ flags: ${KCONFIG_CXX_FLAGS_STR}")
endif()

if(CMAKE_C_LINK_FLAGS)
    string(REPLACE ";" " " CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS}")
    string(STRIP "${CMAKE_C_LINK_FLAGS}" CMAKE_C_LINK_FLAGS)
endif()

if(CMAKE_CXX_LINK_FLAGS)
    string(REPLACE ";" " " CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS}")
    string(STRIP "${CMAKE_CXX_LINK_FLAGS}" CMAKE_CXX_LINK_FLAGS)
endif()

