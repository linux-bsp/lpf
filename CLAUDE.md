# ES-Middleware Project Context

ES-Middleware is now a generic embedded middleware framework. Product-specific satellite/PMC business code has been removed.

## Current Architecture

Core modules:

- OSAL
- HAL
- PCONFIG
- PRL
- PDL
- ACONFIG
- test_framework

Current concrete peripheral/device family:

- MCU only

The framework is still structured for later peripheral expansion. Add new peripheral families by extending PCONFIG types/accessors, PRL device protocol definitions, PDL public headers/source subdirectories, Kconfig entries, and tests.

## Common Commands

```bash
make list
make ctest_x86_minimal_defconfig
make all
./_build/bin/es-middleware-test --all
```

## Layering Rules

- Core modules must not depend on product code.
- Product/application code belongs outside `core/`.
- PDL consumes PCONFIG through typed accessors.
- PRL owns protocol framing and device-message encode/decode helpers.
- HAL and OSAL remain platform abstraction layers.
