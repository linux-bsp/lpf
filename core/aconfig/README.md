# ACONFIG

ACONFIG is the generic application-configuration registry. It owns no product-specific TC/TM schema. Product layers register an opaque function map through `ACONFIG_register_table()`, then provide product-specific query APIs on top of that map.

## Current Responsibility

- Store one active `aconfig_config_table_t`.
- Provide thread-safe registration, unregistration, and table lookup.
- Keep core independent from PMC TC/TM enums and hardware device semantics.

Product-specific interpretation belongs in product modules such as `products/pmc/aconfig`.

## Public API

```c
int32_t ACONFIG_init(void);
void ACONFIG_cleanup(void);
int32_t ACONFIG_register_table(const aconfig_config_table_t *table);
int32_t ACONFIG_unregister_table(void);
const aconfig_config_table_t *ACONFIG_GetTable(void);
```

## Data Model

```c
typedef struct aconfig_function_map aconfig_function_map_t;

typedef struct {
    const char *name;
    aconfig_function_map_t *function_map;
    void *user_data;
} aconfig_config_table_t;
```

`function_map` is intentionally opaque. For PMC, it points to `pmc_aconfig_function_map_t`, and callers should use `PMC_ACONFIG_GetTcConfig()` / `PMC_ACONFIG_GetTmConfig()` instead of casting in application code.

## Initialization Flow

For PMC products, applications should not initialize ACONFIG directly. They should call:

```c
ret = PMC_Runtime_Init();
```

`pmc_runtime` initializes ACONFIG and registers the selected product table generated from `CONFIG_PROJECT_NAME`.

## Layering Rules

- `core/aconfig` must not include product headers.
- `core/aconfig` must not define TC/TM function IDs.
- Product-specific lookup, validation, and function semantics belong in the product layer.
- Platform-specific tables belong under `products/<product>/configs/projects/<project>/aconfig/`.
