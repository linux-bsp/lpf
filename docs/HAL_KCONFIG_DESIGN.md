# HAL Kconfig 设计原则

## 设计理念

HAL 的 Kconfig 配置应该关注**功能特性**，而不是具体的硬件配置。

### 正确的做法 ✅

**HAL Kconfig**：关注驱动功能和特性
```kconfig
config HAL_CAN
    bool "CAN bus driver"
    help
      Enable CAN bus driver support.

config HAL_CAN_FILTER
    bool "CAN filter support"
    depends on HAL_CAN
    help
      Enable CAN message filtering capability.

config HAL_UART_FLOW_CONTROL
    bool "UART hardware flow control"
    depends on HAL_UART
    help
      Enable RTS/CTS hardware flow control.
```

**PCL 配置**：具体的硬件参数
```c
// core/pcl/platform/ti/am625/ccm_h200/hw_config.c
static const pcl_can_config_t can_configs[] = {
    {
        .name = "can0",
        .device = "/dev/can0",
        .bitrate = 500000,
    },
};

static const pcl_uart_config_t uart_configs[] = {
    {
        .name = "uart0",
        .device = "/dev/ttyS0",
        .baudrate = 115200,
        .databits = 8,
        .parity = PCL_UART_PARITY_NONE,
        .stopbits = 1,
    },
};
```

### 错误的做法 ❌

**不要在 HAL Kconfig 中配置硬件参数**：
```kconfig
# ❌ 错误：这些应该在 PCL 中配置
config HAL_CAN_DEVICE
    string "Default CAN device"
    default "can0"

config HAL_UART_BAUDRATE
    int "Default UART baudrate"
    default 115200

config HAL_WATCHDOG_TIMEOUT
    int "Default watchdog timeout"
    default 30
```

## 职责划分

### HAL Kconfig 的职责

1. **驱动开关**：是否启用某个驱动
   ```kconfig
   config HAL_CAN
       bool "CAN bus driver"
   ```

2. **功能特性**：驱动支持的功能
   ```kconfig
   config HAL_CAN_FILTER
       bool "CAN filter support"
   
   config HAL_CAN_ERROR_HANDLING
       bool "CAN error handling"
   ```

3. **高级特性**：可选的高级功能
   ```kconfig
   config HAL_SPI_DMA
       bool "SPI DMA support"
   
   config HAL_GPIO_INTERRUPT
       bool "GPIO interrupt support"
   ```

4. **调试选项**：开发和调试功能
   ```kconfig
   config HAL_DEBUG
       bool "HAL debug logging"
   
   config HAL_STATISTICS
       bool "HAL statistics collection"
   ```

### PCL 的职责

1. **硬件参数**：设备路径、波特率、超时等
2. **硬件拓扑**：设备数量、连接关系
3. **平台差异**：不同平台的硬件配置

## 示例对比

### CAN 驱动配置

**HAL Kconfig**（功能特性）：
```kconfig
config HAL_CAN
    bool "CAN bus driver"
    default y

config HAL_CAN_FILTER
    bool "CAN filter support"
    depends on HAL_CAN
    default y
    help
      Enable CAN message filtering by ID.

config HAL_CAN_ERROR_HANDLING
    bool "CAN error handling"
    depends on HAL_CAN
    default y
    help
      Enable CAN bus error detection.

config HAL_CAN_LOOPBACK
    bool "CAN loopback mode"
    depends on HAL_CAN
    default n
    help
      Enable loopback mode for testing.
```

**PCL 配置**（硬件参数）：
```c
// TI AM625 平台配置
static const pcl_can_config_t can_configs[] = {
    {
        .name = "can0",
        .device = "/dev/can0",
        .bitrate = 500000,
        .sample_point = 875,  // 87.5%
    },
    {
        .name = "can1",
        .device = "/dev/can1",
        .bitrate = 250000,
        .sample_point = 875,
    },
};
```

### UART 驱动配置

**HAL Kconfig**（功能特性）：
```kconfig
config HAL_UART
    bool "UART driver"
    default y

config HAL_UART_FLOW_CONTROL
    bool "UART hardware flow control"
    depends on HAL_UART
    default n
    help
      Enable RTS/CTS flow control.

config HAL_UART_RS485
    bool "RS485 mode support"
    depends on HAL_UART
    default n
    help
      Enable RS485 half-duplex mode.
```

**PCL 配置**（硬件参数）：
```c
// TI AM625 平台配置
static const pcl_uart_config_t uart_configs[] = {
    {
        .name = "uart0",
        .device = "/dev/ttyS0",
        .baudrate = 115200,
        .databits = 8,
        .parity = PCL_UART_PARITY_NONE,
        .stopbits = 1,
        .flow_control = false,
    },
    {
        .name = "uart1",
        .device = "/dev/ttyS1",
        .baudrate = 9600,
        .databits = 8,
        .parity = PCL_UART_PARITY_EVEN,
        .stopbits = 1,
        .flow_control = true,
    },
};
```

### Watchdog 驱动配置

**HAL Kconfig**（功能特性）：
```kconfig
config HAL_WATCHDOG
    bool "Watchdog driver"
    default y

config HAL_WATCHDOG_PRETIMEOUT
    bool "Watchdog pretimeout support"
    depends on HAL_WATCHDOG
    default n
    help
      Enable pretimeout notification.
```

**PCL 配置**（硬件参数）：
```c
// TI AM625 平台配置
static const pcl_watchdog_config_t watchdog_config = {
    .name = "watchdog0",
    .device = "/dev/watchdog",
    .timeout = 30,          // 30 秒
    .pretimeout = 5,        // 提前 5 秒通知
};
```

## 优势

### 1. 关注点分离

- **HAL**：关注"能做什么"（功能）
- **PCL**：关注"怎么做"（配置）

### 2. 平台无关性

HAL 代码完全平台无关：
```c
#include <generated/autoconf.h>

int32 HAL_CANInit(hal_can_handle_t *handle, const char *name)
{
    // 从 PCL 获取硬件配置
    const pcl_can_config_t *config = PCL_GetCANConfig(name);
    
#ifdef CONFIG_HAL_CAN_FILTER
    // 如果启用了过滤功能
    hal_can_setup_filter(handle, config);
#endif

#ifdef CONFIG_HAL_CAN_ERROR_HANDLING
    // 如果启用了错误处理
    hal_can_enable_error_handling(handle);
#endif

    return OS_SUCCESS;
}
```

### 3. 灵活配置

不同产品可以：
- 使用相同的 HAL Kconfig（功能特性）
- 使用不同的 PCL 配置（硬件参数）

例如：
```
产品 A (TI AM625)：
- HAL_CAN=y, HAL_CAN_FILTER=y
- PCL: can0=/dev/can0, bitrate=500000

产品 B (NXP i.MX8)：
- HAL_CAN=y, HAL_CAN_FILTER=y
- PCL: can0=/dev/flexcan0, bitrate=1000000
```

### 4. 易于移植

移植到新平台只需：
1. HAL Kconfig 不变（功能特性相同）
2. 添加新的 PCL 配置（硬件参数不同）

## 总结

| 配置类型 | 位置 | 示例 |
|---------|------|------|
| 驱动开关 | HAL Kconfig | `CONFIG_HAL_CAN` |
| 功能特性 | HAL Kconfig | `CONFIG_HAL_CAN_FILTER` |
| 硬件参数 | PCL | `device="/dev/can0"` |
| 硬件拓扑 | PCL | `can_count=2` |
| 平台差异 | PCL | `platform/ti/am625/` |

**核心原则**：
- HAL Kconfig = 功能开关（What）
- PCL 配置 = 硬件参数（How）
