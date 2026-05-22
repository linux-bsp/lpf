# 构建系统

本目录包含 EMS 构建系统的完整文档。

## 文档列表

- [BUILD_GUIDE.md](BUILD_GUIDE.md) - 构建指南（快速入门）
- [BUILD_SYSTEM.md](BUILD_SYSTEM.md) - 构建系统详解（深入理解）
- [DEFCONFIG_GUIDE.md](DEFCONFIG_GUIDE.md) - Defconfig 配置指南
- [STAGING.md](STAGING.md) - Staging 目录快速配置
- [STAGING_DIRECTORY.md](STAGING_DIRECTORY.md) - Staging 目录详细说明
- [MAKEFILE_FRAMEWORK.md](MAKEFILE_FRAMEWORK.md) - Makefile 框架说明

## 构建系统概述

EMS 采用 Linux 内核风格的 Kbuild 构建系统，主要特性：

- **Kconfig 配置管理** - `menuconfig` 图形化配置
- **条件编译** - `obj-$(CONFIG_XXX)` 语法
- **声明式 Makefile** - `app-y`、`lib-y`、`so-y` 变量
- **Staging 目录** - 统一的构建产物管理
- **外部构建支持** - `O=dir` 指定输出目录
- **并行编译** - `make -j` 支持

## 快速开始

```bash
# 1. 配置
make menuconfig
# 或使用预定义配置
make x86_64_full_defconfig

# 2. 编译
make -j$(nproc)

# 3. 查看输出
ls .staging/bin/
ls .staging/lib/
ls .staging/include/

# 4. 清理
make clean
```

## 推荐阅读顺序

1. **初学者**：BUILD_GUIDE.md → STAGING.md → DEFCONFIG_GUIDE.md
2. **深入理解**：BUILD_SYSTEM.md → STAGING_DIRECTORY.md
3. **框架开发**：MAKEFILE_FRAMEWORK.md

## 相关文档

- [系统集成](../05-integration/) - Buildroot/Yocto 集成
- [架构设计](../03-architecture/) - 了解模块结构
