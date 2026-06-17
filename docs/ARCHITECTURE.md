# ES-Middleware Architecture

ES-Middleware is a generic embedded middleware framework. It is organized as reusable core layers plus optional product/application layers.

## Layer Order

```text
Application/Product code
        ↓
ACONFIG
        ↓
PDL
        ↓
PCONFIG + PRL
        ↓
HAL
        ↓
OSAL
        ↓
Operating system / hardware
```

## Core Modules

- OSAL provides portable operating-system APIs.
- HAL provides hardware driver abstractions.
- PCONFIG stores platform hardware configuration tables.
- PRL provides packet framing and device-message helpers.
- PDL provides high-level peripheral drivers.
- ACONFIG stores application-facing configuration mappings.

## Current Peripheral Scope

The current framework keeps one concrete peripheral/device family:

- MCU configuration in PCONFIG
- MCU protocol in PRL
- MCU driver in PDL

Other peripheral families can be added later by introducing matching PCONFIG types, PRL device protocol definitions, PDL public headers, PDL source subdirectories, and Kconfig entries.

## Dependency Rules

- Core modules do not depend on product/application code.
- Dependencies point downward through the layer stack.
- Product-specific behavior belongs outside `core/`.
- Hardware tables are registered through PCONFIG and consumed by PDL through typed accessors.
