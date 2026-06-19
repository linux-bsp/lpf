# ES-Middleware SDK

ES-Middleware is a Linux-focused embedded middleware framework built with
Kconfig, CMake, and Linux external-module Kbuild support.

## Current Scope

The repository currently contains framework core modules. Product-specific
satellite/PMC business code and the previous test product have been removed.

Current concrete peripheral/device type:

- MCU peripheral type in PCONFIG/PDM, exposed to userspace through PDI

The framework keeps the layered extension points so additional peripheral types can be added later without changing the core architecture.

## Core Layers

- OSAL: operating-system abstraction
- HAL: kernel hardware abstraction module (`hal.ko`)
- PCONFIG: platform hardware configuration registry
- PDM: kernel peripheral driver module
- PDI: userspace peripheral driver interface library
- ACONFIG: application configuration layer
- test_framework: userspace test infrastructure retained for future tests

## Build

```bash
make list
make kernel_x86_modules_defconfig
make modules
```

## Project Layout

```text
ES-Middleware/
├── core/          # Reusable middleware layers
├── configs/       # Development defconfigs
├── docs/          # Architecture and integration documentation
└── scripts/       # Kconfig/CMake build support
```

## Configuration

Defconfigs live under `configs/`, for example:

- `kernel_x86_modules_defconfig`

Use `make menuconfig` for interactive configuration.
