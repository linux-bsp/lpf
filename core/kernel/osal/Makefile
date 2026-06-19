# SPDX-License-Identifier: GPL-2.0

# Kbuild fragment for osal.ko.
ifeq ($(CONFIG_OSAL),y)
obj-m += osal.o
osal-y := osal/src/osal_kernel.o
osal-y += osal/src/ipc/osal_mutex.o
osal-y += osal/src/lib/osal_heap.o
osal-y += osal/src/lib/osal_string.o
osal-y += osal/src/sys/osal_thread.o
osal-y += osal/src/util/osal_log.o
endif
