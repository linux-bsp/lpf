# 系统集成

本目录包含 EMS 与外部构建系统集成的文档。

## 文档列表

- [buildroot/](buildroot/) - Buildroot 集成指南

## 集成方式

### Buildroot 集成

EMS 可以作为 Buildroot package 集成到嵌入式 Linux 系统中。

**快速开始**：
```bash
# 1. 复制 package 定义
cp -r docs/05-integration/buildroot/packages/ems $BR2_EXTERNAL/package/

# 2. 配置 Buildroot
make menuconfig
# 选择 Target packages -> EMS

# 3. 编译
make ems
```

详细说明参考 [buildroot/README.md](buildroot/README.md)。

### Yocto 集成

EMS 也支持 Yocto 集成，通过设置 `STAGING_DIR` 环境变量：

```bash
# 在 recipe 中
do_compile() {
    oe_runmake STAGING_DIR=${STAGING_DIR}/usr
}
```

### 独立编译

EMS 也可以独立编译，不依赖外部构建系统：

```bash
# 直接编译
make x86_64_full_defconfig
make -j$(nproc)

# 指定 staging 目录
make STAGING_DIR=/opt/ems/staging
```

## Staging 目录配置

EMS 使用 `.staging/` 目录管理构建产物，支持三种配置方式：

1. **默认方式**：`.staging/` 在构建目录下
2. **环境变量**：`export STAGING_DIR=/path/to/staging`
3. **命令行参数**：`make STAGING_DIR=/path/to/staging`

优先级：命令行 > 环境变量 > 默认值

详细说明参考 [../02-build-system/STAGING_DIRECTORY.md](../02-build-system/STAGING_DIRECTORY.md)。

## 相关文档

- [构建系统](../02-build-system/) - 了解构建机制
- [快速入门](../01-getting-started/) - 基本编译流程
