# SPDX-License-Identifier: GPL-2.0

ifeq ($(CONFIG_PCONFIG),y)
obj-m += pconfig.o
pconfig-y := pconfig/src/pconfig_api.o
pconfig-y += pconfig/src/pconfig_backend.o
pconfig-y += pconfig/src/pconfig_static_backend.o
pconfig-y += pconfig/src/pconfig_validator.o
pconfig-y += pconfig/configs/pconfig_configs.o
pconfig-y += pconfig/configs/kernel/x86_modules/1.0.0/pconfig_kernel_x86_modules.o
endif
