# ============================================================================
# H200_AM625 产品配置构建
# ============================================================================

# 必须在包含 Makefile.lib 之前定义
GENERATED_DIR ?= $(objtree)/include/generated

include $(srctree)/scripts/Makefile.lib

# 模块名称
MODULE := h200_am625
LIB_NAME := libh200_am625.a

# 输出目录
BUILD_DIR ?= $(objtree)/products/pmc/$(MODULE)
LIB_DIR := $(objtree)/lib

# 源文件目录
PCL_SRC_DIR := $(srctree)/products/pmc/$(MODULE)/pcl
ACL_SRC_DIR := $(srctree)/products/pmc/$(MODULE)/acl

# PCL 源文件
PCL_SRCS := \
	pcl_h200_100p_base.c \
	pcl_h200_100p_v1.c \
	pcl_h200_100p_v2.c

# ACL 源文件（仅包含 CMake 实际编译的文件）
ACL_SRCS := \
	pmc_acl_config.c

# 所有源文件
SRCS := $(PCL_SRCS) $(ACL_SRCS)

# 目标文件
PCL_OBJS := $(patsubst %.c,$(BUILD_DIR)/pcl/%.o,$(PCL_SRCS))
ACL_OBJS := $(patsubst %.c,$(BUILD_DIR)/acl/%.o,$(ACL_SRCS))
OBJS := $(PCL_OBJS) $(ACL_OBJS)

# 编译标志
CFLAGS_MODULE := -I$(srctree)/core/osal/include
CFLAGS_MODULE += -I$(srctree)/core/hal/include
CFLAGS_MODULE += -I$(srctree)/core/pcl/include
CFLAGS_MODULE += -I$(srctree)/core/pcl/include/api
CFLAGS_MODULE += -I$(srctree)/core/pdl/include
CFLAGS_MODULE += -I$(srctree)/core/acl/include
CFLAGS_MODULE += -I$(srctree)/products/pmc/$(MODULE)/acl

# 默认目标
.PHONY: all clean

all: $(LIB_DIR)/$(LIB_NAME)
	@echo "H200_AM625 product config built successfully"

# PCL 编译规则
$(BUILD_DIR)/pcl/%.o: $(PCL_SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "  CC      $@"
	@$(CC) $(CFLAGS) $(CFLAGS_MODULE) -c $< -o $@

# ACL 编译规则
$(BUILD_DIR)/acl/%.o: $(ACL_SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "  CC      $@"
	@$(CC) $(CFLAGS) $(CFLAGS_MODULE) -c $< -o $@

# 静态库链接规则
$(LIB_DIR)/$(LIB_NAME): $(OBJS) | $(LIB_DIR)
	@echo "  AR      $@"
	@$(AR) rcs $@ $^

# 创建输出目录
$(LIB_DIR):
	@mkdir -p $@

# 清理
clean:
	@rm -rf $(BUILD_DIR)
	@rm -f $(LIB_DIR)/$(LIB_NAME)
