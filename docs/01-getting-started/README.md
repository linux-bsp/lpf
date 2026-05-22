# 快速入门

本目录包含 EMS 项目的快速入门文档。

## 文档列表

- [INSTALL.md](INSTALL.md) - 安装指南

## 快速开始

1. **安装依赖**：参考 [INSTALL.md](INSTALL.md)
2. **配置项目**：`make menuconfig` 或使用 defconfig
3. **编译项目**：`make -j$(nproc)`
4. **查看输出**：`.staging/bin/` 和 `.staging/lib/`

## 下一步

- 了解构建系统：[../02-build-system/](../02-build-system/)
- 了解架构设计：[../03-architecture/](../03-architecture/)
- 平台适配指南：[../04-platform/](../04-platform/)
