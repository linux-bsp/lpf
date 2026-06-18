obj-m += osal.o
osal-y := osal/src/osal_kernel.o

obj-m += pdm.o
pdm-y := pdm/src/pdm_main.o

ccflags-y += -I$(src)/include
ccflags-y += -I$(src)/include/osal
ccflags-y += -I$(src)/include/hal
ccflags-y += -I$(src)/include/hal/config
ccflags-y += -I$(src)/include/pconfig
ccflags-y += -I$(src)/include/pdm
ccflags-y += -I$(src)/include/prl
ccflags-y += -I$(src)/../uapi
