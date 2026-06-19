# LPF Architecture

LPF (Linux Peripheral Framework) separates kernel modules from userspace API
libraries while keeping Kconfig-driven feature selection and CMake-driven source
builds. Its main job is to provide a reusable Linux peripheral access stack:
platform configuration, kernel hardware access, peripheral drivers, stable UAPI
headers, and userspace API wrappers.

LPF is intentionally narrower than a general Linux software framework. Product
logic, service lifecycle, deployment policy, and application behavior stay
outside the shared framework layers.

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
PDM + PDM protocol
        ↓
PCONFIG
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
- PDM provides kernel-side peripheral driver modules.
- PDM protocol helpers provide kernel-side packet framing owned by PDM.
- PDI provides userspace APIs over the PDM ioctl ABI.
- ACONFIG stores userspace application-facing configuration mappings.

## Layer Responsibilities

### OSAL

OSAL hides Linux kernel or libc/POSIX API differences from framework code. The
kernel and userspace implementations are separate, but public names should match
where the semantics are equivalent.

### HAL

HAL owns Linux hardware subsystem access in kernel mode. Current HAL support
includes CAN, serial, GPIO, PWM, I2C, and SPI. PDM calls exported HAL symbols;
userspace does not include or call HAL directly.

### PCONFIG

PCONFIG owns kernel-side platform hardware configuration tables. PDM queries
typed PCONFIG accessors during probe and binding. Product hardware tables live
outside core logic and are compiled into the selected configuration.

### PDM

PDM owns kernel-side peripheral devices, driver registration, configured-device
binding, ioctl dispatch, procfs debug nodes, and protocol helpers. Concrete
peripheral drivers such as MCU and LED live under PDM.

### UAPI

UAPI headers define the stable ioctl ABI shared between PDM and PDI. They must
remain valid for both kernel and userspace compilation and should avoid
kernel-internal types.

### PDI

PDI is the userspace C API layer. It opens the matching `/dev/pdm_*` node,
marshals requests through UAPI ioctls, and hides ioctl details from
applications.

### ACONFIG

ACONFIG provides application-facing configuration mapping. It is separate from
PCONFIG so application function metadata does not leak into hardware tables.

## Current Peripheral Scope

The current framework keeps one concrete peripheral/device family:

- MCU configuration in PCONFIG
- MCU protocol in PDM
- MCU driver in PDM
- Userspace access through PDI
- LED configuration in PCONFIG
- LED driver in PDM
- Userspace access through PDI

Other peripheral families can be added later by introducing matching PCONFIG
types, PDM protocol definitions, PDM kernel implementation, PDI userspace API
coverage, UAPI definitions when needed, and Kconfig entries.

## Build And Runtime Boundaries

- Userspace libraries are built by CMake from `user/`.
- Linux kernel modules are built by Kbuild from `kernel/`.
- Shared ABI headers live in `uapi/`.
- Generated configuration and version headers are produced under `include/`.
- Build outputs are written under `_build/`.

## Kernel Module Load Order

Load kernel modules in dependency order:

```text
osal.ko
pconfig.ko
hal.ko
pdm.ko
```

`pdm.ko` consumes PConfig entries and HAL symbols, then probes linked PDM
drivers for each configured peripheral.

## Adding A Peripheral

Add the following pieces together:

- PCONFIG type and platform config array entry.
- PDM driver object registered with `pdm_driver_register`.
- Optional PDM character device `/dev/pdm_<peripheral>` when userspace access
  is needed.
- UAPI header `uapi/pdi/pdi_<peripheral>.h` following
  `docs/PDI_UAPI_ABI.md`.
- Userspace wrapper `user/pdi/src/pdi_<peripheral>.c`.
- Kbuild object selection for the new PDM driver.

Feature selection stays at object/list registration boundaries. Kconfig may
select which objects are linked, but business logic should not branch on
feature macros.

Keep kernel/userspace changes aligned. A peripheral exposed to userspace should
add or update PCONFIG, PDM, UAPI, PDI, and Kconfig coverage together so the ABI
and build configuration remain consistent.

## Dependency Rules

- Core modules do not depend on product/application code.
- Dependencies point downward through the layer stack.
- Product-specific behavior belongs outside shared framework module directories.
- Kernel hardware tables are compiled through PCONFIG and consumed by PDM
  through typed accessors.
- Userspace code must use PDI/UAPI rather than including kernel-internal HAL,
  PCONFIG, or PDM headers.
