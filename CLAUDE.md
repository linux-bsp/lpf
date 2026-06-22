# PDM Project Context

PDM (Linux Peripheral Framework) is now a Linux-focused peripheral framework. Product-specific satellite/PMC business code has been removed.

## Current Architecture

Core modules:

- OSAL
- PDM HW
- PDM Core
- PDM Runtime
- PDM Runtime Config
- PDI
- ACONFIG

Current concrete peripheral/device family:

- MCU
- LED

The framework is still structured for later peripheral expansion. Add new peripheral families by extending PDM runtime config types/accessors, PDM peripheral services/protocol helpers, PDI userspace APIs, Kconfig entries, and tests.

## Common Commands

```bash
make list
make ubuntu_x86_modules_defconfig
make modules
```

## Layering Rules

- Core modules must not depend on product code.
- Product/application code belongs outside shared framework module directories.
- PDM runtime hosts runtime config loading and kernel-side peripheral services.
- PDI exposes the userspace API and wraps PDM UAPI ioctl ABIs.
- PDM HW owns framework hardware access above the SoC adapter; OSAL remains
  the operating-system abstraction layer.
