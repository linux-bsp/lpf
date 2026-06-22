# Kernel And Userspace Split

PDM is Linux-only in its current direction. It separates kernel modules from
userspace API libraries so hardware access, ioctl ABI definitions, and
application-facing APIs remain in the right layer.

## Target Layout

```text
kernel/
  Makefile
  include/
    osal/          # kernel-side cross-module OSAL headers
    lpf/           # kernel-side cross-module PDM headers and layer subdirs
      compat/      # Linux kernel compatibility headers
      config/      # runtime config type headers
      core/        # PDM Core model and shared node headers
      hw/          # PDM HW API headers
      peripheral/  # PDM runtime and service headers
      protocol/    # PDM protocol headers
      soc/         # PDM SoC adapter headers
  osal/
    src/           # builds osal.ko
  pdm-core/
    core/          # PDM device model and shared node infrastructure
    protocol/      # PDM protocol helpers linked into pdm_core.ko
    compat/        # Linux kernel compatibility wrappers
    soc/           # SoC adapter backends
    config/        # runtime configuration backends and parsers
    hw/            # capability-grouped PDM HW implementations
    include/       # runtime-private helper headers
    runtime/       # integrated runtime entry and orchestration
    peripheral/    # framework-owned runtime services and service backends
  pdm-configs/
    configs/       # selected static board descriptions for pdm_configs.ko

user/
  osal/            # userspace OSAL library
  aconfig/         # userspace application configuration
  pdi/             # userspace API library for PDM

uapi/
  lpf/             # ioctl ABI shared by PDM kernel nodes and PDI
```

## Responsibilities

- `kernel/osal` wraps Linux kernel APIs and builds `osal.ko`.
- `kernel/pdm-core/core` owns the PDM device model, control/discovery node, and
  shared chrdev/sysfs/debugfs helpers. It also calls the integrated runtime
  entry linked into `pdm_core.ko`.
- `kernel/pdm-core/peripheral` owns framework runtime service
  implementations; current service paths are linked into `pdm_core.ko`.
- `kernel/pdm-core/protocol` provides kernel-side PDM protocol helpers through
  `pdm_core.ko` for services that need framed communication.
- `kernel/pdm-core/hw` provides capability-grouped PDM-owned hardware access
  implementations used by PDM peripheral services. The objects are linked into
  `pdm_core.ko`; runtime-private helper headers live under
  `kernel/pdm-core/include`.
- `kernel/pdm-core/config` provides PDM runtime config backend/parser objects
  linked into `pdm_core.ko`.
- `kernel/pdm-configs` builds `pdm_configs.ko`; its root holds module glue and
  section sentinels, while `kernel/pdm-configs/configs` holds concrete static
  board descriptions.
- `user/pdi` provides the application-facing C API and wraps open/ioctl.
- `uapi/lpf` is the stable ABI shared by PDM kernel nodes and `user/pdi`.

## Boundary Rules

- Kernel code may include `kernel/include/<module>/` and generated headers.
- Userspace code may include `user/<module>/include/` and `uapi/` headers.
- Userspace code must not include kernel-internal PDM HW, PDM runtime config,
  PDM Core, or PDM peripheral headers.
- UAPI headers must not depend on kernel-only types or private framework
  structures.
- PDI should marshal data and call ioctl; it should not duplicate PDM
  peripheral service or PDM HW behavior in userspace.
- Product code should call PDI and ACONFIG rather than reaching into kernel
  framework internals.

## Device Access Flow

```text
Application
    ↓
PDI userspace API
    ↓
/dev/pdm/<peripheral><index> ioctl
    ↓
PDM peripheral service
    ↓
PDM Core device model + PDM HW APIs
    ↓
Linux kernel subsystem / hardware
```

Kernel-side device creation is driven by runtime config before userspace opens
an instance node:

```text
static config / Device Tree
    ↓
configured-device node table
    ↓
peripheral config driver
    ↓
pdm_device_register()
    ↓
PDM Core type match + service probe
```

Each userspace-visible peripheral should have a matching UAPI header and PDI
wrapper. For example, MCU uses `/dev/pdm/mcu0`, `uapi/pdm/pdm_mcu.h`, and
`user/pdi/src/pdi_mcu.c`.

The previous userspace test product was removed. New tests live under
`tests/` with explicit CMake/CTest integration; UAPI layout checks and PDI
userspace smoke tests currently run through `make tests`. PDI operation-path
tests use an internal syscall mock boundary so ioctl marshaling can be
validated without requiring live PDM device nodes.

## Include Rules

- Kernel cross-module headers live under `kernel/include/<module>/`.
- PDM layer-specific kernel headers live under `kernel/include/pdm/<layer>/`.
- UAPI headers live under `uapi/pdm/` and must be valid for both kernel and
  userspace builds.
- Userspace code must not include non-UAPI kernel headers.
- PDM UAPI rules are documented in `docs/pdm_uapi_abi.md`.
