# SPDX-License-Identifier: GPL-2.0

lpf_core-y += lpf/soc/lpf_soc_adapter.o
lpf_core-$(CONFIG_LPF_SOC_ADAPTER_MOCK) += lpf/soc/mock/lpf_soc_mock.o
ifneq ($(CONFIG_LPF_SOC_ADAPTER_MOCK),y)
lpf_core-y += lpf/soc/generic_linux/lpf_soc_generic_linux.o
endif
