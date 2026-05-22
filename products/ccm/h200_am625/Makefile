# =============================================================================
# H200 AM625 产品配置构建 - Kbuild 框架
# =============================================================================

# 定义库目标
lib-y += h200_am625
so-y += h200_am625

# 源文件列表
h200_am625-objs := \
	acl/ccm_acl_config.o \
	acl/H200_100P/acl_h200_100p.o \
	acl/H200_100P/acl_h200_100p_invalidation.o \
	pcl/pcl_h200_100p_base.o \
	pcl/pcl_h200_100p_v1.o \
	pcl/pcl_h200_100p_v2.o

# 编译标志
ccflags-y := -I$(srctree)/products/ccm/h200_am625
ccflags-y += -I$(srctree)/products/ccm/h200_am625/acl
# 注意：Core 模块头文件通过 staging 目录引用（顶层 Makefile 已全局导出）
# 注意：Kconfig 生成的配置文件在 $(srctree)/include
ccflags-y += -I$(srctree)/include

# 链接库
LDLIBS := -L$(LIB_DIR) -lacl -lpdl -lpcl -lhal -losal
