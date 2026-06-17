# PCONFIG

PCONFIG is the platform hardware-configuration registry. It stores platform-level hardware tables and exposes typed index-based accessors for enabled core device families.

## Current Responsibility

- Register platform configurations (`pconfig_platform_config_t`).
- Track the current board configuration.
- Provide typed accessors for MCU entries.
- Keep hardware configuration data separate from PDL and application logic.

## Public API

```c
int32_t PCONFIG_init(void);
void PCONFIG_cleanup(void);
int32_t PCONFIG_register(const pconfig_platform_config_t *config);
const pconfig_platform_config_t *PCONFIG_GetBoard(void);
int32_t PCONFIG_SetBoard(const pconfig_platform_config_t *config);
const pconfig_platform_config_t *PCONFIG_Find(const char *platform,
                                             const char *product,
                                             const char *version);
int32_t PCONFIG_list(const pconfig_platform_config_t **configs, uint32_t *count);
int32_t PCONFIG_validate(const pconfig_platform_config_t *config);
void PCONFIG_print(const pconfig_platform_config_t *config);
```

## Typed Accessors

The current header provides an inline index-based MCU accessor:

```c
PCONFIG_HW_GetMCU(platform, index);
```

## Layering Rules

- `core/pconfig` defines data structures and registry behavior only.
- Product or board integration code owns concrete platform tables.
- PDL consumes `PCONFIG_GetBoard()` and typed accessors; it should not know concrete product table symbols.
- Applications should initialize PCONFIG through their platform runtime/bootstrap code.
