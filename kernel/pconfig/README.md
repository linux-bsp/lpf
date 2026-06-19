# PCONFIG

PCONFIG is the platform hardware-configuration aggregation layer. It selects a
configuration backend, validates the active platform, and exposes one typed
device list to PDM and other LPF kernel services.

## Current Responsibility

- Select the active configuration backend.
- Keep the built-in static table as the first backend implementation.
- Validate platform identity and per-device configuration.
- Build a normalized enabled-device list for MCU and LED entries.
- Keep hardware configuration data separate from PDM and application logic.

## Public API

```c
const pconfig_platform_config_t *pconfig_get_board(void);
const pconfig_device_config_t *pconfig_get(void);
const pconfig_platform_config_t *pconfig_find(const char *product,
                                             const char *project,
                                             const char *version);
int32_t pconfig_list(const pconfig_platform_config_t **configs, uint32_t *count);
int32_t pconfig_validate(const pconfig_platform_config_t *config);
void pconfig_print(const pconfig_platform_config_t *config);
```

The static backend uses the table symbol from `configs/pconfig_configs.c`:

```c
extern const pconfig_platform_table_t g_pconfig_platform_table;
```

Concrete configs live under:

```text
configs/<product>/<project>/<version>/
```

## Typed Accessors

The current header provides inline index-based peripheral accessors:

```c
pconfig_hw_get_mcu(platform, index);
pconfig_hw_get_led(platform, index);
```

## Layering Rules

- `kernel/pconfig` owns backend selection, validation, and normalized device
  enumeration.
- `kernel/pconfig/configs` owns concrete static platform tables.
- PDM consumes `pconfig_get()` and typed entries; it must not know concrete
  product table symbols or backend implementations.
- New configuration sources should be added as PCONFIG backends. They must
  produce the same `pconfig_platform_config_t` and `pconfig_device_config_t`
  model before PDM sees them.
