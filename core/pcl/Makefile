# ============================================================================
# PCL 模块构建 - Kbuild 风格
# ============================================================================

# 静态库
obj-s := libpcl.a

# 动态库
obj-d := libpcl.so

# 子目录递归
subdir-y += src/

# 从子目录收集对象文件
libpcl.a-objs := $(obj-y)
libpcl.so-objs := $(obj-y)

# 编译标志
CFLAGS_LIB := -I$(srctree)/core/pcl/include
CFLAGS_LIB += -I$(srctree)/core/pcl/include/api
CFLAGS_LIB += -I$(srctree)/core/hal/include
CFLAGS_LIB += -I$(srctree)/core/osal/include
CFLAGS_LIB += -I$(srctree)/include

# 链接库
LDLIBS := -L$(objtree)/lib -lhal -losal

# 包含统一构建框架
include $(scriptdir)/build/Makefile.build
