# EMS SDK

EMS (Embedded Management System) 是一个采用 **Kconfig + CMake** 混合构建系统的嵌入式软件项目。

## 快速开始

### 1. 列出所有产品和配置

```bash
python3 project.py --list
```

### 2. 配置项目

使用图形化配置界面：

```bash
python3 project.py menuconfig
```

或者加载预定义配置：

```bash
python3 project.py build --config ccm_h200_100p_v1
python3 project.py build --config sample_default
```

### 3. 构建项目

```bash
python3 project.py build
```

### 4. 清理构建

```bash
python3 project.py build --clean
```

## 项目架构

```
EMS/
├── Kconfig                    # 根配置文件
├── CMakeLists.txt            # 根构建文件
├── project.py                # 统一构建脚本
├── configs/                  # 预定义配置
│   ├── ccm_h200_100p_v1_defconfig
│   ├── ccm_h200_100p_v2_defconfig
│   ├── ccm_h200_200p_defconfig
│   └── sample_default_defconfig
├── components/               # 核心组件
│   ├── osal/                # 操作系统抽象层
│   ├── hal/                 # 硬件抽象层
│   ├── pcl/                 # 协议控制层
│   ├── pdl/                 # 协议数据层
│   └── acl/                 # 访问控制层
├── products/                 # 产品目录
│   ├── ccm/                 # CCM 产品
│   │   ├── Kconfig
│   │   ├── CMakeLists.txt
│   │   ├── components/      # 产品特定组件
│   │   └── apps/            # 产品应用
│   └── sample/              # Sample 产品
│       ├── Kconfig
│       ├── CMakeLists.txt
│       └── apps/
└── build/                    # 构建输出
    ├── config/              # 配置文件
    └── products/            # 产品构建输出
        ├── ccm/
        └── sample/
```

## 可用产品

- **CCM**: 卫星载荷控制和数据管理系统
- **Sample**: 示例产品（演示和模板）

## 可用配置

- **ccm_h200_100p_v1**: CCM 基础型号（2个应用）
- **ccm_h200_100p_v2**: CCM 增强型号（3个应用）
- **ccm_h200_200p**: CCM 完整型号（5个应用）
- **sample_default**: Sample 默认配置

## 配置系统

项目使用 Kconfig 进行功能配置：

1. **产品选择**: 选择要构建的产品
2. **核心组件**: 配置 OSAL、HAL、PCL、PDL、ACL
3. **产品配置**: 每个产品的特定配置

## 构建系统

- **配置阶段**: Kconfig 生成 `.config` 和 CMake 配置文件
- **构建阶段**: CMake 根据配置选择性编译产品
- **输出**: 所有产品的构建输出在 `build/products/` 下

## 添加新产品

1. 在 `products/` 下创建产品目录
2. 创建 `Kconfig` 和 `CMakeLists.txt`
3. 在根 `Kconfig` 中添加产品选择项
4. 在根 `CMakeLists.txt` 中添加产品构建逻辑
5. 在 `configs/` 下创建 defconfig 文件

详细文档请参考 `CLAUDE.md`。
