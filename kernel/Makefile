PDM_ROOT := $(abspath $(src)/..)
PDM_AUTOCONF := $(PDM_ROOT)/include/config/auto.conf
PDM_AUTOCONF_H := $(PDM_ROOT)/include/generated/gen_autoconf.h

-include $(PDM_AUTOCONF)

include $(src)/osal/Makefile
include $(src)/pdm-core/Makefile
include $(src)/pdm-configs/Makefile

ccflags-y += -I$(src)/include
ccflags-y += -I$(PDM_ROOT)/include
ccflags-y += -I$(src)/include/lpf
ccflags-y += -I$(src)/include/osal
ccflags-y += -I$(src)/pdm-core/core
ccflags-y += -I$(src)/pdm-core/protocol
ccflags-y += -I$(src)/pdm-core/compat
ccflags-y += -I$(src)/pdm-core/soc
ccflags-y += -I$(src)/pdm-core/include
ccflags-y += -I$(src)/pdm-configs/core
ccflags-y += -I$(src)/pdm-configs/parser/static
ccflags-y += -I$(src)/pdm-configs/parser/dt
ccflags-y += -I$(src)/pdm-core/hw
ccflags-y += -I$(src)/pdm-core/peripheral/mcu
ccflags-y += -I$(src)/pdm-core/peripheral/led
ccflags-y += -I$(src)/../uapi

ifneq ($(wildcard $(PDM_AUTOCONF_H)),)
ccflags-y += -include $(PDM_AUTOCONF_H)
endif
