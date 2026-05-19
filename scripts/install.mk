# ============================================================================
# 安装规则（类似 Linux 内核的 make install 和 make headers_install）
# ============================================================================

INSTALL_PREFIX ?= /usr/local
DESTDIR ?=

# 安装目录
INSTALL_LIB_DIR := $(DESTDIR)$(INSTALL_PREFIX)/lib
INSTALL_BIN_DIR := $(DESTDIR)$(INSTALL_PREFIX)/bin
INSTALL_HDR_DIR := $(DESTDIR)$(INSTALL_PREFIX)/include/ems

# ============================================================================
# 主安装目标
# ============================================================================
.PHONY: install install-libs install-bins install-headers

install: install-libs install-bins
	@echo "Installation complete to $(DESTDIR)$(INSTALL_PREFIX)"

# ============================================================================
# 安装库文件
# ============================================================================
install-libs:
	@echo "Installing libraries to $(INSTALL_LIB_DIR)..."
	@install -d $(INSTALL_LIB_DIR)
	@install -m 644 $(objtree)/lib/*.so $(INSTALL_LIB_DIR)/ 2>/dev/null || true
	@install -m 644 $(objtree)/lib/*.a $(INSTALL_LIB_DIR)/ 2>/dev/null || true
	@echo "Libraries installed."

# ============================================================================
# 安装可执行文件
# ============================================================================
install-bins:
	@echo "Installing binaries to $(INSTALL_BIN_DIR)..."
	@install -d $(INSTALL_BIN_DIR)
	@install -m 755 $(objtree)/bin/* $(INSTALL_BIN_DIR)/ 2>/dev/null || true
	@echo "Binaries installed."

# ============================================================================
# 安装头文件（类似 Linux 内核的 headers_install）
# ============================================================================
install-headers: headers_install

headers_install:
	@echo "Installing headers to $(INSTALL_HDR_DIR)..."
	@install -d $(INSTALL_HDR_DIR)

	# 安装 OSAL 头文件
	@echo "  Installing OSAL headers..."
	@install -d $(INSTALL_HDR_DIR)/osal
	@cp -r $(srctree)/core/osal/include/* $(INSTALL_HDR_DIR)/osal/

	# 安装 HAL 头文件
	@echo "  Installing HAL headers..."
	@install -d $(INSTALL_HDR_DIR)/hal
	@cp -r $(srctree)/core/hal/include/* $(INSTALL_HDR_DIR)/hal/

	# 安装 PCL 头文件
	@echo "  Installing PCL headers..."
	@install -d $(INSTALL_HDR_DIR)/pcl
	@cp -r $(srctree)/core/pcl/include/* $(INSTALL_HDR_DIR)/pcl/

	# 安装 PDL 头文件
	@echo "  Installing PDL headers..."
	@install -d $(INSTALL_HDR_DIR)/pdl
	@cp -r $(srctree)/core/pdl/include/* $(INSTALL_HDR_DIR)/pdl/

	# 安装 ACL 头文件
	@echo "  Installing ACL headers..."
	@install -d $(INSTALL_HDR_DIR)/acl
	@cp -r $(srctree)/core/acl/include/* $(INSTALL_HDR_DIR)/acl/

	# 安装 Kconfig 生成的配置头文件
	@if [ -f $(GENERATED_DIR)/autoconf.h ]; then \
		echo "  Installing generated config headers..."; \
		install -d $(INSTALL_HDR_DIR)/generated; \
		install -m 644 $(GENERATED_DIR)/autoconf.h $(INSTALL_HDR_DIR)/generated/; \
	fi
	@if [ -f $(srctree)/include/ems_config.h ]; then \
		install -m 644 $(srctree)/include/ems_config.h $(INSTALL_HDR_DIR)/; \
	fi

	# 清理不需要的文件（类似内核的 headers_install）
	@echo "  Cleaning up unnecessary files..."
	@find $(INSTALL_HDR_DIR) -name '*.c' -delete 2>/dev/null || true
	@find $(INSTALL_HDR_DIR) -name '.gitignore' -delete 2>/dev/null || true
	@find $(INSTALL_HDR_DIR) -name 'Makefile' -delete 2>/dev/null || true
	@find $(INSTALL_HDR_DIR) -name 'CMakeLists.txt' -delete 2>/dev/null || true

	@echo "Headers installed."

# ============================================================================
# 完整安装（包含头文件）
# ============================================================================
install-all: install headers_install
	@echo "Full installation complete (libs + bins + headers)"
