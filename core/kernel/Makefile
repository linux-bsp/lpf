ESMW_ROOT := $(abspath $(src)/../..)
ESMW_AUTOCONF := $(ESMW_ROOT)/include/config/auto.conf
ESMW_AUTOCONF_H := $(ESMW_ROOT)/include/generated/gen_autoconf.h

-include $(ESMW_AUTOCONF)

ifeq ($(CONFIG_OSAL),y)
obj-m += osal.o
osal-y := osal/src/osal_kernel.o
endif

ifeq ($(CONFIG_PDM),y)
obj-m += pdm.o
pdm-y := pdm/src/pdm_main.o
pdm-$(CONFIG_PCONFIG) += pconfig/src/pconfig_api.o
pdm-$(CONFIG_PRL) += prl/src/prl_api.o
pdm-$(CONFIG_PRL) += prl/src/prl_common.o
pdm-$(CONFIG_PRL) += prl/src/prl_device.o
pdm-$(CONFIG_PDM_MCU_SUPPORT) += pdm/src/pdm_mcu/pdm_mcu.o
pdm-$(CONFIG_PDM_MCU_SUPPORT) += pdm/src/pdm_mcu/pdm_mcu_can.o
pdm-$(CONFIG_PDM_MCU_SUPPORT) += pdm/src/pdm_mcu/pdm_mcu_serial.o
pdm-$(CONFIG_PDM_MCU_SUPPORT) += hal/src/kernel/hal_can_kernel.o
pdm-$(CONFIG_PDM_MCU_SUPPORT) += hal/src/kernel/hal_serial_kernel.o
endif

ccflags-y += -I$(src)/include
ccflags-y += -I$(src)/include/osal
ccflags-y += -I$(src)/include/hal
ccflags-y += -I$(src)/include/hal/config
ccflags-y += -I$(src)/include/pconfig
ccflags-y += -I$(src)/include/pdm
ccflags-y += -I$(src)/include/prl
ccflags-y += -I$(src)/../uapi

ifneq ($(wildcard $(ESMW_AUTOCONF_H)),)
ccflags-y += -include $(ESMW_AUTOCONF_H)
endif

ccflags-$(CONFIG_PDM_MCU_SUPPORT) += -DCONFIG_PDM_MCU_SUPPORT=1
ccflags-$(CONFIG_PRL_MCU) += -DCONFIG_PRL_MCU=1
ccflags-$(CONFIG_HAL_CAN) += -DCONFIG_HAL_CAN=1
ccflags-$(CONFIG_HAL_UART) += -DCONFIG_HAL_UART=1
