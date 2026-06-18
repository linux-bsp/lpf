# Repository Guidelines

## Project Structure & Module Organization
`core/` contains reusable middleware layers split between `core/kernel/`, `core/user/`, and `core/uapi/`. `core/kernel/` holds kernel-side modules such as `osal`, `hal`, `pconfig`, `prl`, and `pdm`; `core/user/` holds userspace libraries such as `osal`, `aconfig`, `pdi`, and `test_framework`. `products/ctest/` holds the core test product and most runnable test cases. Configuration presets live under `configs/ctest/`, build support scripts are under `scripts/`, and generated artifacts are written to `_build/`.

## Build, Test, and Development Commands
- `make list` - show available configuration and build targets.
- `make ctest_x86_minimal_defconfig` - load a baseline test configuration.
- `make menuconfig` - open the interactive Kconfig editor.
- `make all` - configure and build the selected target.
- `./_build/bin/es-middleware-test --all` - run the compiled test binary.
- `make clean` - remove build outputs.

## Coding Style & Naming Conventions
Follow the existing C style in the tree: 4-space indentation in newer code, tabs in legacy Kconfig/C sources where already used, braces on the next line for functions, and short `static` helpers for file-local logic. Keep names uppercase for public APIs and macros (`OSAL_*`, `PDI_*`, `PDM_*`), lowercase snake_case for internal files and helpers, and match the module prefix already in use (`osal_`, `hal_`, `pdi_`, `pdm_`).

## Testing Guidelines
Tests are C sources registered through `core/user/test_framework/` and grouped by scope under `products/ctest/` (`unit`, `system`, `stress`, `performance`). Name test files `test_*.c` and keep module-specific configs alongside them in `Config.in`. Prefer running the smallest defconfig that exercises your change, then verify with `./_build/bin/es-middleware-test --all`.

## Commit & Pull Request Guidelines
Recent history uses concise prefixes such as `refactor:` and `fix:`, often with a module scope like `refactor(pdi): ...` or `refactor(pdm): ...`. Follow that pattern: short, imperative, and specific. Pull requests should explain the functional change, list the defconfig or test binary used for verification, and mention any configuration impact or skipped coverage.

## Configuration Notes
Do not edit generated files under `_build/`. When adding a module or test, update the relevant `Config.in` and CMake files together so Kconfig and CMake stay aligned.
