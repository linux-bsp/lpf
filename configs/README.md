# EMS 配置文件说明

本目录包含 EMS 项目的预定义配置文件（defconfig）。

## 配置文件列表

### x86_64 平台

| 配置文件 | 用途 | 优化 | 库类型 | 测试 |
|---------|------|------|--------|------|
| `x86_64_full_defconfig` | 开发调试 | -O2 | 静态+动态 | ❌ |
| `x86_64_minimal_defconfig` | 生产部署 | -Os | 仅静态 | ❌ |
| `x86_64_test_defconfig` | 自动化测试 | -O2 | 静态+动态 | ✅ |

### ARM64 平台

| 配置文件 | 用途 | 优化 | 库类型 | 测试 |
|---------|------|------|--------|------|
| `arm64_full_defconfig` | 开发调试 | -O0 | 动态 | ❌ |
| `arm64_minimal_defconfig` | 生产部署 | -Os | 仅静态 | ❌ |
| `arm64_test_defconfig` | 自动化测试 | -O0 | 静态+动态 | ✅ |

**注**：ARM64 配置针对 TI AM625 SoC (CCM H200 平台)

### 默认配置

| 配置文件 | 说明 |
|---------|------|
| `defconfig` | 通用默认配置，基于 x86_64_full |

## 使用方法

### 加载配置

```bash
# x86_64 平台
make x86_64_full_defconfig      # 开发
make x86_64_minimal_defconfig   # 生产
make x86_64_test_defconfig      # 测试

# ARM64 平台
make arm64_full_defconfig       # 开发
make arm64_minimal_defconfig    # 生产
make arm64_test_defconfig       # 测试

# 默认配置
make defconfig
```

### 自定义配置

```bash
# 1. 加载基础配置
make x86_64_full_defconfig

# 2. 交互式修改
make menuconfig

# 3. 保存为新配置
make savedefconfig
cp defconfig configs/my_custom_defconfig
```

## 配置说明

### Full 配置（开发）

**用途**：日常开发和调试

**特点**：
- 所有功能模块启用
- 调试符号和日志
- 动态库支持快速迭代
- x86_64: -O2 优化（平衡性能）
- ARM64: -O0 优化（最佳调试体验）

**适用场景**：
- 本地开发
- 功能调试
- 性能分析

### Minimal 配置（生产）

**用途**：生产环境部署

**特点**：
- 仅核心功能
- -Os 优化（最小体积）
- 仅静态库（独立部署）
- 禁用调试日志
- 禁用测试框架

**适用场景**：
- 嵌入式设备
- 资源受限环境
- 生产部署

### Test 配置（测试）

**用途**：自动化测试和 CI/CD

**特点**：
- 所有功能模块启用
- 所有测试套件启用
- 静态+动态库（全面测试）
- x86_64: -O2 优化（真实性能）
- ARM64: -O0 优化（准确覆盖率）

**适用场景**：
- 单元测试
- 集成测试
- CI/CD 流水线

## 配置对比

### 功能对比

| 功能 | Full | Minimal | Test |
|------|------|---------|------|
| OSAL | ✅ 全部 | ✅ 核心 | ✅ 全部 |
| HAL | ✅ 全部 | ✅ 核心 | ✅ 全部 |
| PCL | ✅ | ❌ | ✅ |
| PDL | ✅ | ❌ | ✅ |
| ACL | ✅ | ❌ | ✅ |
| 测试框架 | ❌ | ❌ | ✅ |
| 调试日志 | ✅ | ❌ | ✅ |

### 构建产物大小（估算）

| 平台 | Full | Minimal | Test |
|------|------|---------|------|
| x86_64 | ~2MB | ~500KB | ~3MB |
| ARM64 | ~1.5MB | ~400KB | ~2.5MB |

## 平台特定说明

### x86_64 平台

- **目标**：x86_64 Linux (Ubuntu/Debian/CentOS)
- **编译器**：系统默认 GCC
- **交叉编译**：不需要

### ARM64 平台

- **目标**：TI AM625 SoC (ARM Cortex-A53)
- **编译器**：aarch64-linux-gnu-gcc
- **交叉编译**：需要安装工具链

```bash
# Ubuntu/Debian
sudo apt-get install gcc-aarch64-linux-gnu

# 验证
aarch64-linux-gnu-gcc --version
```

## 常见问题

### Q: 如何选择配置？

**A**: 根据使用场景：
- 开发调试 → `*_full_defconfig`
- 生产部署 → `*_minimal_defconfig`
- 自动化测试 → `*_test_defconfig`

### Q: 配置之间可以切换吗？

**A**: 可以，但需要清理：

```bash
make clean
make <new_config>_defconfig
make -j$(nproc)
```

### Q: 如何添加新配置？

**A**: 
1. 基于现有配置修改
2. 使用 `make savedefconfig` 保存
3. 复制到 `configs/` 目录
4. 提交到 Git

### Q: HAL_PLATFORM_NAME 配置错误？

**A**: 确保配置文件中的 `CONFIG_HAL_PLATFORM_NAME` 与实际目录匹配：
- x86_64: `"generic-linux"`
- ARM64: `"generic-linux"` 或 `"ti-am625"`（如果目录存在）

## 维护指南

### 添加新配置

1. 创建配置文件：`configs/new_config_defconfig`
2. 添加头部注释（参考现有文件）
3. 更新本 README
4. 测试构建：`make new_config_defconfig && make`

### 更新现有配置

1. 加载配置：`make xxx_defconfig`
2. 修改：`make menuconfig`
3. 保存：`make savedefconfig`
4. 覆盖：`cp defconfig configs/xxx_defconfig`
5. 提交 Git

### 配置文件格式

```kconfig
#
# 配置名称
# Target: 目标平台
# Purpose: 用途说明
# Optimization: 优化级别
# Libraries: 库类型
#

# ============================================================================
# Build Options
# ============================================================================
CONFIG_XXX=y
...
```

## 相关文档

- [构建指南](../docs/BUILD_GUIDE.md)
- [平台配置](../docs/PLATFORM.md)
- [Kconfig 配置](../Kconfig)

---

**最后更新**: 2026-05-26  
**维护者**: EMS Team
