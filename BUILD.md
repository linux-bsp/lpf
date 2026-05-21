# EMS Build System

## Quick Start

```bash
# Configure with default settings
make defconfig

# Build everything
make -j$(nproc)

# Clean build artifacts
make clean
```

## Configuration

```bash
# Use default configuration
make defconfig

# Use a specific configuration
make ccm_h200_am625_debug_defconfig
make ccm_h200_am625_release_defconfig

# Interactive configuration (requires ncurses)
make menuconfig

# Save current configuration as defconfig
make savedefconfig
```

## Build Targets

```bash
# Build all libraries and applications
make all

# Build only core libraries
make core/

# Build only products
make products/

# Clean everything
make clean

# Remove all generated files including configuration
make mrproper
```

## Build Options

```bash
# Verbose build (show full commands)
make V=1

# Out-of-tree build
make O=/path/to/build/dir defconfig
make O=/path/to/build/dir -j$(nproc)

# Parallel build (recommended)
make -j$(nproc)
```

## Output

- **Libraries**: `lib/` directory
  - Static libraries: `lib*.a`
  - Shared libraries: `lib*.so`
- **Applications**: `bin/` directory
- **Kernel modules**: `ko/` directory

## Configuration Files

- `.config` - Current build configuration
- `configs/defconfig` - Default configuration
- `configs/*_defconfig` - Platform-specific configurations
- `include/config/auto.conf` - Auto-generated Makefile configuration
- `include/generated/autoconf.h` - Auto-generated C header

## Kconfig System

The build system uses Linux kernel's Kconfig for configuration management:

- `Kconfig` - Root configuration file
- `core/*/Kconfig` - Core library configurations
- `products/*/Kconfig` - Product configurations

## Standards

- **C Standard**: C90 (ANSI C)
- **Compiler**: GCC or Clang
- **Build System**: Linux kernel-style kbuild

## Troubleshooting

### Configuration Issues

If you encounter configuration errors:

```bash
# Clean all configuration files
make mrproper

# Regenerate configuration
make defconfig
```

### Build Issues

If the build fails or produces no output:

```bash
# Clean and rebuild
make clean
make defconfig
make -j$(nproc)

# For verbose output to debug issues
make V=1
```

### Incremental Builds

The build system automatically tracks dependencies. After modifying source files, simply run:

```bash
make -j$(nproc)
```

Only modified files and their dependents will be recompiled.
