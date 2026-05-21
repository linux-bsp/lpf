# Staging 目录说明

**日期**: 2026-05-22  
**版本**: 1.0.0

## 概述

EMS 构建系统使用统一的 **staging 目录**来管理所有构建产物，这是一个符合行业标准的做法，被 Buildroot、Yocto 等主流构建系统广泛采用。

## 目录结构

```
EMS/
├── staging/                    # 统一的构建产物输出目录
│   ├── bin/                   # 可执行文件
│   │   ├── ccm_collector
│   │   ├── ccm_logger
│   │   ├── ccm_health
│   │   ├── ccm_supervisor
│   │   └── ccm_comm
│   ├── lib/                   # 库文件
│   │   ├── libosal.a         # 静态库
│   │   ├── libosal.so        # 动态库
│   │   ├── libhal.a
│   │   ├── libhal.so
│   │   ├── libpcl.a
│   │   ├── libpcl.so
│   │   ├── libpdl.a
│   │   ├── libpdl.so
│   │   ├── libacl.a
│   │   ├── libacl.so
│   │   ├── libccm.a
│   │   ├── libccm.so
│   │   ├── libh200_am625.a
│   │   ├── libh200_am625.so
│   │   └── modules/          # 内核模块（预留）
│   └── include/              # 头文件（Staging）
│       ├── osal.h
│       ├── osal_types.h
│       ├── hal_can.h
│       ├── pcl.h
│       ├── pdl_bmc.h
│       ├── acl_api.h
│       ├── ipc/              # IPC 相关头文件
│       │   ├── osal_mutex.h
│       │   └── osal_semaphore.h
│       ├── lib/              # 库相关头文件
│       │   ├── osal_errno.h
│       │   └── osal_heap.h
│       ├── net/              # 网络相关头文件
│       │   └── osal_socket.h
│       ├── sys/              # 系统相关头文件
│       │   ├── osal_thread.h
│       │   └── osal_time.h
│       └── util/             # 工具相关头文件
│           └── osal_log.h
├── core/                      # 核心模块源码
├── products/                  # 产品模块源码
└── ...
```

## 优势

### 1. 目录结构更清晰

所有构建产物集中在 `staging/` 目录下，源码目录保持干净：

```bash
# 构建产物
staging/
├── bin/
├── lib/
└── include/

# 源码目录
core/
products/
```

### 2. 符合行业标准

- **Buildroot**: 使用 `$(HOST_DIR)` 和 `$(STAGING_DIR)`
- **Yocto**: 使用 `${STAGING_DIR}` 和 `${DEPLOY_DIR}`
- **Linux Kernel**: 使用 `INSTALL_MOD_PATH` 和 `INSTALL_HDR_PATH`

### 3. 便于清理

只需删除 staging 目录即可清理所有构建产物：

```bash
# 清理所有构建产物
make clean

# 或手动清理
rm -rf staging/
```

### 4. 便于打包

直接打包 staging 目录即可：

```bash
# 打包所有构建产物
tar czf ems-1.0.0.tar.gz staging/

# 或创建 deb/rpm 包
fpm -s dir -t deb -n ems -v 1.0.0 staging/=/usr/local/
```

### 5. 避免污染源码树

所有输出都在 staging 下，源码目录不会被污染：

```bash
# 源码目录保持干净
$ ls
core/  products/  scripts/  Makefile  Kconfig  ...

# 构建产物在 staging 下
$ ls staging/
bin/  lib/  include/
```

### 6. 便于集成到其他构建系统

其他构建系统可以直接使用 staging 目录：

```makefile
# Buildroot package
define EMS_INSTALL_STAGING_CMDS
    cp -r $(@D)/staging/* $(STAGING_DIR)/
endef

define EMS_INSTALL_TARGET_CMDS
    cp -r $(@D)/staging/bin/* $(TARGET_DIR)/usr/bin/
    cp -r $(@D)/staging/lib/*.so $(TARGET_DIR)/usr/lib/
endef
```

## 使用方式

### 编译

```bash
# 配置
make menuconfig
# 或使用预定义配置
make ccm_h200_am625_debug_defconfig

# 编译
make -j$(nproc)

# 查看输出
ls -lh staging/bin/
ls -lh staging/lib/
ls -lh staging/include/
```

### 清理

```bash
# 清理构建产物（保留配置）
make clean

# 深度清理（删除配置）
make mrproper

# 手动清理 staging 目录
rm -rf staging/
```

### 安装

```bash
# 安装到系统目录（需要 root 权限）
sudo cp -r staging/bin/* /usr/local/bin/
sudo cp -r staging/lib/*.so /usr/local/lib/
sudo cp -r staging/include/* /usr/local/include/
sudo ldconfig

# 或使用 DESTDIR
make install DESTDIR=/tmp/ems-install
```

### 打包

```bash
# 创建 tar.gz 包
tar czf ems-1.0.0-$(uname -m).tar.gz staging/

# 创建 deb 包（需要 fpm）
fpm -s dir -t deb -n ems -v 1.0.0 \
    --prefix /usr/local \
    staging/bin=/usr/local/bin \
    staging/lib=/usr/local/lib \
    staging/include=/usr/local/include

# 创建 rpm 包（需要 fpm）
fpm -s dir -t rpm -n ems -v 1.0.0 \
    --prefix /usr/local \
    staging/bin=/usr/local/bin \
    staging/lib=/usr/local/lib \
    staging/include=/usr/local/include
```

## 变量说明

在 Makefile 中，以下变量指向 staging 目录：

```makefile
# Staging 根目录
STAGING_DIR := $(objtree)/staging

# 子目录
BIN_DIR     := $(STAGING_DIR)/bin
LIB_DIR     := $(STAGING_DIR)/lib
KO_DIR      := $(STAGING_DIR)/lib/modules
INCLUDE_DIR := $(STAGING_DIR)/include
```

在子目录 Makefile 中使用：

```makefile
# 应用程序 Makefile
app-y += myapp
myapp-objs := main.o utils.o

# 包含 staging 头文件
ccflags-y += -I$(INCLUDE_DIR)

# 链接 staging 库
myapp-ldflags := -L$(LIB_DIR) -losal -lhal
```

## 外部构建支持

staging 目录也支持外部构建（O=dir）：

```bash
# 在独立目录中构建
mkdir ../build
make O=../build menuconfig
make O=../build -j$(nproc)

# staging 目录在构建目录下
ls ../build/staging/
```

## Buildroot/Yocto 集成

### Buildroot 集成示例

```makefile
# package/ems/ems.mk
EMS_VERSION = 1.0.0
EMS_SITE = $(TOPDIR)/../EMS
EMS_SITE_METHOD = local
EMS_INSTALL_STAGING = YES

define EMS_CONFIGURE_CMDS
    $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) \
        CROSS_COMPILE=$(TARGET_CROSS) \
        ccm_h200_am625_release_defconfig
endef

define EMS_BUILD_CMDS
    $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) \
        CROSS_COMPILE=$(TARGET_CROSS) \
        -j$(PARALLEL_JOBS)
endef

define EMS_INSTALL_STAGING_CMDS
    # 安装头文件和库到 staging
    cp -r $(@D)/staging/include/* $(STAGING_DIR)/usr/include/
    cp -r $(@D)/staging/lib/*.a $(STAGING_DIR)/usr/lib/
    cp -r $(@D)/staging/lib/*.so $(STAGING_DIR)/usr/lib/
endef

define EMS_INSTALL_TARGET_CMDS
    # 安装可执行文件和动态库到目标
    $(INSTALL) -D -m 0755 $(@D)/staging/bin/* $(TARGET_DIR)/usr/bin/
    $(INSTALL) -D -m 0755 $(@D)/staging/lib/*.so $(TARGET_DIR)/usr/lib/
endef

$(eval $(generic-package))
```

### Yocto 集成示例

```bitbake
# recipes-ems/ems/ems_1.0.0.bb
SUMMARY = "EMS - Embedded Management System"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=..."

SRC_URI = "file://EMS"
S = "${WORKDIR}/EMS"

inherit autotools

do_configure() {
    oe_runmake ccm_h200_am625_release_defconfig
}

do_compile() {
    oe_runmake -j${@oe.utils.cpu_count()}
}

do_install() {
    # 安装到 ${D}
    install -d ${D}${bindir}
    install -m 0755 ${S}/staging/bin/* ${D}${bindir}/
    
    install -d ${D}${libdir}
    install -m 0644 ${S}/staging/lib/*.a ${D}${libdir}/
    install -m 0755 ${S}/staging/lib/*.so ${D}${libdir}/
    
    install -d ${D}${includedir}
    cp -r ${S}/staging/include/* ${D}${includedir}/
}

FILES_${PN} = "${bindir}/* ${libdir}/*.so"
FILES_${PN}-dev = "${includedir}/* ${libdir}/*.a"
```

## 常见问题

### Q: 为什么不直接输出到 bin/、lib/、include/？

A: 使用 staging 目录有以下优势：
1. 符合行业标准（Buildroot、Yocto）
2. 目录结构更清晰
3. 便于清理和打包
4. 避免污染源码树
5. 便于集成到其他构建系统

### Q: staging 目录会被 git 跟踪吗？

A: 不会。staging 目录已添加到 `.gitignore`：

```gitignore
# 构建产物
/staging/
```

### Q: 如何在应用程序中使用 staging 的库？

A: 使用 `-L$(LIB_DIR)` 和 `-I$(INCLUDE_DIR)`：

```makefile
# 应用程序 Makefile
ccflags-y += -I$(INCLUDE_DIR)
myapp-ldflags := -L$(LIB_DIR) -losal -lhal
```

### Q: 如何部署到目标板？

A: 直接复制 staging 目录：

```bash
# 方法 1: 直接复制
scp -r staging/* root@target:/usr/local/

# 方法 2: 打包后复制
tar czf ems.tar.gz staging/
scp ems.tar.gz root@target:/tmp/
ssh root@target "cd /usr/local && tar xzf /tmp/ems.tar.gz --strip-components=1"

# 方法 3: 使用 rsync
rsync -av staging/ root@target:/usr/local/
```

## 参考文档

- [BUILD_GUIDE.md](BUILD_GUIDE.md) - 构建指南
- [BUILD_SYSTEM.md](BUILD_SYSTEM.md) - 构建系统详解
- [ARCHITECTURE.md](ARCHITECTURE.md) - 架构设计
- [CLAUDE.md](../CLAUDE.md) - AI 助手指南

---

**最后更新**: 2026-05-22  
**维护者**: wanguo  
**分支**: feature/kconfig-integration
