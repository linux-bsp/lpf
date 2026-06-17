# H200-100P-AM625 ACONFIG

This directory contains the runtime ACONFIG table for the H200-100P-AM625 PMC product.

Current source:
- `aconfig_h200_100p_am625.c`: product table consumed by `pmc_runtime` and registered through `ACONFIG_register_table()`.

Rules:
- Do not add standalone example `.c` files here. Product CMake treats `aconfig_${CONFIG_PROJECT_NAME}.c` as the authoritative runtime table.
- Keep product-specific function IDs and concrete TC/TM mappings in the PMC product layer, not in `core/aconfig`.
- Hardware device lookup remains index-based through `PCONFIG`; application code should access TC/TM mappings through `PMC_ACONFIG_*` APIs.
