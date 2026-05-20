# ============================================================================
# HAL 模块构建 - Kbuild 风格
# ============================================================================

# 静态库
obj-s := libhal.a

# 动态库
obj-d := libhal.so

# 子目录递归
subdir-y += src/

# 从子目录收集对象文件
libhal.a-objs := $(obj-y)
libhal.so-objs := $(obj-y)

# 编译标志
CFLAGS_LIB := -I$(srctree)/core/hal/include
CFLAGS_LIB += -I$(srctree)/core/hal/include/config
CFLAGS_LIB += -I$(srctree)/core/osal/include
CFLAGS_LIB += -I$(srctree)/include

# 链接库
LDLIBS := -L$(objtree)/lib -losal

# 导出的头文件
EXPORT_HEADERS := \
	hal_can.h \
	hal_serial.h \
	hal_i2c.h \
	hal_spi.h \
	hal_gpio.h \
	hal_watchdog.h \
	config/hal_can_config.h \
	config/hal_serial_config.h \
	config/hal_i2c_config.h \
	config/hal_spi_config.h \
	config/hal_gpio_config.h \
	config/hal_watchdog_config.h

# 包含统一构建框架
include $(scriptdir)/build/Makefile.build
