# PDM - Linux Peripheral Framework

PDM (Linux Peripheral Framework) is a Linux-focused peripheral access framework.
It provides the shared layers needed to describe platform devices, access
hardware from kernel modules, expose stable kernel/userspace ABIs, and offer
application-facing C APIs.

PDM is not limited to embedded products. The current modules are useful for
embedded Linux boards, industrial controllers, development machines, and other
Linux systems that need a reusable peripheral driver and access stack.

The build system combines Kconfig feature selection, CMake userspace builds,
and Linux external-module Kbuild support.

## Current Scope

The repository currently contains framework core modules. Product-specific
satellite/PMC business code and the previous test product have been removed.

Current concrete peripheral/device types:

- MCU peripheral type in PDM runtime config, exposed to
  userspace through PDI
- LED peripheral type in PDM runtime config, exposed to
  userspace through PDI

The framework keeps layered extension points so additional peripheral types can
be added later without changing the core architecture.

PDM currently targets Linux only. OSAL still separates operating-system-facing
APIs from higher layers, but non-Linux ports are outside the current direction.

## Core Layers

- OSAL: operating-system abstraction
- PDM Configs: DTS-like static board description provider
  (`pdm_configs.ko`)
- PDM Core: framework device model, shared kernel infrastructure, integrated
  runtime orchestration, PDM HW, runtime config backends, and reusable
  peripheral services (`pdm_core.ko`)
- PDM HW: framework-owned hardware access APIs linked into `pdm_core.ko`
- PDM Runtime Config: platform hardware configuration registry linked into
  `pdm_core.ko`
- PDI: userspace peripheral driver interface library
- ACONFIG: application configuration layer

## What PDM Provides

- A clear split between kernel modules, UAPI headers, and userspace libraries.
- Kernel-side peripheral drivers built on PDM runtime config, PDM HW APIs,
  and OSAL.
- Userspace PDI libraries that hide ioctl details from applications.
- Kconfig-controlled feature selection for modules and peripheral families.
- CMake/Kbuild integration for userspace libraries and Linux kernel modules.
- A repeatable extension path for adding new peripheral types.

## What PDM Does Not Own

- Product or business logic.
- Product-specific init scripts, services, or deployment policy.
- Generic Linux application framework behavior unrelated to peripheral access.
- Generated build artifacts under `_build/`.

The previous userspace test product has been removed. New tests now live under
`tests/` with CMake/CTest integration. Current coverage includes UAPI ABI
layout checks, runtime config normalization checks, PDI userspace API
validation/error-path smoke tests, and syscall-mocked PDI ioctl operation-path
tests.

## Quick Start

```bash
make list
make ubuntu_x86_modules_defconfig
make all
```

To build only the kernel modules:

```bash
make modules
```

To build and run the current test targets:

```bash
make tests
```

To build the no-hardware kernel mock preset:

```bash
make ubuntu_x86_mock_modules_defconfig
make modules
```

That preset also builds `pdm_configs.ko` and `pdm_hw_mock_selftest.ko`;
loading the modules in order runs PDM HW GPIO/PWM/CAN/UART/I2C/SPI checks
through the mock SoC adapter.

Kernel module load order is `osal.ko`, `pdm_configs.ko`, then `pdm_core.ko`.

Generated libraries are written under `_build/lib/`. Kernel modules are written
under `_build/modules/`.

## Configuration

Defconfigs live under `configs/`, for example:

- `ubuntu_x86_modules_defconfig`
- `ubuntu_x86_mock_modules_defconfig`

Use `make menuconfig` for interactive configuration.

## Project Layout

```text
PDM/
├── kernel/        # Kernel modules and kernel-side headers
├── user/          # Userspace libraries
├── uapi/          # Shared userspace/kernel ABI headers
├── configs/       # Development defconfigs
├── docs/          # Architecture and integration documentation
├── tests/         # ABI, mock, and integration test targets
└── scripts/       # Kconfig/CMake build support
```

## Documentation

- `docs/architecture.md`: current module architecture and extension flow.
- `docs/pdm_target_architecture.md`: target layered architecture.
- `docs/pdm_module_boundaries.md`: allowed dependencies and forbidden shortcuts.
- `docs/pdm_refactor_roadmap.md`: staged cleanup roadmap.
- `docs/pdm_kernel_compat_policy.md`: supported kernel baseline and compat
  feature-gate rules.
- `docs/kernel_user_split.md`: kernel/userspace boundary and include rules.
- `docs/pdm_long_term_optimization_plan.md`: long-term optimization plan for
  multi-kernel and multi-SoC deployments.
- `docs/pdm_uapi_abi.md`: UAPI ABI and PDI wrapper rules for new peripherals.
