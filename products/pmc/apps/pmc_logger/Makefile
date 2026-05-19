# ============================================================================
# PMC Logger 应用构建
# ============================================================================

# 必须在包含 Makefile.lib 之前定义
GENERATED_DIR ?= $(objtree)/include/generated

include $(srctree)/scripts/Makefile.lib

# 应用名称
APP_NAME := pmc_logger

# 输出目录
BUILD_DIR ?= $(objtree)/products/pmc/apps/$(APP_NAME)
BIN_DIR := $(objtree)/bin

# 源文件目录
SRC_DIR := $(srctree)/products/pmc/apps/$(APP_NAME)/src
INC_DIR := $(srctree)/products/pmc/apps/$(APP_NAME)/include

# 源文件列表
SRCS := \
	main.c \
	pmc_logger.c

# 目标文件
OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))

# 编译标志
CFLAGS_MODULE := -I$(INC_DIR)
CFLAGS_MODULE += -I$(srctree)/core/osal/include
CFLAGS_MODULE += -I$(srctree)/core/hal/include
CFLAGS_MODULE += -I$(srctree)/core/pcl/include
CFLAGS_MODULE += -I$(srctree)/core/pcl/include/api
CFLAGS_MODULE += -I$(srctree)/core/pdl/include
CFLAGS_MODULE += -I$(srctree)/core/acl/include
CFLAGS_MODULE += -I$(srctree)/products/pmc/libs/libpmc/include

# 链接库
LDLIBS := -L$(objtree)/lib
LDLIBS += -lh200_am625 -lpmc
LDLIBS += -lacl -lpdl -lpcl -lhal -losal
LDLIBS += -lpthread -lrt

# 默认目标
.PHONY: all clean

all: $(BIN_DIR)/$(APP_NAME)
	@echo "$(APP_NAME) built successfully"

# 编译规则
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "  CC      $@"
	@$(CC) $(CFLAGS) $(CFLAGS_MODULE) -c $< -o $@

# 链接规则
$(BIN_DIR)/$(APP_NAME): $(OBJS) | $(BIN_DIR)
	@echo "  LD      $@"
	@$(CC) -o $@ $^ $(LDLIBS)

# 创建输出目录
$(BIN_DIR):
	@mkdir -p $@

# 清理
clean:
	@rm -rf $(BUILD_DIR)
	@rm -f $(BIN_DIR)/$(APP_NAME)
