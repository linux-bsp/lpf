# LPF Project Context

LPF (Linux Peripheral Framework) is now a Linux-focused peripheral framework. Product-specific satellite/PMC business code has been removed.

## Current Architecture

Core modules:

- OSAL
- HAL
- PCONFIG
- PDM
- PDI
- ACONFIG

Current concrete peripheral/device family:

- MCU
- LED

The framework is still structured for later peripheral expansion. Add new peripheral families by extending PCONFIG types/accessors, PDM kernel modules/protocol helpers, PDI userspace APIs, Kconfig entries, and tests.

## Common Commands

```bash
make list
make kernel_x86_modules_defconfig
make modules
```

## Layering Rules

- Core modules must not depend on product code.
- Product/application code belongs outside shared framework module directories.
- PDM consumes PCONFIG through typed accessors and owns kernel-side peripheral logic.
- PDI exposes the userspace API and wraps the PDM ioctl ABI.
- HAL and OSAL remain platform abstraction layers.
