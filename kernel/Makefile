LPF_ROOT := $(abspath $(src)/..)
LPF_AUTOCONF := $(LPF_ROOT)/include/config/auto.conf
LPF_AUTOCONF_H := $(LPF_ROOT)/include/generated/gen_autoconf.h

-include $(LPF_AUTOCONF)

include $(src)/osal/Makefile
include $(src)/lpf-core/Makefile
include $(src)/lpf-configs/Makefile

ccflags-y += -I$(src)/include
ccflags-y += -I$(LPF_ROOT)/include
ccflags-y += -I$(src)/include/lpf
ccflags-y += -I$(src)/include/osal
ccflags-y += -I$(src)/lpf-core/core
ccflags-y += -I$(src)/lpf-core/protocol
ccflags-y += -I$(src)/lpf-core/compat
ccflags-y += -I$(src)/lpf-core/soc
ccflags-y += -I$(src)/lpf-core/include
ccflags-y += -I$(src)/lpf-core/config/src
ccflags-y += -I$(src)/lpf-core/hw
ccflags-y += -I$(src)/lpf-core/peripheral/mcu
ccflags-y += -I$(src)/lpf-core/peripheral/led
ccflags-y += -I$(src)/../uapi

ifneq ($(wildcard $(LPF_AUTOCONF_H)),)
ccflags-y += -include $(LPF_AUTOCONF_H)
endif
