# EMS Kconfig Wrapper Makefile
# This Makefile handles Kconfig operations only.
# Actual building is done by CMake (via build.sh or direct cmake commands).

# Kconfig configuration
KCONFIG_CONFIG := .config
export KCONFIG_CONFIG

CONF := scripts/kconfig/conf
MCONF := scripts/kconfig/mconf

# Directories
INCLUDE_DIR := include
CONFIG_DIR := $(INCLUDE_DIR)/config
GENERATED_DIR := $(INCLUDE_DIR)/generated

# ============================================================================
# Kconfig Targets
# ============================================================================

.PHONY: menuconfig config defconfig savedefconfig oldconfig silentoldconfig syncconfig

menuconfig: $(MCONF)
	@mkdir -p $(CONFIG_DIR) $(GENERATED_DIR)
	@echo "Starting menuconfig..."
	@$< Kconfig
	@$(MAKE) syncconfig

config: $(CONF)
	@mkdir -p $(CONFIG_DIR) $(GENERATED_DIR)
	@$< --oldaskconfig Kconfig
	@$(MAKE) syncconfig

defconfig: $(CONF)
	@mkdir -p $(CONFIG_DIR) $(GENERATED_DIR)
	@echo "Loading default configuration..."
	@$< --defconfig=defconfig Kconfig
	@$(MAKE) syncconfig

%_defconfig: configs/%_defconfig $(CONF)
	@mkdir -p $(CONFIG_DIR) $(GENERATED_DIR)
	@echo "Loading configuration: $*"
	@$(CONF) --defconfig=$< Kconfig
	@$(MAKE) syncconfig

savedefconfig: $(CONF)
	@echo "Saving minimal configuration to defconfig..."
	@$< --savedefconfig=defconfig Kconfig
	@echo "Configuration saved."

oldconfig: $(CONF)
	@mkdir -p $(CONFIG_DIR) $(GENERATED_DIR)
	@$< --oldconfig Kconfig
	@$(MAKE) syncconfig

silentoldconfig: syncconfig

syncconfig: $(CONF)
	@mkdir -p $(CONFIG_DIR) $(GENERATED_DIR)
	@$< --syncconfig Kconfig
	@echo "Configuration synchronized."

# ============================================================================
# Build Kconfig Tools
# ============================================================================

$(CONF) $(MCONF):
	@echo "Building kconfig tools..."
	@$(MAKE) -C scripts/kconfig $(notdir $@)

# ============================================================================
# CMake Integration Targets
# ============================================================================

.PHONY: cmake-configure cmake-build build rebuild

cmake-configure: syncconfig
	@echo "Configuring CMake with Kconfig settings..."
	@if [ ! -f .config ]; then \
		echo "Error: No .config found. Run 'make menuconfig' or 'make defconfig' first."; \
		exit 1; \
	fi
	@./build.sh

cmake-build: cmake-configure
	@echo "Building with CMake..."
	@cmake --build build -j$$(nproc)

build: cmake-build

rebuild: distclean defconfig build

# ============================================================================
# Clean Targets
# ============================================================================

.PHONY: clean-config distclean

clean-config:
	@echo "Cleaning Kconfig artifacts..."
	@rm -f .config .config.old
	@rm -rf $(CONFIG_DIR) $(GENERATED_DIR)

distclean: clean-config
	@echo "Cleaning kconfig tools..."
	@$(MAKE) -C scripts/kconfig clean

# ============================================================================
# Help
# ============================================================================

.PHONY: help

help:
	@echo "EMS Kconfig Configuration System"
	@echo ""
	@echo "Configuration targets:"
	@echo "  menuconfig       - Interactive configuration (ncurses)"
	@echo "  defconfig        - Load default configuration"
	@echo "  <name>_defconfig - Load preset from configs/<name>_defconfig"
	@echo "  savedefconfig    - Save minimal config to defconfig"
	@echo "  oldconfig        - Update config with new options"
	@echo "  syncconfig       - Synchronize and generate headers"
	@echo ""
	@echo "Build targets (Kconfig + CMake):"
	@echo "  build            - Configure and build with CMake"
	@echo "  cmake-configure  - Run CMake configuration"
	@echo "  cmake-build      - Build with CMake"
	@echo "  rebuild          - Clean config, load defaults, and build"
	@echo ""
	@echo "Clean targets:"
	@echo "  clean-config     - Remove .config and generated headers"
	@echo "  distclean        - Remove config and kconfig tools"
	@echo ""
	@echo "Typical workflow:"
	@echo "  1. make menuconfig      # Configure features"
	@echo "  2. make build           # Build with CMake"
	@echo "  or"
	@echo "  1. make defconfig       # Use defaults"
	@echo "  2. ./build.sh           # Build directly"
	@echo ""
	@echo "Available presets:"
	@ls -1 configs/*_defconfig 2>/dev/null | sed 's|configs/||' | sed 's|^|  |' || echo "  (none)"

.DEFAULT_GOAL := help
