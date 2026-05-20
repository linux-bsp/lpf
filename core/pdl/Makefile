# ============================================================================
# PDL 模块构建 - Kbuild 风格
# ============================================================================

# 静态库
obj-s := libpdl.a

# 动态库
obj-d := libpdl.so

# 子目录递归
subdir-y += src/

# 从子目录收集对象文件
libpdl.a-objs := $(obj-y)
libpdl.so-objs := $(obj-y)

# 编译标志
CFLAGS_LIB := -I$(srctree)/core/pdl/include
CFLAGS_LIB += -I$(srctree)/core/pcl/include
CFLAGS_LIB += -I$(srctree)/core/hal/include
CFLAGS_LIB += -I$(srctree)/core/osal/include
CFLAGS_LIB += -I$(srctree)/include

# 链接库
LDLIBS := -L$(objtree)/lib -lpcl -lhal -losal

# 包含统一构建框架
include $(scriptdir)/build/Makefile.build
