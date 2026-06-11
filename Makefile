# ES-Middleware Makefile
# Convenient wrapper for CMake-based build system
# No Python dependency required

.PHONY: help config menuconfig nconfig build clean distclean savedefconfig oldconfig list test

# Configuration
BUILD_DIR := _build
CONFIG_FILE := .config
CONFIGS_DIR := configs

# Default target
.DEFAULT_GOAL := help

# Detect number of CPU cores
NPROC := $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

help:
	@echo "ES-Middleware Build System"
	@echo "=========================="
	@echo ""
	@echo "Configuration:"
	@echo "  make config CONFIG=<name>  Load configuration (e.g., tests_x86_minimal)"
	@echo "  make menuconfig            Open configuration UI (ncurses)"
	@echo "  make nconfig               Open alternative configuration UI"
	@echo "  make savedefconfig [NAME=] Save configuration as defconfig"
	@echo "  make oldconfig             Update configuration with new options"
	@echo "  make list                  List available configurations"
	@echo ""
	@echo "Building:"
	@echo "  make build                 Build project (parallel)"
	@echo "  make build JOBS=N          Build with N parallel jobs"
	@echo "  make all                   Configure and build in one step"
	@echo ""
	@echo "Cleaning:"
	@echo "  make clean                 Clean build directory"
	@echo "  make distclean             Clean build + configuration"
	@echo ""
	@echo "Testing:"
	@echo "  make test                  Run tests"
	@echo ""
	@echo "Examples:"
	@echo "  make config CONFIG=tests_x86_minimal"
	@echo "  make build"
	@echo "  make all CONFIG=ccm_h200_100p_am625_debug"
	@echo ""

# List available configurations
list:
	@echo "Available configurations:"
	@echo ""
	@for config in $(wildcard $(CONFIGS_DIR)/*_defconfig); do \
		name=$$(basename $$config _defconfig); \
		echo "  $$name"; \
	done
	@echo ""
	@echo "Usage: make config CONFIG=<name>"

# Load configuration
config:
ifndef CONFIG
	@echo "Error: CONFIG not specified"
	@echo "Usage: make config CONFIG=<name>"
	@echo ""
	@echo "Available configurations:"
	@$(MAKE) list
	@exit 1
endif
	@config_file="$(CONFIGS_DIR)/$(CONFIG)_defconfig"; \
	if [ ! -f "$$config_file" ]; then \
		config_file="$(CONFIGS_DIR)/$(CONFIG).defconfig"; \
	fi; \
	if [ ! -f "$$config_file" ]; then \
		config_file="$(CONFIGS_DIR)/$(CONFIG)"; \
	fi; \
	if [ ! -f "$$config_file" ]; then \
		echo "Error: Configuration not found: $(CONFIG)"; \
		echo ""; \
		echo "Available configurations:"; \
		$(MAKE) list; \
		exit 1; \
	fi; \
	echo "Loading configuration: $$config_file"; \
	cp "$$config_file" $(CONFIG_FILE); \
	echo "✓ Configuration loaded"

# Interactive configuration (menuconfig)
menuconfig: _ensure_build_dir
	@cd $(BUILD_DIR) && $(MAKE) menuconfig
	@echo "✓ Configuration updated"

# Interactive configuration (nconfig)
nconfig: _ensure_build_dir
	@cd $(BUILD_DIR) && $(MAKE) nconfig
	@echo "✓ Configuration updated"

# Save configuration as defconfig
savedefconfig: _ensure_build_dir
	@cd $(BUILD_DIR) && $(MAKE) savedefconfig
	@if [ -n "$(NAME)" ]; then \
		mkdir -p $(CONFIGS_DIR); \
		cp $(BUILD_DIR)/defconfig $(CONFIGS_DIR)/$(NAME)_defconfig; \
		echo "✓ Configuration saved to $(CONFIGS_DIR)/$(NAME)_defconfig"; \
	else \
		echo "✓ Configuration saved to $(BUILD_DIR)/defconfig"; \
		echo ""; \
		echo "To save as a named config:"; \
		echo "  make savedefconfig NAME=<name>"; \
	fi

# Update configuration with new options
oldconfig: _ensure_build_dir
	@cd $(BUILD_DIR) && $(MAKE) oldconfig
	@echo "✓ Configuration updated"

# Build project
build: _ensure_build_dir _ensure_config
	@echo "Building ES-Middleware..."
	@cd $(BUILD_DIR) && cmake .. >/dev/null && $(MAKE) -j$(JOBS)
	@echo ""
	@echo "✓ Build successful"
	@echo ""
	@echo "Binaries: $(BUILD_DIR)/bin/"
	@ls -lh $(BUILD_DIR)/bin/ 2>/dev/null || true

# Set default JOBS
JOBS ?= $(NPROC)

# Build all (configure + build)
all: config build

# Clean build directory (keep configuration)
clean:
	@if [ -d "$(BUILD_DIR)" ]; then \
		echo "Cleaning $(BUILD_DIR)..."; \
		rm -rf $(BUILD_DIR); \
		echo "✓ Clean complete"; \
	else \
		echo "Nothing to clean"; \
	fi

# Complete clean (remove configuration too)
distclean: clean
	@if [ -f "$(CONFIG_FILE)" ]; then \
		echo "Removing $(CONFIG_FILE)..."; \
		rm -f $(CONFIG_FILE); \
		echo "✓ Distclean complete"; \
	fi

# Run tests
test: _ensure_build_dir
	@if [ -f "$(BUILD_DIR)/bin/es-middleware-test" ]; then \
		echo "Running tests..."; \
		$(BUILD_DIR)/bin/es-middleware-test --all; \
	else \
		echo "Error: Test binary not found. Build first with 'make build'"; \
		exit 1; \
	fi

# Internal: Ensure build directory exists
_ensure_build_dir:
	@if [ ! -d "$(BUILD_DIR)" ]; then \
		echo "Creating build directory..."; \
		mkdir -p $(BUILD_DIR); \
		cd $(BUILD_DIR) && cmake .. >/dev/null 2>&1 || true; \
	fi

# Internal: Ensure configuration exists
_ensure_config:
	@if [ ! -f "$(CONFIG_FILE)" ]; then \
		echo "Error: No configuration found"; \
		echo ""; \
		echo "Please configure first:"; \
		echo "  make config CONFIG=<name>"; \
		echo "  make menuconfig"; \
		echo ""; \
		echo "Or see available configurations:"; \
		echo "  make list"; \
		exit 1; \
	fi
