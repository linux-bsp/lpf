# PDM Architecture

PDM (Linux Peripheral Framework) separates kernel modules from userspace API
libraries while keeping Kconfig-driven feature selection and CMake-driven source
builds. Its main job is to provide a reusable Linux peripheral access stack:
platform configuration, kernel hardware access, peripheral drivers, stable UAPI
headers, and userspace API wrappers.

PDM is intentionally narrower than a general Linux software framework. Product
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
PDM peripheral services + PDM protocol
        ↓
PDM runtime config mapping
        ↓
PDM HW API
        ↓
PDM SoC Adapter
        ↓
PDM Kernel Compat
        ↓
OSAL
        ↓
Linux kernel / hardware
```

## Core Modules

- User OSAL provides libc/POSIX wrappers for userspace libraries and tests.
- Kernel OSAL wraps Linux kernel APIs used by kernel modules.
- PDM Core owns the framework device and driver registry, lifecycle, device
  names, capability metadata, the active SoC adapter, and the integrated
  runtime entry used by `pdm_core.ko`.
- PDM Configs provides the selected static board description in
  `pdm_configs.ko`.
- PDM HW provides framework-owned kernel-side hardware access helpers linked
  into `pdm_core.ko`.
- PDM SoC Adapter isolates SoC or vendor BSP differences below PDM HW.
- PDM Kernel Compat isolates Linux kernel API differences below the generic
  Linux SoC adapter.
- PDM runtime config selects a kernel-side platform configuration backend and
  exposes configured-device nodes to the runtime.
- PDM peripheral services provide kernel-side peripheral business behavior.
- PDM protocol helpers provide kernel-side packet framing for services that use
  framed peripheral communication.
- PDM Core owns the control/discovery node for device snapshots.
- PDI provides userspace APIs over PDM UAPI ioctl nodes.
- ACONFIG stores userspace application-facing configuration mappings.

## Layer Responsibilities

### OSAL

OSAL hides Linux kernel or libc/POSIX API differences from framework code. The
kernel and userspace implementations are separate, but public names should match
where the semantics are equivalent.

### PDM HW

PDM HW is the framework-owned hardware access layer linked into
`pdm_core.ko`. It owns the kernel-side `pdm_hw_*` APIs consumed
by PDM peripheral services and their service-owned transport backends. Current
support includes CAN, UART, GPIO, PWM, I2C, and SPI. PDM HW calls the PDM SoC
Adapter instead of Linux subsystem or vendor BSP APIs directly. Public
kernel-internal PDM HW headers live under `kernel/include/pdm/hw/`, while HW
implementations are grouped by capability under `kernel/pdm-core/hw/`.

### PDM Core

PDM Core owns the framework-level device model. Peripheral services register as
PDM drivers, and configured device instances are registered as PDM devices. PDM
Core handles bind, probe, remove ordering, stable device names, and capability
metadata. It does not depend on PDM_CONFIG; callers map configuration backends into
PDM device configs before registration. PDM Core also initializes the default
SoC adapter path used by hardware access paths. Device discovery callers should
use the snapshot APIs (`pdm_device_list()`, `pdm_device_get_info_by_name()`,
and capability queries) instead of retaining internal `pdm_device_t` pointers
across lifecycle events. Kernel code that needs to keep a device active across
operations should use `pdm_device_get()` or the name/capability variants and
release the returned handle with `pdm_device_put()`. PDM Core emits kernel
device events for registration, bind, state changes, errors, remove start, and
remove completion. These events are kernel-only in PDM v1; userspace observes
lifecycle and health through synchronous `/dev/pdm_ctl`, sysfs, and procfs
snapshots.
It also provides reusable kernel infrastructure helpers for PDM instance
character devices, instance sysfs attributes, and debugfs command files so
peripheral services do not duplicate node lifecycle code.
The PDM control/discovery node `/dev/pdm_ctl` is implemented in PDM Core and
exposes read-only snapshots of the PDM device model through
`uapi/pdm/pdm_ctl.h`. Public kernel-internal PDM Core headers live under
`kernel/include/pdm/core/`.

### PDM SoC Adapter

The SoC adapter layer owns hardware backend differences below PDM hardware
access APIs. The current default backend is `generic-linux`, which calls PDM
kernel compat wrappers for CAN, serial, GPIO, PWM, I2C, and SPI. Kconfig
selects the default backend built into `pdm_core.ko`; `kernel/pdm-core/soc/mock/`
provides a deterministic mock backend for development and framework tests that
should not require live hardware. The mock preset can also build
`pdm_hw_mock_selftest.ko`, which runs PDM HW operation-path checks over
the mock backend, and `pdm_dummy_service_selftest.ko`, which runs PDM Core
dummy service lifecycle checks. Future SoC-specific adapters should live under
`kernel/pdm-core/soc/` and must keep vendor BSP calls out of hardware access APIs
and PDM peripheral services. Public kernel-internal SoC adapter headers live
under `kernel/include/pdm/soc/`.

### PDM Kernel Compat

The compat layer wraps Linux kernel API details that may vary across kernel
versions. Public kernel-internal compat headers live under
`kernel/include/pdm/compat/`. GPIO, PWM, I2C, and SPI wrappers currently live
under `kernel/pdm-core/compat/`, along with CAN and serial wrappers. Peripheral
services and PDM HW business-facing APIs should not use `LINUX_VERSION_CODE` or
vendor BSP APIs directly.

### PDM Runtime Config

PDM runtime config owns kernel-side platform hardware configuration
aggregation. It selects a backend, validates the active platform, and exposes
enabled configured-device nodes in one ordered table. Backend selection is
controlled by the `backend` module parameter on `pdm_core.ko`: `auto` tries
the static table exported by the already loaded `pdm_configs.ko` first and
falls back to Device Tree, while `static` and `dt` require a specific backend
and do not fall back. Runtime config backend code is linked into
`pdm_core.ko`; selected static C configs are built as `pdm_configs.ko`, which
exports board-description data and does not register devices. Static C configs
register themselves through a linker section in `pdm_configs.ko`, so product
selection stays at Kbuild/Kconfig object boundaries instead of central platform
extern tables. Static C configs should author the board description as
first-class configured-device node tables, with legacy
per-peripheral arrays kept only as compatibility payload storage where needed.
Future board-profile or product-selection backends should produce the same
runtime config model before PDM peripheral configuration sees the data. Public
kernel-internal runtime config headers live under `kernel/include/pdm/config/`.

### PDM Peripheral Services

PDM peripheral services own kernel-side peripheral business behavior, ioctl
dispatch, state, error handling, debug command handlers, and service-specific
transport backends. Services register with PDM Core for device lifecycle
handling and use PDM chrdev/sysfs/debugfs helpers for runtime nodes. MCU and
LED services now live under `kernel/pdm-core/peripheral/` and expose
instance nodes such as `/dev/pdm/mcu0` and `/dev/pdm/led0`; they remain
integrated through `pdm_core.ko` rather than being split into one KO per
peripheral.
`kernel/pdm-core/runtime/pdm_runtime.c` owns the unified runtime entry that
is called by `pdm_core.ko` module initialization. Public kernel-internal
peripheral headers live under `kernel/include/pdm/peripheral/`.

### PDM Protocol

PDM protocol helpers own reusable kernel-side packet framing for peripheral
services that need a standard message envelope. The current implementation
lives under `kernel/pdm-core/protocol/`, exports encode/decode entry points from
`pdm_core.ko`, and keeps protocol constants and MCU message definitions under
`kernel/include/pdm/protocol/`.

### PDM Integrated Runtime

The integrated runtime is linked into `pdm_core.ko`. It owns runtime entry
ordering, PDM HW initialization, per-service registration, configured-device
probing, and runtime config node-to-PDM-device orchestration.
Business operations stay on PDM instance nodes such as `/dev/pdm/mcu0` and
`/dev/pdm/led0`; PDM service status snapshots live under `/proc/pdm/`.

### UAPI

UAPI headers define the stable ioctl ABI shared between PDM kernel nodes and
PDI. They must remain valid for both kernel and userspace compilation and
should avoid kernel-internal types.

### PDI

PDI is the userspace C API layer. It opens the matching `/dev/pdm/*` node,
marshals requests through UAPI ioctls, and hides ioctl details from
applications. Discovery APIs use the PDM control node `/dev/pdm_ctl` to list
PDM device snapshots and look up devices by stable name or capability.

### ACONFIG

ACONFIG provides application-facing configuration mapping. It is separate from
PDM_CONFIG so application function metadata does not leak into hardware tables.

## Current Peripheral Scope

The current framework keeps one concrete peripheral/device family:

- MCU configuration in PDM runtime config
- MCU protocol in PDM protocol
- MCU service in PDM peripheral layer
- MCU CAN/UART transport backends in the MCU peripheral implementation
- Userspace access through PDI
- LED configuration in PDM runtime config
- LED service in PDM peripheral layer
- Userspace access through PDI

Other peripheral families can be added later by introducing matching runtime
config types, PDM peripheral-service implementations, PDI userspace API
coverage, UAPI definitions when needed, and Kconfig entries. Protocol
definitions are only needed for peripherals that use framed PDM protocol
communication.

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
pdm_configs.ko
pdm_core.ko
```

`pdm_configs.ko` hosts the selected DTS-like static board description and only
depends on OSAL. `pdm_core.ko` hosts the PDM device model plus the integrated
runtime: configuration backend selection, PDM HW hardware access objects,
framework-hosted peripheral services, configured-device traversal, and PDM Core
driver probing.

## Adding A Peripheral

Add the following pieces together:

- Runtime config type and configured-device node payload.
- PDM config device-node payload and runtime config driver.
- PDM device type and capability mapping for the configured node.
- PDM service driver object registered with `pdm_driver_register`.
- Optional PDM instance character device `/dev/pdm/<peripheral><index>` when
  userspace access is needed.
- UAPI header `uapi/pdm/pdm_<peripheral>.h` following
  `docs/pdm_uapi_abi.md`.
- Userspace wrapper `user/pdi/src/pdi_<peripheral>.c`.
- Kbuild object selection for the new PDM service inside `pdm_core.ko`.

Feature selection stays at object/list registration boundaries. Kconfig may
select which objects are linked, but business logic should not branch on
feature macros.

Keep kernel/userspace changes aligned. A peripheral exposed to userspace should
add or update runtime config, PDM peripheral service, UAPI, PDI, and Kconfig
coverage together so the ABI and build configuration remain consistent.

## Runtime Interfaces

- `/dev/pdm_ctl` is the PDM Core-owned management/discovery node.
- `/dev/pdm_ctl` is a synchronous snapshot and lookup ABI; PDM v1 does not
  expose asynchronous userspace device events.
- `/dev/pdm/<peripheral><index>` nodes are the stable per-instance business ABI.
- `/dev/pdm/<peripheral><index>` node permissions are controlled by
  `CONFIG_PDM_INSTANCE_DEVNODE_MODE`, which defaults to `0660`; products should
  use udev, devtmpfs policy, or init scripts to assign the intended group.
- `/sys/class/misc/<device>/` attributes are read-only per-instance sysfs
  inspection data, including runtime `state`, `last_error`, and `error_count`.
- PDM discovery snapshots report the same runtime `state`, `last_error`, and
  `error_count` values for management clients.
- Runtime operation failures mark the instance `ERROR`; later successful
  runtime operations recover it to `BOUND` while keeping `last_error` and
  `error_count` as historical diagnostics.
- Caller-side ABI errors such as malformed arguments or unsupported ioctl
  commands are returned to the caller without marking runtime
  health.
- `/proc/pdm/*` nodes are read-only PDM service status snapshots.
- `/sys/kernel/debug/pdm/*` nodes are debug-only command entry points and must
  not be treated as stable product ABI.

## Dependency Rules

- Core modules do not depend on product/application code.
- Dependencies point downward through the layer stack.
- Product-specific behavior belongs outside shared framework module directories.
- Kernel hardware configuration is selected through PDM runtime config backends,
  consumed by PDM peripheral config drivers through configured-device nodes,
  then mapped into PDM Core device configs.
- PDM HW APIs should call PDM SoC Adapter APIs for SoC-backed hardware
  capabilities.
- Kernel-version conditionals belong in `kernel/pdm-core/compat/` or
  `kernel/include/pdm/compat/pdm_compat_*` helper headers.
- Userspace code must use PDI/UAPI rather than including kernel-internal PDM HW,
  PDM_CONFIG, PDM Core, or PDM peripheral headers.
