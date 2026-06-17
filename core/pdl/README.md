# PDL

PDL is the Peripheral Driver Layer. It contains high-level peripheral drivers built on top of PCONFIG, PRL, HAL, and OSAL.

## Current Scope

The framework currently keeps one concrete peripheral type:

- `pdl_mcu`: MCU peripheral driver with CAN and serial transports.

The layer is intentionally still organized by peripheral type so future peripherals can be added under `core/pdl/src/<peripheral>/` with matching public headers and Kconfig entries.

## Configuration

```text
CONFIG_PDL=y
CONFIG_PDL_MCU_SUPPORT=y
CONFIG_PDL_MCU_CAN_SUPPORT=y
CONFIG_PDL_MCU_UART_SUPPORT=y
```

## Layout

```text
core/pdl/
├── Config.in
├── CMakeLists.txt
├── include/pdl/
│   ├── pdl.h
│   └── pdl_mcu.h
└── src/pdl_mcu/
    ├── Config.in
    ├── pdl_mcu.c
    ├── pdl_mcu_can.c
    ├── pdl_mcu_serial.c
    └── pdl_mcu_internal.h
```

## Layering

`PDL_MCU_init()` resolves MCU indexes through `PCONFIG_GetBoard()` and `PCONFIG_HW_GetMCU()`. PDL consumes typed platform configuration and should not know concrete product table symbols.
