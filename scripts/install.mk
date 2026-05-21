# ============================================================================
# EMS 安装脚本 - 参考 Linux 内核 scripts/Makefile.headersinst
# ============================================================================

# 安装路径
INSTALL_PREFIX ?= /usr/local
INSTALL_HDR_PATH := $(INSTALL_PREFIX)/include
INSTALL_LIB_PATH := $(INSTALL_PREFIX)/lib
INSTALL_BIN_PATH := $(INSTALL_PREFIX)/bin

# DESTDIR 支持（用于打包）
ifdef DESTDIR
  INSTALL_HDR_PATH := $(DESTDIR)$(INSTALL_HDR_PATH)
  INSTALL_LIB_PATH := $(DESTDIR)$(INSTALL_LIB_PATH)
  INSTALL_BIN_PATH := $(DESTDIR)$(INSTALL_BIN_PATH)
endif

.PHONY: install headers_install install-all

# ============================================================================
# 头文件安装 - Linux 内核风格
# ============================================================================
headers_install:
	@echo "Installing headers to $(INSTALL_HDR_PATH)..."
	@mkdir -p $(INSTALL_HDR_PATH)

	# 首先导出所有模块的头文件到顶层 include/
	@echo "  EXPORT  core module headers"
	@$(MAKE) -C $(srctree)/core headers_install_all

	# 复制顶层 include/ 到安装目录（保持目录结构）
	@echo "  INSTALL headers"
	@rsync -a --include='*/' --include='*.h' --exclude='*' \
		$(srctree)/include/ $(INSTALL_HDR_PATH)/

	# 复制生成的配置头文件
	@if [ -f "$(objtree)/include/generated/autoconf.h" ]; then \
		echo "  INSTALL generated/autoconf.h"; \
		mkdir -p $(INSTALL_HDR_PATH)/generated; \
		cp $(objtree)/include/generated/autoconf.h \
		   $(INSTALL_HDR_PATH)/generated/; \
	fi

	# 复制顶层配置头文件
	@if [ -f "$(srctree)/include/ems_config.h" ]; then \
		echo "  INSTALL ems_config.h"; \
		cp $(srctree)/include/ems_config.h $(INSTALL_HDR_PATH)/; \
	fi

	@echo "Headers installed successfully to $(INSTALL_HDR_PATH)"

# ============================================================================
# 库和二进制文件安装
# ============================================================================
install:
	@echo "Installing libraries to $(INSTALL_LIB_PATH)..."
	@mkdir -p $(INSTALL_LIB_PATH)
	@if [ -d "$(objtree)/lib" ]; then \
		cp -a $(objtree)/lib/*.so $(INSTALL_LIB_PATH)/ 2>/dev/null || true; \
		cp -a $(objtree)/lib/*.a $(INSTALL_LIB_PATH)/ 2>/dev/null || true; \
		echo "Libraries installed successfully"; \
	fi

	@echo "Installing binaries to $(INSTALL_BIN_PATH)..."
	@mkdir -p $(INSTALL_BIN_PATH)
	@if [ -d "$(objtree)/bin" ]; then \
		cp -a $(objtree)/bin/* $(INSTALL_BIN_PATH)/ 2>/dev/null || true; \
		echo "Binaries installed successfully"; \
	fi

# ============================================================================
# 完整安装（库 + 二进制 + 头文件）
# ============================================================================
install-all: install headers_install
	@echo "Full installation complete"
	@echo "  Headers: $(INSTALL_HDR_PATH)"
	@echo "  Libraries: $(INSTALL_LIB_PATH)"
	@echo "  Binaries: $(INSTALL_BIN_PATH)"
