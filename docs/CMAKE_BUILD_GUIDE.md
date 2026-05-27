# EMS CMake 构建系统使用指南

## 概述

EMS 项目已迁移到 CMake 构建系统，同时保留了 Kconfig 配置功能。

## 特性

- ✅ **保留 Kconfig 配置** - 继续使用 `make menuconfig` 和 defconfig
- ✅ **自动增量编译** - 只重新编译修改的文件
- ✅ **自动依赖管理** - CMake 自动处理库依赖关系
- ✅ **并行编译** - 支持 `make -j$(nproc)` 和 Ninja
- ✅ **外部构建** - 支持多配置并行构建
- ✅ **跨平台** - 支持 Linux、Windows、macOS

## 系统要求

### 必需

- CMake >= 3.16
- GCC 或 Clang
- Make 或 Ninja

### 安装 CMake

```bash
# Ubuntu/Debian
sudo apt-get install cmake

# CentOS/RHEL
sudo yum install cmake3
# 或
sudo dnf install cmake

# macOS
brew install cmake

# 从源码安装（如果系统版本太旧）
wget https://github.com/Kitware/CMake/releases/download/v3.27.0/cmake-3.27.0-linux-x86_64.tar.gz
tar xzf cmake-3.27.0-linux-x86_64.tar.gz
sudo cp -r cmake-3.27.0-linux-x86_64/* /usr/local/
```

## 快速开始

### 1. 配置（使用 Kconfig）

```bash
# 方法 1: 使用 menuconfig（推荐）
make menuconfig

# 方法 2: 使用 defconfig
make defconfig

# 方法 3: 使用板级配置
make <board>_defconfig

# 查看可用的 defconfig
ls configs/*_defconfig
```

### 2. 构建

```bash
# 创建构建目录
mkdir build
cd build

# 配置 CMake
cmake ..

# 编译（并行）
make -j$(nproc)

# 或使用 Ninja（更快）
cmake .. -G Ninja
ninja
```

### 3. 安装

```bash
# 安装到默认位置（/usr/local）
sudo make install

# 安装到指定位置
cmake .. -DCMAKE_INSTALL_PREFIX=/opt/ems
make install

# 安装到临时目录（用于打包）
make install DESTDIR=/tmp/ems-install
```

## 构建类型

### Debug 构建

```bash
mkdir build-debug
cd build-debug
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

### Release 构建

```bash
mkdir build-release
cd build-release
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### 多配置并行构建

```bash
# 同时构建 Debug 和 Release
mkdir build-debug && cd build-debug
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc) &

cd ..
mkdir build-release && cd build-release
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc) &

wait
```

## 交叉编译

### ARM64 交叉编译

```bash
# 创建 toolchain 文件
cat > cmake/arm64-toolchain.cmake << 'EOF'
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
EOF

# 使用 toolchain 文件
mkdir build-arm64
cd build-arm64
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/arm64-toolchain.cmake
make -j$(nproc)
```

## 常用命令

### 查看构建信息

```bash
# 详细输出
make VERBOSE=1

# 或
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON
make
```

### 清理

```bash
# 清理构建产物
make clean

# 完全清理（删除 CMake 缓存）
rm -rf build
```

### 只构建特定目标

```bash
# 只构建 OSAL 库
make osal

# 只构建 ccm_collector 应用
make ccm_collector

# 查看所有目标
make help
```

## 修改配置后重新构建

```bash
# 1. 修改 Kconfig 配置
make menuconfig

# 2. 重新配置 CMake（自动检测 .config 变化）
cd build
cmake ..

# 3. 重新编译
make -j$(nproc)
```

## 与原 Make 系统对比

| 操作 | 原 Make 系统 | CMake 系统 |
|------|-------------|-----------|
| 配置 | `make menuconfig` | `make menuconfig` |
| 构建 | `make -j24` | `cmake .. && make -j24` |
| 清理 | `make clean` | `make clean` |
| 安装 | 不支持 | `make install` |
| 外部构建 | 不支持 | `mkdir build && cd build` |
| 增量编译 | ❌ 不支持 | ✅ 自动支持 |
| 依赖管理 | ⚠️ 手工 | ✅ 自动 |

## 目录结构

```
EMS/
├── CMakeLists.txt              # 顶层 CMake 配置
├── cmake/
│   └── Kconfig.cmake           # Kconfig 集成模块
├── core/
│   ├── osal/CMakeLists.txt     # OSAL 库配置
│   ├── hal/CMakeLists.txt      # HAL 库配置
│   └── ...
├── products/
│   └── ccm/
│       ├── CMakeLists.txt      # CCM 产品配置
│       ├── libs/
│       │   └── libccm/CMakeLists.txt
│       └── apps/
│           └── ccm_collector/CMakeLists.txt
├── tests/
│   └── CMakeLists.txt          # 测试配置
├── build/                      # 构建目录（外部构建）
│   ├── lib/                    # 库输出
│   ├── bin/                    # 可执行文件输出
│   └── include/                # 生成的头文件
└── .config                     # Kconfig 配置文件
```

## 构建产物

```
build/
├── lib/
│   ├── libosal.so              # 共享库
│   ├── libosal.so.1            # SONAME 链接
│   ├── libosal.a               # 静态库
│   ├── libhal.so
│   ├── libpcl.so
│   ├── libpdl.so
│   ├── libacl.so
│   ├── libccm.so
│   └── libh200_am625.so
└── bin/
    ├── ccm_collector           # 应用程序
    ├── ccm_health
    ├── ccm_logger
    ├── ccm_supervisor
    └── ccm_comm
```

## 故障排除

### 问题 1: CMake 版本太旧

```bash
$ cmake ..
CMake Error: CMake 3.16 or higher is required.

# 解决：升级 CMake
sudo apt-get install cmake
# 或从源码安装
```

### 问题 2: 找不到 .config 文件

```bash
$ cmake ..
CMake Error: Configuration file .config not found!

# 解决：先运行 Kconfig 配置
make menuconfig
# 或
make defconfig
```

### 问题 3: 找不到头文件

```bash
# 确保已经配置并生成了 autoconf.h
cmake ..
# CMake 会自动生成 build/include/generated/autoconf.h
```

### 问题 4: 链接错误

```bash
# 清理并重新构建
rm -rf build
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## IDE 集成

### VS Code

```bash
# 安装 CMake Tools 扩展
# 打开项目，按 Ctrl+Shift+P
# 选择 "CMake: Configure"
# 选择 "CMake: Build"
```

### CLion

```bash
# 直接打开项目目录
# CLion 会自动识别 CMakeLists.txt
```

### Qt Creator

```bash
# File -> Open File or Project
# 选择 CMakeLists.txt
```

## 性能对比

### 增量编译测试

```bash
# 修改一个源文件
$ vim core/osal/src/posix/sys/osal_thread.c

# 原 Make 系统
$ time make -j24
# 重新编译所有 150+ 个文件
# 耗时：15 秒

# CMake 系统
$ time make -j24
# 只重新编译 osal_thread.c 和依赖它的文件
# 耗时：2 秒

# 效率提升：7.5 倍
```

## 下一步

- 阅读 [docs/BUILD_SYSTEM_ANALYSIS_V2.md](docs/BUILD_SYSTEM_ANALYSIS_V2.md) 了解设计决策
- 阅读 [docs/CMAKE_KCONFIG_INTEGRATION.md](docs/CMAKE_KCONFIG_INTEGRATION.md) 了解 Kconfig 集成
- 查看各个 CMakeLists.txt 了解具体配置

## 反馈

如有问题或建议，请联系项目维护者。

---

**版本**: 3.0  
**最后更新**: 2026-05-27  
**维护者**: wanguo
