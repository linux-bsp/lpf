# SPDX-License-Identifier: GPL-2.0

pdm_core-y += pdm-core/soc/pdm_soc_adapter.o
pdm_core-$(CONFIG_PDM_SOC_ADAPTER_MOCK) += pdm-core/soc/mock/pdm_soc_mock.o
ifneq ($(CONFIG_PDM_SOC_ADAPTER_MOCK),y)
pdm_core-y += pdm-core/soc/generic_linux/pdm_soc_generic_linux.o
endif
