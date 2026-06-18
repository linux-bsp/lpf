# PRL

PRL is the Protocol Layer for encoded device communication. It provides shared packet framing, CRC handling, sequence management, and device message encode/decode helpers.

## Current Scope

The framework currently keeps one concrete device protocol:

- `PRL_DEV_TYPE_MCU`
- `prl_mcu.h`
- `CONFIG_PRL_MCU`

The protocol layer remains organized so additional device protocols can be added later through new device type IDs, headers, and Kconfig options.

## Configuration

```text
CONFIG_PRL=y
CONFIG_PRL_MCU=y
CONFIG_PRL_CRC_CHECK=y
CONFIG_PRL_SEQUENCE_CHECK=y
```

## Layout

```text
core/kernel/prl/
├── Config.in
├── CMakeLists.txt
└── src/
    ├── prl_api.c
    ├── prl_common.c
    └── prl_device.c

core/kernel/include/prl/
├── prl.h
├── prl_common.h
├── prl_device.h
└── prl_mcu.h
```
