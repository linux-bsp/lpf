# PCONFIG

PCONFIG is the platform hardware-configuration query layer. It owns read-only platform tables compiled directly into `pconfig.ko` and exposes typed index-based accessors for enabled core device families.

## Current Responsibility

- Read built-in platform configurations (`pconfig_platform_config_t`).
- Track the current board through a compile-time `current_index` in `g_pconfig_platform_table`.
- Provide typed accessors for MCU entries.
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

The module provides the table symbol from `configs/pconfig_configs.c`:

```c
extern const pconfig_platform_table_t g_pconfig_platform_table;
```

Concrete configs live under:

```text
configs/<product>/<project>/<version>/
```

## Typed Accessors

The current header provides an inline index-based MCU accessor:

```c
pconfig_hw_get_mcu(platform, index);
```

## Layering Rules

- `kernel/pconfig` defines data structures and read-only query behavior only.
- `kernel/pconfig/configs` owns concrete platform tables.
- PDM consumes `pconfig_get_board()` and typed accessors; it should not know concrete product table symbols.
- Runtime code must not register, switch, reload, or mutate PCONFIG tables through core APIs.
