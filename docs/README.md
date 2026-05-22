# EMS 项目文档

欢迎使用 EMS (Embedded Management System) 项目文档。

## 文档结构

本文档按功能模块组织，推荐按以下顺序阅读：

### [01-getting-started/](01-getting-started/) - 快速入门
新用户从这里开始，了解如何安装和编译 EMS。

- [INSTALL.md](01-getting-started/INSTALL.md) - 安装依赖和环境配置

### [02-build-system/](02-build-system/) - 构建系统
深入了解 EMS 的 Kbuild 构建系统。

- [BUILD_GUIDE.md](02-build-system/BUILD_GUIDE.md) - 构建指南（推荐首读）
- [BUILD_SYSTEM.md](02-build-system/BUILD_SYSTEM.md) - 构建系统详解
- [DEFCONFIG_GUIDE.md](02-build-system/DEFCONFIG_GUIDE.md) - Defconfig 配置
- [STAGING.md](02-build-system/STAGING.md) - Staging 目录快速配置
- [STAGING_DIRECTORY.md](02-build-system/STAGING_DIRECTORY.md) - Staging 详细说明
- [MAKEFILE_FRAMEWORK.md](02-build-system/MAKEFILE_FRAMEWORK.md) - Makefile 框架

### [03-architecture/](03-architecture/) - 架构设计
了解 EMS 的分层架构和各模块设计。

- [ARCHITECTURE.md](03-architecture/ARCHITECTURE.md) - 总体架构
- [README.md](03-architecture/README.md) - 架构文档导航
- 各层详细设计文档（OSAL、HAL、PCL、PDL、ACL、App）

### [04-platform/](04-platform/) - 平台适配
如何将 EMS 移植到新的硬件平台或操作系统。

- [PLATFORM.md](04-platform/PLATFORM.md) - 平台支持总览
- [OSAL_PLATFORM.md](04-platform/OSAL_PLATFORM.md) - OSAL 平台适配
- [HAL_PLATFORM.md](04-platform/HAL_PLATFORM.md) - HAL 平台适配

### [05-integration/](05-integration/) - 系统集成
将 EMS 集成到 Buildroot、Yocto 等构建系统。

- [buildroot/](05-integration/buildroot/) - Buildroot 集成指南

### [06-development/](06-development/) - 开发规范
代码风格、开发流程和最佳实践。

- [CODING_STANDARDS.md](06-development/CODING_STANDARDS.md) - 编码规范

## 快速导航

### 我想...

- **快速编译项目** → [BUILD_GUIDE.md](02-build-system/BUILD_GUIDE.md)
- **了解系统架构** → [ARCHITECTURE.md](03-architecture/ARCHITECTURE.md)
- **移植到新平台** → [PLATFORM.md](04-platform/PLATFORM.md)
- **集成到 Buildroot** → [buildroot/README.md](05-integration/buildroot/README.md)
- **贡献代码** → [CODING_STANDARDS.md](06-development/CODING_STANDARDS.md)
- **配置构建选项** → [DEFCONFIG_GUIDE.md](02-build-system/DEFCONFIG_GUIDE.md)
- **理解 staging 目录** → [STAGING_DIRECTORY.md](02-build-system/STAGING_DIRECTORY.md)

## 推荐学习路径

### 初学者路径
1. [INSTALL.md](01-getting-started/INSTALL.md) - 安装环境
2. [BUILD_GUIDE.md](02-build-system/BUILD_GUIDE.md) - 编译项目
3. [ARCHITECTURE.md](03-architecture/ARCHITECTURE.md) - 了解架构

### 开发者路径
1. [BUILD_SYSTEM.md](02-build-system/BUILD_SYSTEM.md) - 深入构建系统
2. [03-architecture/](03-architecture/) - 详细架构设计
3. [CODING_STANDARDS.md](06-development/CODING_STANDARDS.md) - 编码规范

### 移植工程师路径
1. [PLATFORM.md](04-platform/PLATFORM.md) - 平台支持概览
2. [OSAL_PLATFORM.md](04-platform/OSAL_PLATFORM.md) - OS 层适配
3. [HAL_PLATFORM.md](04-platform/HAL_PLATFORM.md) - 硬件层适配

### 集成工程师路径
1. [STAGING_DIRECTORY.md](02-build-system/STAGING_DIRECTORY.md) - 理解 staging
2. [buildroot/README.md](05-integration/buildroot/README.md) - Buildroot 集成
3. [BUILD_SYSTEM.md](02-build-system/BUILD_SYSTEM.md) - 构建系统细节

## 文档维护

- 文档版本与代码版本同步
- 每次架构变更需更新相关文档
- 欢迎提交文档改进建议

## 获取帮助

- 查看相关文档目录的 README.md
- 搜索项目 issue
- 联系项目维护者
