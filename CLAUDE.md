# ES-Middleware Project Context

ES-Middleware is now a Linux-focused embedded middleware framework. Product-specific satellite/PMC business code has been removed.

## Current Architecture

Core modules:

- OSAL
- HAL
- PCONFIG
- PDM
- PDI
- ACONFIG
- test_framework

Current concrete peripheral/device family:

- MCU only

The framework is still structured for later peripheral expansion. Add new peripheral families by extending PCONFIG types/accessors, PDM kernel modules/protocol helpers, PDI userspace APIs, Kconfig entries, and tests.

## Common Commands

```bash
make list
make kernel_x86_modules_defconfig
make modules
```

## Layering Rules

- Core modules must not depend on product code.
- Product/application code belongs outside shared middleware module directories.
- PDM consumes PCONFIG through typed accessors and owns kernel-side peripheral logic.
- PDI exposes the userspace API and wraps the PDM ioctl ABI.
- HAL and OSAL remain platform abstraction layers.
