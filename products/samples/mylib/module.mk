# =============================================================================
# 库模板 - 复制此文件创建新库
# =============================================================================
# 使用说明：
# 1. 复制此文件到你的库目录
# 2. 将所有 "mylib" 替换为你的库名（小写）
# 3. 将所有 "MYLIB" 替换为你的库名（大写）
# 4. 修改源文件列表、头文件路径、依赖库
# 5. 在顶层 Makefile 中添加：include products/xxx/mylib/module.mk
# =============================================================================

# -----------------------------------------------------------------------------
# 第 1 步：定义源文件（必填）
# -----------------------------------------------------------------------------
# 列出所有 .c 源文件的完整路径
mylib_SRCS := \
	products/samples/mylib/src/mylib_api.c \
	products/samples/mylib/src/mylib_internal.c

# -----------------------------------------------------------------------------
# 第 2 步：定义头文件路径（必填）
# -----------------------------------------------------------------------------
# 添加本模块的头文件目录
mylib_CFLAGS := \
	-Iproducts/samples/mylib/include

# 添加依赖模块的头文件（从 staging 目录）
mylib_CFLAGS += \
	-Iinclude/osal

# -----------------------------------------------------------------------------
# 第 3 步：定义链接库（动态库需要）
# -----------------------------------------------------------------------------
# 如果构建动态库，需要链接依赖的库
mylib_LDFLAGS := \
	-L$(STAGING_DIR)/lib \
	-Wl,--no-as-needed \
	-losal \
	-Wl,--as-needed

# 设置 SONAME（动态库版本管理）
mylib_LDFLAGS += -Wl,-soname,libmylib.so.1

# -----------------------------------------------------------------------------
# 第 4 步：配置开关（可选）
# -----------------------------------------------------------------------------
# 根据 Kconfig 配置决定构建类型
# 在 Kconfig 中添加：
#   config MYLIB
#   config MYLIB_BUILD_STATIC
#   config MYLIB_BUILD_SHARED

ifeq ($(CONFIG_MYLIB),y)
  MYLIB_ENABLED := y
else
  MYLIB_ENABLED := n
endif

# -----------------------------------------------------------------------------
# 第 5 步：导出头文件（可选，推荐）
# -----------------------------------------------------------------------------
# 列出需要安装到 staging 目录的头文件
# 路径相对于 products/samples/mylib/include/
mylib_HEADERS := \
	mylib.h \
	mylib_types.h

# -----------------------------------------------------------------------------
# 以下内容通常不需要修改（标准构建流程）
# -----------------------------------------------------------------------------

# 生成目标文件列表
mylib_OBJS := $(call srcs_to_objs,$(mylib_SRCS))

# 定义库文件路径
ifeq ($(MYLIB_ENABLED),y)
  ifeq ($(CONFIG_MYLIB_BUILD_SHARED),y)
    mylib_SO_TARGET := $(STAGING_DIR)/lib/libmylib.so
    ALL_TARGETS += $(mylib_SO_TARGET)
  endif

  ifeq ($(CONFIG_MYLIB_BUILD_STATIC),y)
    mylib_A_TARGET := $(STAGING_DIR)/lib/libmylib.a
    ALL_TARGETS += $(mylib_A_TARGET)
  endif
endif

# 添加编译标志
$(mylib_OBJS): CFLAGS += $(mylib_CFLAGS)

# 定义构建规则
ifeq ($(MYLIB_ENABLED),y)

# 动态库构建
ifeq ($(CONFIG_MYLIB_BUILD_SHARED),y)
$(mylib_SO_TARGET): $(mylib_OBJS)
$(mylib_SO_TARGET): $(STAGING_DIR)/lib/libosal.so

$(mylib_SO_TARGET):
	@echo "  LD      $@"
	@mkdir -p $(dir $@)
	@$(CC) -shared -o $@ $(mylib_OBJS) $(mylib_LDFLAGS)
	@if [ -n "$(mylib_LDFLAGS)" ] && echo "$(mylib_LDFLAGS)" | grep -q "soname,"; then \
		soname=$$(echo "$(mylib_LDFLAGS)" | sed -n 's/.*-soname,\([^ ]*\).*/\1/p'); \
		if [ -n "$$soname" ] && [ "$$soname" != "$$(basename $@)" ]; then \
			ln -sf $$(basename $@) $$(dirname $@)/$$soname; \
		fi; \
	fi
endif

# 静态库构建
ifeq ($(CONFIG_MYLIB_BUILD_STATIC),y)
$(mylib_A_TARGET): $(mylib_OBJS)

$(mylib_A_TARGET):
	@echo "  AR      $@"
	@mkdir -p $(dir $@)
	@rm -f $@
	@ar rcs $@ $(mylib_OBJS)
endif

# 安装头文件到 staging 目录
ifneq ($(mylib_HEADERS),)
$(mylib_SO_TARGET) $(mylib_A_TARGET): | install_mylib_headers

.PHONY: install_mylib_headers
install_mylib_headers:
	@mkdir -p $(STAGING_DIR)/include/mylib
	@for header in $(mylib_HEADERS); do \
		src="products/samples/mylib/include/$$header"; \
		dst="$(STAGING_DIR)/include/mylib/$$header"; \
		mkdir -p $$(dirname $$dst); \
		cp -f $$src $$dst; \
	done
endif

endif

# 清理规则
CLEAN_TARGETS += $(mylib_OBJS) $(mylib_SO_TARGET) $(mylib_A_TARGET)
