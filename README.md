# EMS SDK

嵌入式管理系统 SDK，采用 Kconfig + CMake 混合构建系统。

## 快速开始

### 从根目录构建

```bash
# 列出所有产品和配置
python3 build.py --list

# 构建指定产品和配置
python3 build.py --product ccm_product --config h200_100p_v1

# 清理并重新构建
python3 build.py --product ccm_product --config h200_200p --clean

# 并行编译（使用 8 个线程）
python3 build.py --product ccm_product --config h200_100p_v2 -j 8
```

构建结果在 `build/<product>/` 目录（符号链接到产品的 build 目录）。

### 从产品目录构建

```bash
cd products/ccm_product

# 选择配置
cp configs/h200_100p_v1_defconfig .config.mk

# 编译
python3 project.py build

# 或使用 menuconfig 自定义配置
python3 project.py menuconfig
python3 project.py build
```

## 项目结构

```
EMS/
├── build.py                  # 根目录构建脚本
├── CMakeLists.txt            # 顶层 CMake 配置
│
├── components/               # SDK 组件（可复用）
│   ├── osal/                # 操作系统抽象层
│   ├── hal/                 # 硬件抽象层
│   ├── pcl/                 # 协议控制层
│   ├── pdl/                 # 协议数据层
│   └── acl/                 # 访问控制层
│
├── products/                 # 产品目录
│   └── ccm_product/         # CCM 产品线
│       ├── configs/         # 产品配置
│       ├── components/      # 产品组件
│       └── apps/            # 应用程序
│
└── tools/                    # 构建工具
    └── cmake/               # CMake 模块
```

## 产品配置

### CCM 产品线

| 型号 | 配置文件 | 应用组合 |
|------|---------|---------|
| H200-100P V1 | h200_100p_v1 | collector + comm |
| H200-100P V2 | h200_100p_v2 | collector + comm + health |
| H200-200P | h200_200p | 全部应用 |

## 构建选项

```bash
python3 build.py --help

Options:
  --list              列出所有产品和配置
  --product, -p       指定产品名称
  --config, -c        指定配置名称
  --build-dir, -b     指定构建目录（默认: build）
  --clean             清理后重新构建
  --jobs, -j          并行编译线程数
```

## 添加新产品

1. 在 `products/` 下创建产品目录
2. 添加 `CMakeLists.txt` 和 `project.py`
3. 创建 `configs/` 目录存放配置文件
4. 创建 `components/` 和 `apps/` 目录

详见 `products/ccm_product/` 示例。

## 开发指南

- [架构设计](docs/ARCHITECTURE.md)
- [CMake 构建指南](docs/CMAKE_BUILD_GUIDE.md)
- [编码规范](docs/CODING_STANDARDS.md)

## 许可证

Apache 2.0
