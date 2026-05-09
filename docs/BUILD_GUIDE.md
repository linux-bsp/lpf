# EMS 构建指南

本文档提供 EMS 框架的完整构建说明，包括本地编译和交叉编译。

## 快速开始

### 标准 CMake 命令

```bash
# 本地编译
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Debug 模式
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)

# 交叉编译
cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/arm32-linux-gnueabihf.cmake
cmake --build build -j$(nproc)

cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/aarch64-linux-gnu.cmake
cmake --build build -j$(nproc)

cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/riscv64-linux-gnu.cmake
cmake --build build -j$(nproc)
```

### 使用 build.sh（便捷脚本）

```bash
./build.sh              # Release 模式
./build.sh -d           # Debug 模式
./build.sh -c           # 清理
./build.sh -a arm32     # ARM32 交叉编译
./build.sh -a arm64     # ARM64 交叉编译
./build.sh -a riscv64   # RISC-V 64 交叉编译
```

## 交叉编译

### 支持的架构

| 架构 | 工具链文件 | 工具链 | 状态 |
|------|-----------|--------|------|
| x86_64 | - | gcc | ✅ 完全支持 |
| ARM32 | arm32-linux-gnueabihf.cmake | arm-linux-gnueabihf-gcc | ✅ 完全支持 |
| ARM64 | aarch64-linux-gnu.cmake | aarch64-linux-gnu-gcc | ✅ 完全支持 |
| RISC-V 64 | riscv64-linux-gnu.cmake | riscv64-linux-gnu-gcc | ✅ 完全支持 |

### 安装工具链

**Ubuntu/Debian:**
```bash
sudo apt-get install gcc-arm-linux-gnueabihf      # ARM32
sudo apt-get install gcc-aarch64-linux-gnu        # ARM64
sudo apt-get install gcc-riscv64-linux-gnu        # RISC-V 64
```

**Fedora/RHEL:**
```bash
sudo dnf install gcc-arm-linux-gnu                # ARM32
sudo dnf install gcc-aarch64-linux-gnu            # ARM64
sudo dnf install gcc-riscv64-linux-gnu            # RISC-V 64
```

### 架构特定编译选项

- **ARM32**: `-march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=hard`
- **ARM64**: `-march=armv8-a`
- **RISC-V 64**: `-march=rv64imafdc -mabi=lp64d`

## 构建选项

```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=OFF \
  -DBUILD_SHARED_LIBS=OFF
cmake --build build
```

**可用选项:**
- `BUILD_TESTING`: 是否构建测试（默认 ON）
- `BUILD_SHARED_LIBS`: 是否构建动态库（默认 ON）
- `CMAKE_BUILD_TYPE`: 构建类型（Debug/Release）

## 输出目录

```
build/
├── bin/              # 可执行文件
│   ├── sample_app
│   └── ems-test
└── lib/              # 库文件
    ├── libosal.so
    ├── libhal.so
    ├── libpcl.so
    └── libpdl.so
```

## 运行程序

```bash
# 运行示例应用
./build/bin/sample_app

# 运行测试
./build/bin/ems-test -i     # 交互式
./build/bin/ems-test -a     # 运行所有测试
./build/bin/osal-test -a    # 仅运行OSAL层测试
```

## 构建系统架构

### 模块依赖层次

```
Apps → PDL → HAL → OSAL
         ↓
        PCL
```

### 接口库设计

每个模块提供两个库：
- **接口库** (`<module>_public_api`) - 仅头文件，用于编译时依赖
- **实现库** (`<module>`) - 静态/动态库，用于链接

**示例:**
```cmake
# 链接 OSAL
target_link_libraries(my_module 
    PUBLIC ems::osal_public_api    # 获取头文件
    PRIVATE ems::osal)             # 链接实现
```

## 常见问题

**Q: 找不到交叉编译工具链？**
安装对应的工具链（见上方安装说明）。

**Q: macOS 上编译？**
macOS 使用打桩实现的 HAL 层，仅用于编译验证，不支持实际硬件操作。

**Q: 如何指定自定义构建目录？**
```bash
cmake -B my-build -DCMAKE_BUILD_TYPE=Release
cmake --build my-build
```

**Q: 如何清理构建？**
```bash
rm -rf build
# 或使用脚本
./build.sh -c
```

## 参考文档

- [架构设计](ARCHITECTURE.md)
- [编码规范](CODING_STANDARDS.md)
- [Buildroot 集成](buildroot/README.md)
