# SPDX-License-Identifier: GPL-2.0

obj-$(CONFIG_LPF_CORE) += lpf_core.o
obj-$(CONFIG_LPF_PERIPHERAL_RUNTIME) += lpf_peripheral_runtime.o
obj-$(CONFIG_LPF_HW_MOCK_SELFTEST) += lpf_hw_mock_selftest.o
obj-$(CONFIG_LPF_DUMMY_SERVICE_SELFTEST) += lpf_dummy_service_selftest.o

include $(src)/lpf/core/Makefile
include $(src)/lpf/protocol/Makefile
include $(src)/lpf/compat/Makefile
include $(src)/lpf/soc/Makefile
include $(src)/lpf/config/Makefile
include $(src)/lpf/hw/Makefile
include $(src)/lpf/hw/selftest/Makefile
include $(src)/lpf/peripheral/Makefile
include $(src)/lpf/peripheral/mcu/Makefile
include $(src)/lpf/peripheral/led/Makefile
include $(src)/lpf/transport/mcu/Makefile
include $(src)/lpf/config/selftest/Makefile
lpf_peripheral_runtime-y += lpf/peripheral/lpf_peripheral_entry_end.o
include $(src)/lpf/peripheral/selftest/Makefile
