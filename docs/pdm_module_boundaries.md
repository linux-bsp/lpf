# PDM Module Boundaries

## Dependency Direction

Dependencies should point downward through the PDM stack:

```text
Product → ACONFIG/PDI → UAPI → Peripheral Service → Core
        → PDM HW → SoC Adapter → Kernel Compat → Linux Kernel
```

Cross-layer shortcuts should be treated as architecture debt unless they are
explicitly documented as a transitional compatibility path.

## Allowed Dependencies

### PDI

- May include UAPI headers from `uapi/pdm/`.
- May use libc/POSIX and userspace OSAL helpers.
- Must not include kernel-internal PDM headers.

### UAPI

- May use Linux UAPI scalar types such as `__u32`.
- Must contain only fixed-layout structures, ABI constants, ioctl constants, and
  enum values that are part of the ABI.
- Must not expose SDK context types, helper functions, default-open paths, or
  kernel-private framework structures.

### PDM Peripheral Services

- May use PDM Core device/driver APIs.
- May use PDM HW APIs.
- May use PDM runtime config typed entries as configuration input.
- May use shared PDM chrdev, sysfs, procfs, and debugfs helpers.
- May own service-specific transport backends under the peripheral directory
  when the backend is not shared by multiple peripheral families.
- Must not call SoC adapter, compat, vendor BSP, or Linux hardware subsystem APIs
  directly.

### PDM Core

- May own PDM device/driver lifecycle, discovery, events, and shared runtime node
  helpers.
- Acts as the PDM-owned device model / pseudo bus. Matching is by
  `pdm_device_type_t`; `pdm_device_register()` binds a configured device to the
  already registered driver with the same type and calls `probe()`.
- May own reusable infrastructure wrappers for chrdev, sysfs, procfs, and
  debugfs.
- Owns its global lifecycle in `pdm_core.ko` module init/exit; public Core APIs
  must require initialized state instead of calling Core init implicitly.
- Owns shared PDM character-device node policy. Instance nodes under
  `/dev/pdm/` use `CONFIG_PDM_INSTANCE_DEVNODE_MODE`, while product-specific
  ownership such as group assignment belongs in udev/devtmpfs/init policy.
- Must not depend on concrete runtime config backends or product tables.

### PDM HW

- May call PDM SoC Adapter APIs.
- May hold hardware capability state that is meaningful above a concrete SoC
  backend.
- Must not call Linux subsystem or vendor BSP APIs directly.

### PDM SoC Adapter

- May call PDM Kernel Compat APIs or vendor BSP APIs.
- Must keep SoC-specific code under `kernel/pdm-core/soc/`.
- Must not contain peripheral business behavior.

### PDM Kernel Compat

- May include Linux kernel headers and wrap kernel API differences.
- Should be the default location for `LINUX_VERSION_CODE` and equivalent
  version/feature handling. Use `pdm_compat_features.h` feature gates rather
  than raw version checks outside the compat layer.
- Must not contain peripheral business behavior.

### Runtime Config

- Owns backend selection, validation, and configured-device node generation.
- Backends may read static tables, Device Tree, module parameters, or future
  board-profile sources.
- Runtime config drivers must consume configured-device nodes, not concrete
  backend symbols. PDM Core consumes only `pdm_device_config_t`.

## Forbidden Patterns

- Peripheral service code calling `pdm_soc_*`, `pdm_compat_*`, `gpio_*`,
  `pwm_*`, `i2c_*`, `spi_*`, CAN socket helpers, TTY helpers, or vendor BSP APIs.
- Userspace code including `kernel/include/pdm/` headers other than installed
  UAPI copies.
- Product-specific business logic under shared framework service directories.
- New per-peripheral kernel modules when the integrated runtime boundary can
  host the reusable service.
- New global fixed-size registries when PDM Core device lifecycle or dynamic
  service-owned registries are sufficient.

## Transitional Allowances

- Instance character device arrays may retain configured maxima while UAPI info
  still reports `max_devices`.
- ABI or protocol constants may remain fixed-size when changing them would break
  userspace or wire compatibility.
- Procfs/debugfs/sysfs helpers may live in PDM Core while compat wrappers for
  their API-version differences are still being defined.
