# 示例模块 - 快速开始指南

本目录包含标准化的模块模板，帮助你快速创建新的应用程序和库。

## 目录结构

```
products/samples/
├── README.md           # 本文件
├── myapp/             # 应用程序示例
│   ├── module.mk      # 应用程序模板（带详细注释）
│   ├── src/           # 源文件目录
│   └── include/       # 头文件目录
└── mylib/             # 库示例
    ├── module.mk      # 库模板（带详细注释）
    ├── src/           # 源文件目录
    └── include/       # 头文件目录
```

## 快速开始

### 创建新应用程序

```bash
# 1. 复制应用程序模板
cp -r products/samples/myapp products/myproduct/mynewapp

# 2. 编辑 module.mk
cd products/myproduct/mynewapp
# 将所有 "myapp" 替换为 "mynewapp"
# 将所有 "MYAPP" 替换为 "MYNEWAPP"
sed -i 's/myapp/mynewapp/g' module.mk
sed -i 's/MYAPP/MYNEWAPP/g' module.mk

# 3. 修改源文件列表和依赖
vim module.mk
# 编辑 mynewapp_SRCS、mynewapp_CFLAGS、mynewapp_LDFLAGS

# 4. 在顶层 Makefile 中添加
echo "include products/myproduct/mynewapp/module.mk" >> Makefile

# 5. 添加 Kconfig 配置（可选）
vim products/myproduct/Kconfig
# 添加：
# config BUILD_MYNEWAPP
#     bool "Build mynewapp application"
#     default y

# 6. 编译
make menuconfig  # 启用 BUILD_MYNEWAPP
make -j$(nproc)
```

### 创建新库

```bash
# 1. 复制库模板
cp -r products/samples/mylib products/myproduct/mynewlib

# 2. 编辑 module.mk
cd products/myproduct/mynewlib
sed -i 's/mylib/mynewlib/g' module.mk
sed -i 's/MYLIB/MYNEWLIB/g' module.mk

# 3. 修改源文件列表和依赖
vim module.mk

# 4. 在顶层 Makefile 中添加
echo "include products/myproduct/mynewlib/module.mk" >> Makefile

# 5. 添加 Kconfig 配置
vim products/myproduct/Kconfig
# 添加：
# config MYNEWLIB
#     bool "Enable mynewlib"
#     default y
# config MYNEWLIB_BUILD_STATIC
#     bool "Build static library"
#     default y
# config MYNEWLIB_BUILD_SHARED
#     bool "Build shared library"
#     default y

# 6. 编译
make menuconfig
make -j$(nproc)
```

## module.mk 模板说明

### 应用程序模板（myapp/module.mk）

包含以下部分：

1. **源文件列表** - 定义所有 .c 文件
2. **头文件路径** - 定义 include 路径
3. **链接库** - 定义依赖的库
4. **配置开关** - 根据 Kconfig 控制构建
5. **标准构建流程** - 自动处理编译和链接

### 库模板（mylib/module.mk）

包含以下部分：

1. **源文件列表** - 定义所有 .c 文件
2. **头文件路径** - 定义 include 路径
3. **链接库** - 动态库需要链接依赖
4. **配置开关** - 控制静态库/动态库
5. **头文件导出** - 安装到 staging 目录
6. **标准构建流程** - 自动处理编译和打包

## 常见场景

### 场景 1：创建简单应用（只依赖 OSAL）

```makefile
# 源文件
myapp_SRCS := products/myproduct/myapp/src/main.c

# 头文件
myapp_CFLAGS := -Iinclude/osal

# 链接库
myapp_LDFLAGS := -L$(STAGING_DIR)/lib -losal -lpthread
```

### 场景 2：创建复杂应用（依赖多个库）

```makefile
# 源文件
myapp_SRCS := \
	products/myproduct/myapp/src/main.c \
	products/myproduct/myapp/src/module1.c \
	products/myproduct/myapp/src/module2.c

# 头文件
myapp_CFLAGS := \
	-Iproducts/myproduct/myapp/include \
	-Iinclude/osal \
	-Iinclude/hal \
	-Iinclude/pcl

# 链接库（按依赖顺序）
myapp_LDFLAGS := \
	-L$(STAGING_DIR)/lib \
	-Wl,--no-as-needed \
	-lpcl \
	-lhal \
	-losal \
	-Wl,--as-needed \
	-lpthread
```

### 场景 3：创建库（导出头文件）

```makefile
# 源文件
mylib_SRCS := \
	products/myproduct/mylib/src/api.c \
	products/myproduct/mylib/src/internal.c

# 头文件
mylib_CFLAGS := \
	-Iproducts/myproduct/mylib/include \
	-Iinclude/osal

# 链接库（动态库）
mylib_LDFLAGS := \
	-L$(STAGING_DIR)/lib \
	-Wl,-soname,libmylib.so.1 \
	-losal

# 导出头文件
mylib_HEADERS := \
	mylib.h \
	mylib_types.h \
	subdir/mylib_api.h
```

## 注意事项

1. **命名规范**：
   - 变量名使用小写：`myapp_SRCS`
   - 配置项使用大写：`CONFIG_BUILD_MYAPP`

2. **路径规范**：
   - 源文件使用完整路径：`products/xxx/src/main.c`
   - 头文件使用 `-I` 指定目录

3. **依赖顺序**：
   - 链接库按依赖顺序：高层库在前，底层库在后
   - 例如：`-lpcl -lhal -losal`

4. **配置集成**：
   - 在 Kconfig 中添加配置项
   - 在 module.mk 中使用 `CONFIG_XXX` 变量

5. **头文件导出**：
   - 库的头文件应该导出到 staging 目录
   - 应用程序从 staging 目录引用头文件

## 故障排除

### 找不到头文件

```bash
# 检查头文件是否在 staging 目录
ls -la .staging/include/

# 检查 CFLAGS 是否正确
make V=1 | grep "myapp"
```

### 找不到库文件

```bash
# 检查库是否构建
ls -la .staging/lib/

# 检查依赖顺序
ldd .staging/bin/myapp
```

### 链接错误

```bash
# 使用详细输出查看链接命令
make V=1

# 检查库的依赖关系
readelf -d .staging/lib/libmylib.so
```

## 参考

- [构建指南](../../docs/BUILD.md)
- [配置指南](../../docs/CONFIGURATION.md)
- [编码规范](../../docs/CODING_STANDARDS.md)
