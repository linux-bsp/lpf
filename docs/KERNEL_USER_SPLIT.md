# Kernel And Userspace Split

The Linux-only direction separates kernel modules from userspace API libraries.

## Target Layout

```text
core/
  kernel/
    Makefile
    include/
      osal/          # kernel-side cross-module OSAL headers
      hal/           # kernel-side cross-module HAL headers
      pconfig/       # kernel-side cross-module PConfig headers
      pdm/           # kernel-side cross-module PDM headers
    osal/
      src/           # builds osal.ko
    hal/
      src/           # kernel-only hardware access implementation
    pconfig/
      src/           # kernel-side configuration implementation
    pdm/
      src/           # builds pdm.ko, owns ioctl dispatch and protocol helpers

  user/
    osal/            # userspace OSAL library
    aconfig/         # userspace application configuration
    pdi/             # userspace API library for PDM
    test_framework/  # userspace-only test support library

  uapi/
    pdi/             # ioctl ABI shared by PDM and PDI
```

## Responsibilities

- `core/kernel/osal` wraps Linux kernel APIs and builds `osal.ko`.
- `core/kernel/pdm` owns the kernel module entry, device node, ioctl boundary,
  and links kernel-side PCONFIG/PDM protocol objects into `pdm.ko` when enabled.
- `core/kernel/hal` provides kernel-only hardware access used by PDM.
  It builds as `hal.ko` and exports HAL API symbols for PDM.
- `core/kernel/pconfig` provides kernel-side platform/product configuration used
  by PDM and HAL; it is currently linked into `pdm.ko`, not built as a separate
  module.
- `core/user/pdi` provides the application-facing C API and wraps open/ioctl.
- `core/user/test_framework` provides userspace-only test infrastructure.
- `core/uapi/pdi` is the stable ABI shared by `core/kernel/pdm` and
  `core/user/pdi`.

## Include Rules

- Kernel cross-module headers live under `core/kernel/include/<module>/`.
- UAPI headers live under `core/uapi/pdi/` and must be valid for both kernel and
  userspace builds.
- Userspace code must not include non-UAPI kernel headers.
