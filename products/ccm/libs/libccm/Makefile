# ============================================================================
# libccm 构建
# ============================================================================

# 必须在包含 Makefile.lib 之前定义
GENERATED_DIR ?= $(objtree)/include/generated

include $(srctree)/scripts/Makefile.lib

# 库名称
LIB_NAME := libccm.a

# 输出目录
BUILD_DIR ?= $(objtree)/products/ccm/libs/libccm
LIB_DIR := $(objtree)/lib

# 源文件目录
SRC_DIR := $(srctree)/products/ccm/libs/libccm/src
INC_DIR := $(srctree)/products/ccm/libs/libccm/include

# 源文件列表
SRCS := \
	libccm_ipc.c \
	libccm_protocol.c

# 目标文件
OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))

# 编译标志
CFLAGS_MODULE := -I$(INC_DIR)
CFLAGS_MODULE += -I$(srctree)/core/osal/include
CFLAGS_MODULE += -I$(srctree)/core/hal/include

# 默认目标
.PHONY: all clean

all: $(LIB_DIR)/$(LIB_NAME)
	@echo "libccm built successfully"

# 编译规则
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
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
