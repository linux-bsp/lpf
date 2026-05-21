# EMS Configuration Files Guide

**Last Updated**: 2026-05-22  
**Platform**: x86_64 (default)

---

## 📋 Available Configurations

EMS provides multiple pre-configured defconfig files optimized for different use cases:

### 1. **defconfig** (Default - Recommended)

**Purpose**: Balanced configuration for general x86_64 development and deployment

**Features**:
- ✅ All core modules enabled (OSAL, HAL, PCL, PDL, ACL)
- ✅ All CCM applications enabled
- ✅ Both static and dynamic libraries
- ✅ Optimization: -O2 (balanced)
- ✅ Resource limits: Standard (128 tasks, 64 queues)
- ✅ All hardware drivers enabled
- ✅ All PDL protocols enabled

**Use Cases**:
- General development
- Production deployment
- Testing and validation

**Usage**:
```bash
make defconfig
make -j$(nproc)
```

---

### 2. **x86_64_full_defconfig** (All Features)

**Purpose**: Maximum features for development and testing

**Features**:
- ✅ All features from default config
- ✅ Debug symbols enabled
- ✅ Verbose logging (level 4)
- ✅ Optimization: -O2
- ✅ Resource limits: Generous (256 tasks, 128 queues)
- ✅ All debug options enabled
- ✅ Unit tests and integration tests
- ✅ Debug logging for all modules

**Use Cases**:
- Development and debugging
- Feature testing
- Performance profiling
- Integration testing

**Usage**:
```bash
make x86_64_full_defconfig
make -j$(nproc)
```

---

### 3. **x86_64_release_defconfig** (Production)

**Purpose**: Optimized for production deployment

**Features**:
- ✅ All core modules enabled
- ✅ All CCM applications enabled
- ✅ Both static and dynamic libraries
- ✅ Optimization: -O3 (maximum speed)
- ✅ Resource limits: Standard
- ✅ Minimal logging (level 2)
- ❌ Debug symbols disabled
- ❌ Verbose logging disabled

**Use Cases**:
- Production deployment
- Performance-critical applications
- Release builds

**Usage**:
```bash
make x86_64_release_defconfig
make -j$(nproc)
```

---

### 4. **x86_64_minimal_defconfig** (Embedded)

**Purpose**: Minimal footprint for resource-constrained systems

**Features**:
- ✅ Core modules enabled
- ✅ Essential CCM applications only (supervisor, logger)
- ✅ Static libraries only
- ✅ Optimization: -Os (size)
- ✅ Resource limits: Minimal (32 tasks, 16 queues)
- ✅ Essential drivers only (UART, Watchdog)
- ❌ Network support disabled
- ❌ Advanced features disabled

**Use Cases**:
- Embedded x86_64 systems
- Resource-constrained environments
- Minimal deployments

**Usage**:
```bash
make x86_64_minimal_defconfig
make -j$(nproc)
```

---

### 5. **ccm_h200_am625_debug_defconfig** (Legacy)

**Purpose**: Debug configuration for H200 AM625 platform

**Features**:
- ✅ Debug build (-O0)
- ✅ All features enabled
- ✅ Debug logging enabled
- ✅ Architecture: x86_64 (can be changed to ARM64)

**Use Cases**:
- H200 AM625 platform development
- Debugging specific issues

**Usage**:
```bash
make ccm_h200_am625_debug_defconfig
make -j$(nproc)
```

---

### 6. **ccm_h200_am625_release_defconfig** (Legacy)

**Purpose**: Release configuration for H200 AM625 platform

**Features**:
- ✅ Release build (-O2)
- ✅ All features enabled
- ✅ Minimal logging

**Use Cases**:
- H200 AM625 platform deployment

**Usage**:
```bash
make ccm_h200_am625_release_defconfig
make -j$(nproc)
```

---

## 🎯 Quick Selection Guide

| Use Case | Recommended Config | Build Time | Binary Size | Features |
|----------|-------------------|------------|-------------|----------|
| **Development** | x86_64_full_defconfig | ~30s | Large | All + Debug |
| **Production** | x86_64_release_defconfig | ~25s | Medium | All |
| **General Use** | defconfig | ~25s | Medium | All |
| **Embedded** | x86_64_minimal_defconfig | ~15s | Small | Essential |
| **H200 Debug** | ccm_h200_am625_debug_defconfig | ~30s | Large | All + Debug |
| **H200 Release** | ccm_h200_am625_release_defconfig | ~25s | Medium | All |

---

## 📊 Feature Comparison

| Feature | defconfig | x86_64_full | x86_64_release | x86_64_minimal |
|---------|-----------|-------------|----------------|----------------|
| **Optimization** | -O2 | -O2 | -O3 | -Os |
| **Debug Symbols** | ❌ | ✅ | ❌ | ❌ |
| **Static Libraries** | ✅ | ✅ | ✅ | ✅ |
| **Dynamic Libraries** | ✅ | ✅ | ✅ | ❌ |
| **OSAL** | ✅ | ✅ | ✅ | ✅ |
| **HAL (All Drivers)** | ✅ | ✅ | ✅ | ⚠️ Essential |
| **PCL** | ✅ | ✅ | ✅ | ✅ |
| **PDL (All Modules)** | ✅ | ✅ | ✅ | ⚠️ Essential |
| **ACL** | ✅ | ✅ | ✅ | ✅ |
| **CCM Apps (All)** | ✅ | ✅ | ✅ | ⚠️ Essential |
| **Network Support** | ✅ | ✅ | ✅ | ❌ |
| **Debug Logging** | ❌ | ✅ | ❌ | ❌ |
| **Unit Tests** | ❌ | ✅ | ❌ | ❌ |
| **Max Tasks** | 128 | 256 | 128 | 32 |
| **Log Level** | 2 | 4 | 2 | 1 |

---

## 🔧 Customization

### Method 1: Use menuconfig (Recommended)

```bash
# Start with a base configuration
make defconfig

# Customize interactively
make menuconfig

# Save your custom configuration
make savedefconfig
mv defconfig configs/my_custom_defconfig
```

### Method 2: Edit defconfig directly

```bash
# Copy an existing config
cp configs/defconfig configs/my_custom_defconfig

# Edit the file
vim configs/my_custom_defconfig

# Use your custom config
make my_custom_defconfig
make -j$(nproc)
```

### Method 3: Override on command line

```bash
# Load base config
make defconfig

# Override specific options
make CFLAGS_OPTIMIZE="-O3" -j$(nproc)
```

---

## 📝 Configuration Options Reference

### Build Options

```makefile
CONFIG_OPT_O0=y              # No optimization (-O0)
CONFIG_OPT_O2=y              # Standard optimization (-O2)
CONFIG_OPT_O3=y              # Aggressive optimization (-O3)
CONFIG_OPT_OS=y              # Size optimization (-Os)

CONFIG_BUILD_TYPE_DEBUG=y    # Debug build
CONFIG_BUILD_TESTING=y       # Enable testing

CONFIG_LIBRARY_BUILD_TYPE_STATIC=y    # Build static libraries
CONFIG_LIBRARY_BUILD_TYPE_DYNAMIC=y   # Build dynamic libraries
```

### Architecture

```makefile
CONFIG_ARCH_X86_64=y         # x86_64 (default)
CONFIG_ARCH_ARM32=y          # ARM 32-bit
CONFIG_ARCH_ARM64=y          # ARM 64-bit
CONFIG_ARCH_RISCV64=y        # RISC-V 64-bit

CONFIG_CROSS_COMPILE=""      # Cross-compiler prefix
```

### OSAL Options

```makefile
CONFIG_OSAL=y                # Enable OSAL
CONFIG_OSAL_BUILD_STATIC=y   # Build static library
CONFIG_OSAL_BUILD_SHARED=y   # Build dynamic library

CONFIG_OSAL_FILE=y           # File I/O support
CONFIG_OSAL_NETWORK=y        # Network support
CONFIG_OSAL_IPC=y            # IPC support
CONFIG_OSAL_THREAD=y         # Thread support

CONFIG_OSAL_MAX_TASKS=128    # Maximum tasks
CONFIG_OSAL_MAX_QUEUES=64    # Maximum queues
CONFIG_OSAL_MAX_MUTEXES=64   # Maximum mutexes
```

### HAL Options

```makefile
CONFIG_HAL=y                 # Enable HAL
CONFIG_HAL_BUILD_STATIC=y    # Build static library
CONFIG_HAL_BUILD_SHARED=y    # Build dynamic library

CONFIG_HAL_CAN=y             # CAN driver
CONFIG_HAL_UART=y            # UART driver
CONFIG_HAL_I2C=y             # I2C driver
CONFIG_HAL_SPI=y             # SPI driver
CONFIG_HAL_GPIO=y            # GPIO driver
CONFIG_HAL_WATCHDOG=y        # Watchdog driver
```

### CCM Applications

```makefile
CONFIG_BUILD_CCM_SUPERVISOR=y    # Supervisor application
CONFIG_BUILD_CCM_COLLECTOR=y     # Collector application
CONFIG_BUILD_CCM_COMM=y          # Communication application
CONFIG_BUILD_CCM_HEALTH=y        # Health monitor application
CONFIG_BUILD_CCM_LOGGER=y        # Logger application
```

---

## 🚀 Usage Examples

### Example 1: Quick Start (Default Config)

```bash
# Use default configuration
make defconfig
make -j$(nproc)

# Output in staging/
ls -lh staging/bin/
ls -lh staging/lib/
```

### Example 2: Development Build

```bash
# Use full configuration with all debug features
make x86_64_full_defconfig
make -j$(nproc)

# Run with debug logging
./staging/bin/ccm_collector --verbose
```

### Example 3: Production Build

```bash
# Use release configuration
make x86_64_release_defconfig
make -j$(nproc)

# Strip binaries for smaller size
strip staging/bin/*
strip staging/lib/*.so
```

### Example 4: Minimal Build

```bash
# Use minimal configuration
make x86_64_minimal_defconfig
make -j$(nproc)

# Check binary sizes
du -sh staging/bin/*
du -sh staging/lib/*
```

### Example 5: Custom Build

```bash
# Start with default
make defconfig

# Customize
make menuconfig
# Navigate to options and modify as needed

# Save custom config
make savedefconfig
mv defconfig configs/my_project_defconfig

# Use custom config later
make my_project_defconfig
make -j$(nproc)
```

---

## 📚 Related Documentation

- [BUILD_GUIDE.md](BUILD_GUIDE.md) - Complete build guide
- [BUILD_SYSTEM.md](BUILD_SYSTEM.md) - Build system details
- [STAGING_DIRECTORY.md](STAGING_DIRECTORY.md) - Staging directory usage
- [CLAUDE.md](../CLAUDE.md) - AI assistant guide

---

**Maintainer**: wanguo  
**Branch**: feature/kconfig-integration  
**Platform**: x86_64 (default)
