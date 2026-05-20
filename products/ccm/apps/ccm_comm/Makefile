# ============================================================================
# CCM Application Build
# ============================================================================

# 必须在包含 Makefile.app 之前定义
GENERATED_DIR ?= $(objtree)/include/generated

# 应用配置
APP_NAME := ccm_comm

# 输出目录（必须在 include 之前定义）
BUILD_DIR := $(objtree)/build/products/ccm/apps/$(APP_NAME)

# 源文件目录（必须在 include 之前定义）
SRC_DIR := $(srctree)/products/ccm/apps/$(APP_NAME)/src
INC_DIR := $(srctree)/products/ccm/apps/$(APP_NAME)/include

# 源文件列表（必须在 include 之前定义）
SRCS := \
	main.c \
	ccm_comm.c

# 编译标志（必须在 include 之前定义）
CFLAGS_APP += -I$(srctree)/core/osal/include
CFLAGS_APP += -I$(srctree)/core/hal/include
CFLAGS_APP += -I$(srctree)/core/pcl/include
CFLAGS_APP += -I$(srctree)/core/pcl/include/api
CFLAGS_APP += -I$(srctree)/core/pdl/include
CFLAGS_APP += -I$(srctree)/core/acl/include
CFLAGS_APP += -I$(srctree)/products/ccm/libs/libccm/include

# 链接库（必须在 include 之前定义）
LDLIBS := -L$(objtree)/lib
LDLIBS += -lh200_am625 -lccm
LDLIBS += -lacl -lpdl -lpcl -lhal -losal
LDLIBS += -lpthread -lrt

include $(scriptdir)/build/Makefile.app
