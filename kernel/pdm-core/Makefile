# SPDX-License-Identifier: GPL-2.0

obj-$(CONFIG_PDM_CORE) += pdm_core.o

include $(src)/pdm-core/core/Makefile
include $(src)/pdm-core/protocol/Makefile
include $(src)/pdm-core/compat/Makefile
include $(src)/pdm-core/soc/Makefile
include $(src)/pdm-core/hw/Makefile
include $(src)/pdm-core/hw/selftest/Makefile
include $(src)/pdm-core/runtime/Makefile
include $(src)/pdm-core/peripheral/mcu/Makefile
include $(src)/pdm-core/peripheral/led/Makefile
pdm_core-$(CONFIG_PDM_RUNTIME) += pdm-core/runtime/pdm_runtime_entry_end.o
include $(src)/pdm-core/peripheral/selftest/Makefile
