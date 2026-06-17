# ACONFIG

ACONFIG is the application configuration layer. It stores application-facing function mappings separately from hardware configuration.

## Layering

- ACONFIG owns application-level mappings and opaque function metadata.
- PCONFIG owns hardware tables.
- Product-specific interpretation belongs in product modules outside the core framework.
- Core ACONFIG APIs must not depend on a concrete product business layer.
