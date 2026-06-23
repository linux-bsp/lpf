# PDM Module Boundaries

## Dependency Direction

Dependencies should point downward through the current PDM stack:

```text
Application -> PDI -> UAPI/ioctl -> PDM peripheral driver
            -> PDM bus/core helpers -> Linux subsystem or controller driver
```

Cross-layer shortcuts are architecture debt unless they are documented as a
temporary compatibility path.

## Driver Architecture

The PDM kernel stack uses four explicit layers:

```text
Linux native bus or vendor,pdm-bus DT node
    -> PDM device
    -> PDM peripheral driver
    -> PDM backend
    -> Linux subsystem resource or native transport driver
```

- The PDM bus owns `struct bus_type`, PDM device registration, driver matching,
  probe/remove dispatch, and common device state.
- A PDM device is the stable service instance exposed under `/sys/bus/pdm` and,
  after binding, under `/dev/pdm/`. It may be created by the `vendor,pdm-bus`
  controller or by a backend-owned native bus driver such as serdev, I2C, or SPI.
- A PDM peripheral driver owns service semantics: UAPI/ioctl handling, client
  node policy, generic device state, and selection of a backend through
  `pdm_backend_find()`. It must not directly register backend-owned Linux
  subsystem drivers.
- A PDM backend owns implementation details for one compatible class. Backends
  provide service operation tables and, when the hardware is enumerated by a
  native Linux bus, own registration and unregistration of that native driver.

Backends that do not need native enumeration, such as CAN socket transport or
LED GPIO/PWM control, should register backend ops without an init/exit hook.
Backends that create PDM devices from native Linux children, such as serdev,
I2C, and SPI MCU transports, should put the native driver lifecycle in the
backend init/exit hooks.

## Allowed Dependencies

### PDI

- May include public PDI headers and UAPI headers from `uapi/pdm/`.
- May use libc/POSIX and userspace OSAL helpers.
- Must not include `kernel/include/pdm/` headers.

### UAPI

- May use Linux fixed-width UAPI scalar types such as `__u32` and `__s32`.
- Must contain only fixed-layout structures, ABI constants, ioctl constants, and
  enum values that are part of the stable ABI.
- Must not expose kernel-private PDM structs, backend ops, OSAL objects, or PDI
  context types.

### PDM Core

- Owns Linux `bus_type` registration, PDM device lifecycle, driver matching,
  `/dev/pdm_ctl`, shared client node policy, sysfs, procfs, debugfs, and module
  init/exit ordering.
- May own generic registries such as the section-based driver and backend entry
  walkers.
- Must not contain product-specific business logic.
- Must not require every peripheral or backend to be called manually from core
  init code.

### PDM Peripheral Drivers

- May use PDM bus, device, client-node, procfs, sysfs, debugfs, and backend
  registry helpers.
- May own service-specific operation tables and match logic.
- May call `pdm_backend_find()` to select a backend for a matching compatible
  string.
- Should keep transport or control details in backend files such as
  `pdm_mcu_uart.c`, `pdm_mcu_i2c.c`, `pdm_mcu_spi.c`, `pdm_mcu_can.c`,
  `pdm_led_gpio.c`, and `pdm_led_pwm.c`.

### PDM Backends

- May call the Linux subsystem API for the backend they own, for example serdev,
  I2C, SPI, CAN, GPIO, or PWM.
- Must register through `pdm_backend_register()` instead of adding direct core
  init calls.
- Must keep optional feature guards local and compile to a no-op when the
  required Linux subsystem is unavailable.

### Kernel Compatibility

- Linux version and feature checks belong in `kernel/include/pdm/compat/` or
  small local compatibility wrappers.
- Shared driver, backend, and userspace-facing logic should use feature helpers
  rather than raw `LINUX_VERSION_CODE` checks.

## Forbidden Patterns

- Userspace code including kernel-internal PDM headers.
- Product-specific business logic under shared PDM driver directories.
- New fixed global registries when the section-based driver or backend registry
  is sufficient.
- PDM Core manually calling each MCU, LED, or future peripheral backend init.
- UAPI structs carrying kernel pointers, private structs, or backend-specific
  implementation details.
