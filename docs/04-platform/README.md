# 平台适配

本目录包含 EMS 平台适配相关文档。

## 文档列表

- [PLATFORM.md](PLATFORM.md) - 平台支持总览
- [OSAL_PLATFORM.md](OSAL_PLATFORM.md) - OSAL 平台适配指南
- [HAL_PLATFORM.md](HAL_PLATFORM.md) - HAL 平台适配指南

## 平台适配概述

EMS 支持多平台运行，通过分层抽象实现平台无关性：

```
应用层 (App)
    ↓
抽象层 (ACL/PDL/PCL)
    ↓
硬件抽象层 (HAL) ← 平台相关
    ↓
操作系统抽象层 (OSAL) ← 平台相关
    ↓
硬件/操作系统
```

## 支持的平台

### 操作系统 (OSAL)
- Linux (POSIX)
- FreeRTOS
- Zephyr
- Bare-metal

### 硬件平台 (HAL)
- x86_64
- ARM Cortex-A (AM625, etc.)
- ARM Cortex-M
- RISC-V

## 适配新平台

### 1. OSAL 适配
参考 [OSAL_PLATFORM.md](OSAL_PLATFORM.md)，实现：
- 线程管理
- 内存管理
- 同步原语
- 时间管理

### 2. HAL 适配
参考 [HAL_PLATFORM.md](HAL_PLATFORM.md)，实现：
- 外设驱动接口
- 中断管理
- DMA 管理
- 时钟配置

### 3. 配置系统
在 Kconfig 中添加平台选项：
```kconfig
config PLATFORM_NEW_BOARD
    bool "New Board Support"
    depends on ARCH_ARM
    help
      Support for new board platform
```

## 相关文档

- [架构设计](../03-architecture/) - 了解分层架构
- [构建系统](../02-build-system/) - 配置和编译
