# ES-Middleware Architecture

ES-Middleware is a Linux-focused middleware framework. It separates kernel
modules from userspace API libraries while keeping Kconfig-driven feature
selection and CMake-driven source builds.

## Layer Order

```text
Application/Product code
        ↓
ACONFIG
        ↓
PDI (userspace API)
        ↓
ioctl / UAPI
        ↓
PDM
        ↓
PCONFIG + PRL
        ↓
HAL
        ↓
OSAL
        ↓
Linux kernel / hardware
```

## Core Modules

- User OSAL provides libc/POSIX wrappers for userspace libraries and tests.
- Kernel OSAL wraps Linux kernel APIs used by kernel modules.
- HAL provides kernel-side hardware access helpers.
- PCONFIG stores kernel-side platform hardware configuration tables.
- PRL provides kernel-side packet framing and device-message helpers.
- PDM provides kernel-side peripheral driver modules.
- PDI provides userspace APIs over the PDM ioctl ABI.
- ACONFIG stores userspace application-facing configuration mappings.

## Current Peripheral Scope

The current framework keeps one concrete peripheral/device family:

- MCU configuration in PCONFIG
- MCU protocol in PRL
- MCU driver in PDM
- Userspace access through PDI

Other peripheral families can be added later by introducing matching PCONFIG
types, PRL protocol definitions, PDM kernel implementation, PDI userspace API
coverage, UAPI definitions when needed, and Kconfig entries.

## Dependency Rules

- Core modules do not depend on product/application code.
- Dependencies point downward through the layer stack.
- Product-specific behavior belongs outside `core/`.
- Kernel hardware tables are compiled through PCONFIG and consumed by PDM
  through typed accessors.
- Userspace code must use PDI/UAPI rather than including kernel-internal HAL,
  PCONFIG, PRL, or PDM headers.
