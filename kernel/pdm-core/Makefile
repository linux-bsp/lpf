# SPDX-License-Identifier: GPL-2.0

obj-$(CONFIG_PDM_CORE) += pdm_core.o

# New Linux bus_type implementation
include $(src)/pdm-core/bus/Makefile

include $(src)/pdm-core/core/Makefile
include $(src)/pdm-core/protocol/Makefile
include $(src)/pdm-core/compat/Makefile
# include $(src)/pdm-core/soc/Makefile  # 已删除：无使用者
# include $(src)/pdm-core/hw/Makefile  # 已删除：无使用者
# include $(src)/pdm-core/hw/selftest/Makefile  # 已删除：无使用者
# include $(src)/pdm-core/runtime/Makefile  # 已删除：无实际作用
include $(src)/pdm-core/peripheral/mcu/Makefile
include $(src)/pdm-core/peripheral/led/Makefile
include $(src)/pdm-core/peripheral/selftest/Makefile
