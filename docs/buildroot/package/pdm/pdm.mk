################################################################################
#
# lpf
#
################################################################################

# Default: use the canonical PDM git repository.
# Product trees can set PDM_OVERRIDE_SRCDIR in BR2_PACKAGE_OVERRIDE_FILE for
# local development or board integration testing.
PDM_VERSION = v1.0.0
PDM_SITE = ssh://gitea@192.168.18.254:4022/CSPD/PDM.git
PDM_SITE_METHOD = git
PDM_LICENSE = Proprietary
PDM_INSTALL_TARGET = YES
PDM_ADD_TOOLCHAIN_DEPENDENCY = NO

# Dependencies - Kconfig + CMake/Kbuild build system
PDM_DEPENDENCIES = host-pkgconf host-cmake host-flex host-bison linux

# Kconfig configuration to use (from Buildroot config)
PDM_KCONFIG_DEFCONFIG = $(call qstrip,$(BR2_PACKAGE_LPF_DEFCONFIG))

# Build type from Buildroot configuration
PDM_BUILD_TYPE = $(call qstrip,$(BR2_PACKAGE_LPF_BUILD_TYPE))

# Build in-tree for Buildroot (output/build/pdm/_build)
PDM_BUILD_OUTPUT = $(@D)/_build
PDM_MODULES_OUTPUT = $(@D)/_build/modules
PDM_MODULE_EXTRA_DIR = extra/lpf

PDM_MAKE_OPTS = \
	BUILD_DIR="$(PDM_BUILD_OUTPUT)" \
	MODULES_BUILD_DIR="$(PDM_MODULES_OUTPUT)" \
	BR2_EXTERNAL=1 \
	KERNEL_SRC="$(LINUX_DIR)" \
	ARCH="$(KERNEL_ARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	CMAKE_BUILD_TYPE="$(PDM_BUILD_TYPE)" \
	CMAKE_TOOLCHAIN_FILE="$(HOST_DIR)/share/buildroot/toolchainfile.cmake" \
	CMAKE_EXTRA_FLAGS='-DCMAKE_C_COMPILER="$(TARGET_CC)" -DCMAKE_C_FLAGS="$(TARGET_CFLAGS)" -DCMAKE_EXE_LINKER_FLAGS="$(TARGET_LDFLAGS)"' \
	CMAKE_INSTALL_PREFIX="/usr"

# Configure using make (kernel-style interface)
# This loads the defconfig and generates .config + autoconf.h
define PDM_CONFIGURE_CMDS
	@test -n "$(PDM_KCONFIG_DEFCONFIG)" || { \
		echo "PDM: BR2_PACKAGE_LPF_DEFCONFIG must be set"; \
		exit 1; \
	}
	@echo "PDM: Loading defconfig $(PDM_KCONFIG_DEFCONFIG)"
	$(MAKE) -C $(@D) $(PDM_MAKE_OPTS) $(PDM_KCONFIG_DEFCONFIG)
	@echo "PDM: Configuration loaded successfully"
endef

# Build userspace libraries and kernel modules.
define PDM_BUILD_CMDS
	@echo "PDM: Building with configuration $(PDM_KCONFIG_DEFCONFIG)"
	rm -f "$(PDM_BUILD_OUTPUT)/CMakeCache.txt"
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(PDM_MAKE_OPTS) all
endef

# Clean build artifacts (keep configuration)
define PDM_CLEAN_CMDS
	@echo "PDM: Cleaning build artifacts"
	-$(MAKE) -C $(@D) clean
endef

# Complete clean (remove configuration and build directory)
define PDM_DISTCLEAN_CMDS
	@echo "PDM: Complete clean"
	-$(MAKE) -C $(@D) distclean
endef

# Install userspace libraries and kernel modules to target.
define PDM_INSTALL_TARGET_CMDS
	@echo "PDM: Installing to target"
	$(MAKE) -C $(@D) \
		BUILD_DIR="$(PDM_BUILD_OUTPUT)" \
		CMAKE_INSTALL_PREFIX="/usr" \
		install DESTDIR=$(TARGET_DIR)
	kernel_release="$(LINUX_VERSION_PROBED)"; \
	if [ -z "$$kernel_release" ]; then \
		kernel_release="$$($(MAKE) -C "$(LINUX_DIR)" \
			ARCH="$(KERNEL_ARCH)" \
			CROSS_COMPILE="$(TARGET_CROSS)" \
			--no-print-directory -s kernelrelease)"; \
	fi; \
	module_install_dir="$(TARGET_DIR)/lib/modules/$$kernel_release/$(PDM_MODULE_EXTRA_DIR)"; \
	$(INSTALL) -d "$$module_install_dir"; \
	modules_found=0; \
	for module in "$(PDM_MODULES_OUTPUT)"/*.ko; do \
		[ -e "$$module" ] || continue; \
		modules_found=1; \
		$(INSTALL) -m 0644 "$$module" "$$module_install_dir/$$(basename "$$module")"; \
	done; \
	if [ "$$modules_found" -eq 0 ]; then \
		echo "PDM: no kernel modules found in $(PDM_MODULES_OUTPUT)"; \
		exit 1; \
	fi
	@echo "PDM: Target installation complete"
endef

# Optional: Install development headers and userspace libraries to staging.
ifeq ($(BR2_PACKAGE_LPF_INSTALL_HEADERS),y)
PDM_INSTALL_STAGING = YES
define PDM_INSTALL_STAGING_CMDS
	@echo "PDM: Installing libraries to staging"
	$(MAKE) -C $(@D) \
		BUILD_DIR="$(PDM_BUILD_OUTPUT)" \
		CMAKE_INSTALL_PREFIX="/usr" \
		install DESTDIR=$(STAGING_DIR)
	@echo "PDM: Installing development headers to staging"
	$(MAKE) -C $(@D) \
		BUILD_DIR="$(PDM_BUILD_OUTPUT)" \
		CMAKE_INSTALL_PREFIX="/usr" \
		install_headers DESTDIR=$(STAGING_DIR)
	@echo "PDM: Staging installation complete"
endef
endif

$(eval $(generic-package))
