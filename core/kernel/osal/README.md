# Kernel OSAL

Kernel OSAL wraps Linux kernel APIs needed by kernel HAL, PConfig, and PDM.

It is intentionally separate from `core/user/osal` and should not try to mirror
every userspace OSAL API. Add wrappers only when kernel modules need them.

The compatibility entry `core/kernel/include/osal/osal.h` exists for
kernel-side modules that still share OSAL-style call names with transitional
core code. Keep that surface small and backed by Linux kernel primitives.
