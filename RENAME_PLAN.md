# PDM to PDM Renaming Plan

## Phase 1: Global Renaming (PDM → PDM)

This document tracks the systematic renaming of all PDM symbols, files, and identifiers to PDM.

### Renaming Strategy

1. **Bottom-up approach**: Start with leaf identifiers, move to aggregate structures
2. **Verify compilation** after each major category
3. **Commit incrementally** to enable easy rollback
4. **Test functionality** after completion

### Symbol Mapping Table

| Category | Old (PDM) | New (PDM) |
|----------|-----------|-----------|
| **Module Names** | | |
| Kernel module | `pdm_core.ko` | `pdm_core.ko` |
| Config module | `pdm_configs.ko` | `pdm_configs.ko` |
| OSAL module | `osal.ko` | `osal.ko` (keep) |
| **Directory Names** | | |
| Root | `lpf/` | `pdm/` |
| Core | `pdm-core/` | `pdm-core/` |
| Configs | `pdm-configs/` | `pdm-configs/` |
| **Core Types** | | |
| Device | `pdm_device_t` | `pdm_device_t` |
| Driver | `pdm_driver_t` | `pdm_driver_t` |
| Device config | `pdm_device_config_t` | `pdm_device_config_t` |
| Device handle | `pdm_device_handle_t` | `pdm_device_handle_t` |
| Device type enum | `pdm_device_type_t` | `pdm_device_type_t` |
| Capability | `pdm_capability_t` | `pdm_capability_t` |
| **Function Prefixes** | | |
| Core functions | `pdm_*` | `pdm_*` |
| Device functions | `pdm_device_*` | `pdm_device_*` |
| Driver functions | `pdm_driver_*` | `pdm_driver_*` |
| **Macro Prefixes** | | |
| General | `PDM_*` | `PDM_*` |
| Device types | `PDM_DEVICE_TYPE_*` | `PDM_DEVICE_TYPE_*` |
| Capabilities | `PDM_DEVICE_CAP_*` | `PDM_DEVICE_CAP_*` |
| **Device Nodes** | | |
| Control node | `/dev/pdm_ctl` | `/dev/pdm_ctl` |
| Instance nodes | `/dev/pdm/mcu0` | `/dev/pdm/mcu0` |
| Instance nodes | `/dev/pdm/led0` | `/dev/pdm/led0` |
| **Sysfs/Procfs** | | |
| Proc directory | `/proc/pdm/` | `/proc/pdm/` |
| Debugfs directory | `/sys/kernel/debug/pdm/` | `/sys/kernel/debug/pdm/` |
| **Include Paths** | | |
| Public headers | `lpf/core/` | `pdm/core/` |
| Config headers | `lpf/config/` | `pdm/config/` |
| HW headers | `lpf/hw/` | `pdm/hw/` |
| **UAPI** | | |
| Control header | `lpf/pdm_ctl.h` | `pdm/pdm_ctl.h` |
| MCU header | `lpf/pdm_mcu.h` | `pdm/pdm_mcu.h` |
| LED header | `lpf/pdm_led.h` | `pdm/pdm_led.h` |
| **Userspace** | | |
| PDI library | `libpdi.so` | `libpdi.so` (keep name) |
| PDI functions | `pdi_*` | `pdi_*` (keep, already good) |

### Execution Plan

#### Step 1: Rename directories
- `kernel/pdm-core/` → `kernel/pdm-core/`
- `kernel/pdm-configs/` → `kernel/pdm-configs/`
- `kernel/include/pdm/` → `kernel/include/pdm/`
- `uapi/pdm/` → `uapi/pdm/`

#### Step 2: Rename files
- All `pdm_*.c` → `pdm_*.c`
- All `pdm_*.h` → `pdm_*.h`
- Update `#include` paths

#### Step 3: Rename symbols in source
- Type definitions: `pdm_*_t` → `pdm_*_t`
- Function names: `pdm_*` → `pdm_*`
- Macros: `PDM_*` → `PDM_*`
- Comments and strings

#### Step 4: Update build system
- Makefile module names
- Kconfig options: `CONFIG_PDM_*` → `CONFIG_PDM_*`
- Config.in entries

#### Step 5: Update documentation
- README.md
- All docs/*.md files
- CLAUDE.md

#### Step 6: Verify and test
- Compile all configurations
- Test on target hardware
- Verify device nodes created
- Run test suite

### Implementation Notes

1. **OSAL stays as-is**: It's a generic OS abstraction, name still makes sense
2. **PDI stays as-is**: "Peripheral Driver Interface" is generic
3. **Keep git history**: Use `git mv` for file renames
4. **Incremental commits**: One commit per step
5. **Tag before rename**: Create `before-pdm-rename` tag

### Validation Checklist

- [ ] All files renamed
- [ ] All `#include` paths updated
- [ ] All symbols renamed
- [ ] Kconfig compiles
- [ ] Kernel modules compile
- [ ] Userspace libraries compile
- [ ] Device nodes created correctly
- [ ] Test suite passes
- [ ] Documentation updated

---

**Status**: Planning complete, ready to execute  
**Branch**: `refactor/lpf-to-pdm-bus`  
**Date**: 2026-06-22
