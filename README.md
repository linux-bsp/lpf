# ES-Middleware SDK

ES-Middleware is a Linux-focused embedded middleware framework built with Kconfig and CMake. It provides reusable core layers for userspace API libraries and kernel modules.

## Current Scope

The repository currently contains framework core modules and the core test product. Product-specific satellite/PMC business code has been removed.

Current concrete peripheral/device type:

- MCU peripheral type in PCONFIG/PDM, exposed to userspace through PDI

The framework keeps the layered extension points so additional peripheral types can be added later without changing the core architecture.

## Core Layers

- OSAL: operating-system abstraction
- HAL: hardware abstraction
- PCONFIG: platform hardware configuration registry
- PDM: kernel peripheral driver module
- PDI: userspace peripheral driver interface library
- ACONFIG: application configuration layer
- test_framework: core test infrastructure

## Build

```bash
make list
make ctest_x86_minimal_defconfig
make all
```

Run the generated test binary:

```bash
./_build/bin/es-middleware-test --all
```

## Project Layout

```text
ES-Middleware/
├── core/          # Reusable middleware layers
├── products/ctest # Core module test product
├── configs/ctest  # Test and development defconfigs
├── docs/          # Architecture and integration documentation
└── scripts/       # Kconfig/CMake build support
```

## Configuration

Defconfigs live under `configs/ctest/` and are named with the `ctest_` prefix, for example:

- `ctest_x86_minimal_defconfig`
- `ctest_x86_full_defconfig`
- `ctest_arm64_minimal_defconfig`
- `ctest_arm64_full_defconfig`

Use `make menuconfig` for interactive configuration.
