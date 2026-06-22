# PDM Device Tree 配置示例

本文档展示如何使用 Device Tree 配置 PDM 外设驱动（新总线架构）。

## 前提条件

确保编译时启用了新总线支持：
```bash
make menuconfig
# 选择: PDM Core > Enable new Linux bus_type architecture (CONFIG_PDM_NEW_BUS)
```

---

## 基础结构

```dts
/ {
    pdm-bus {
        compatible = "vendor,pdm-bus";
        #address-cells = <1>;
        #size-cells = <0>;

        /* 在这里添加 PDM 外设节点 */
    };
};
```

---

## MCU 外设配置

### UART 接口示例

```dts
pdm-bus {
    compatible = "vendor,pdm-bus";
    #address-cells = <1>;
    #size-cells = <0>;

    mcu@0 {
        compatible = "vendor,pdm-mcu-uart";
        reg = <0>;
        label = "Main MCU";
        
        pdm,name = "main_mcu";
        pdm,interface = "uart";
        pdm,uart-device = "ttyS1";
        pdm,baudrate = <115200>;
        pdm,data-bits = <8>;
        pdm,stop-bits = <1>;
        pdm,cmd-timeout-ms = <1000>;
        pdm,retry-count = <3>;
    };

    mcu@1 {
        compatible = "vendor,pdm-mcu-uart";
        reg = <1>;
        label = "Secondary MCU";
        
        pdm,name = "sec_mcu";
        pdm,interface = "serial";
        pdm,uart-device = "ttyS2";
        pdm,baudrate = <9600>;
        pdm,data-bits = <8>;
        pdm,stop-bits = <1>;
    };
};
```

**字符设备**：`/dev/lpf/mcu0`, `/dev/lpf/mcu1`

### CAN 接口示例

```dts
pdm-bus {
    compatible = "vendor,pdm-bus";
    
    mcu@0 {
        compatible = "vendor,pdm-mcu-can";
        reg = <0>;
        label = "CAN MCU";
        
        pdm,name = "can_mcu";
        pdm,interface = "can";
        pdm,can-device = "can0";
        pdm,bitrate = <500000>;
        pdm,tx-id = <0x100>;
        pdm,rx-id = <0x200>;
        pdm,cmd-timeout-ms = <2000>;
        pdm,retry-count = <5>;
    };
};
```

**字符设备**：`/dev/lpf/mcu0`

---

## LED 外设配置

### GPIO 控制 LED

```dts
pdm-bus {
    compatible = "vendor,pdm-bus";
    
    led@0 {
        compatible = "vendor,pdm-led-gpio";
        reg = <0>;
        label = "Power LED";
        
        pdm,name = "power_led";
        pdm,control = "gpio";
        pdm,gpio-name = "led-power";        /* GPIO consumer name */
        pdm,gpio-active-low;                 /* 可选：低电平有效 */
        pdm,max-brightness = <1>;            /* GPIO: 0 或 1 */
        pdm,default-brightness = <1>;
    };

    led@1 {
        compatible = "vendor,pdm-led-gpio";
        reg = <1>;
        label = "Status LED";
        
        pdm,name = "status_led";
        pdm,control = "gpio";
        pdm,gpio-name = "led-status";
        pdm,max-brightness = <1>;
        pdm,default-brightness = <0>;        /* 默认关闭 */
    };
};
```

**字符设备**：`/dev/lpf/led0`, `/dev/lpf/led1`

**注意**：GPIO 实际引脚需要在板级 DT 或 pinctrl 中定义。

### PWM 控制 LED

```dts
pdm-bus {
    compatible = "vendor,pdm-bus";
    
    led@0 {
        compatible = "vendor,pdm-led-pwm";
        reg = <0>;
        label = "Backlight";
        
        pdm,name = "backlight";
        pdm,control = "pwm";
        pdm,pwm-name = "pwm-backlight";      /* PWM consumer name */
        pdm,pwm-period-ns = <1000000>;       /* 1ms = 1kHz */
        pdm,pwm-inverted;                    /* 可选：反相 */
        pdm,max-brightness = <255>;          /* 0-255 */
        pdm,default-brightness = <128>;
    };

    led@1 {
        compatible = "vendor,pdm-led-pwm";
        reg = <1>;
        label = "RGB LED Red";
        
        pdm,name = "rgb_red";
        pdm,control = "pwm";
        pdm,pwm-name = "pwm-rgb-r";
        pdm,pwm-period-ns = <2000000>;       /* 500Hz */
        pdm,max-brightness = <100>;
        pdm,default-brightness = <0>;
    };
};
```

**字符设备**：`/dev/lpf/led0`, `/dev/lpf/led1`

---

## 混合配置示例

```dts
/ {
    pdm-bus {
        compatible = "vendor,pdm-bus";
        #address-cells = <1>;
        #size-cells = <0>;

        /* MCU 设备 */
        mcu@0 {
            compatible = "vendor,pdm-mcu-uart";
            reg = <0>;
            label = "Main Controller";
            pdm,name = "main_mcu";
            pdm,interface = "uart";
            pdm,uart-device = "ttyS1";
            pdm,baudrate = <115200>;
            pdm,data-bits = <8>;
            pdm,stop-bits = <1>;
            pdm,cmd-timeout-ms = <1000>;
        };

        mcu@1 {
            compatible = "vendor,pdm-mcu-can";
            reg = <1>;
            label = "Motor Controller";
            pdm,name = "motor_mcu";
            pdm,interface = "can";
            pdm,can-device = "can0";
            pdm,bitrate = <500000>;
            pdm,tx-id = <0x100>;
            pdm,rx-id = <0x200>;
        };

        /* LED 设备 */
        led@0 {
            compatible = "vendor,pdm-led-gpio";
            reg = <0>;
            label = "Power LED";
            pdm,name = "power";
            pdm,control = "gpio";
            pdm,gpio-name = "led-power";
            pdm,max-brightness = <1>;
            pdm,default-brightness = <1>;
        };

        led@1 {
            compatible = "vendor,pdm-led-pwm";
            reg = <1>;
            label = "Status LED";
            pdm,name = "status";
            pdm,control = "pwm";
            pdm,pwm-name = "pwm-status";
            pdm,pwm-period-ns = <1000000>;
            pdm,max-brightness = <255>;
            pdm,default-brightness = <0>;
        };
    };
};
```

**生成的设备**：
- `/dev/lpf/mcu0` - Main Controller (UART)
- `/dev/lpf/mcu1` - Motor Controller (CAN)
- `/dev/lpf/led0` - Power LED (GPIO)
- `/dev/lpf/led1` - Status LED (PWM)

---

## 属性参考

### MCU 必需属性

| 属性 | 类型 | 说明 |
|-----|------|------|
| `compatible` | string | `"vendor,pdm-mcu-uart"` 或 `"vendor,pdm-mcu-can"` |
| `reg` | u32 | 设备索引（0-31） |
| `pdm,interface` | string | `"uart"`, `"serial"`, 或 `"can"` |

### MCU UART 属性

| 属性 | 类型 | 默认值 | 说明 |
|-----|------|--------|------|
| `pdm,uart-device` | string | 无（必需） | 串口设备名（如 "ttyS1"） |
| `pdm,baudrate` | u32 | 115200 | 波特率 |
| `pdm,data-bits` | u32 | 8 | 数据位（5-8） |
| `pdm,stop-bits` | u32 | 1 | 停止位（1-2） |

### MCU CAN 属性

| 属性 | 类型 | 默认值 | 说明 |
|-----|------|--------|------|
| `pdm,can-device` | string | 无（必需） | CAN 设备名（如 "can0"） |
| `pdm,bitrate` | u32 | 500000 | 波特率（bps） |
| `pdm,tx-id` | u32 | 0x100 | 发送 CAN ID |
| `pdm,rx-id` | u32 | 0x200 | 接收 CAN ID |

### MCU 通用属性

| 属性 | 类型 | 默认值 | 说明 |
|-----|------|--------|------|
| `pdm,name` | string | dev_name | 设备名称 |
| `pdm,cmd-timeout-ms` | u32 | 1000 | 命令超时（ms） |
| `pdm,retry-count` | u32 | 3 | 重试次数 |
| `label` | string | 可选 | 描述信息 |

---

### LED 必需属性

| 属性 | 类型 | 说明 |
|-----|------|------|
| `compatible` | string | `"vendor,pdm-led-gpio"` 或 `"vendor,pdm-led-pwm"` |
| `reg` | u32 | 设备索引（0-31） |
| `pdm,control` | string | `"gpio"` 或 `"pwm"` |

### LED GPIO 属性

| 属性 | 类型 | 默认值 | 说明 |
|-----|------|--------|------|
| `pdm,gpio-name` | string | 无（必需） | GPIO consumer 名称 |
| `pdm,gpio-active-low` | bool | false | 低电平有效 |

### LED PWM 属性

| 属性 | 类型 | 默认值 | 说明 |
|-----|------|--------|------|
| `pdm,pwm-name` | string | 无（必需） | PWM consumer 名称 |
| `pdm,pwm-period-ns` | u32 | 1000000 | PWM 周期（ns） |
| `pdm,pwm-inverted` | bool | false | 反相输出 |

### LED 通用属性

| 属性 | 类型 | 默认值 | 说明 |
|-----|------|--------|------|
| `pdm,name` | string | dev_name | 设备名称 |
| `pdm,max-brightness` | u32 | 255 | 最大亮度 |
| `pdm,default-brightness` | u32 | 0 | 默认亮度 |
| `label` | string | 可选 | 描述信息 |

---

## 使用 Device Tree Overlay

对于支持 overlay 的平台（如 Raspberry Pi），可以动态加载配置：

```dts
/dts-v1/;
/plugin/;

/ {
    compatible = "vendor,board";

    fragment@0 {
        target-path = "/";
        __overlay__ {
            pdm-bus {
                compatible = "vendor,pdm-bus";
                #address-cells = <1>;
                #size-cells = <0>;

                mcu@0 {
                    compatible = "vendor,pdm-mcu-uart";
                    reg = <0>;
                    pdm,interface = "uart";
                    pdm,uart-device = "ttyS1";
                    pdm,baudrate = <115200>;
                };
            };
        };
    };
};
```

编译并加载：
```bash
dtc -@ -I dts -O dtb -o pdm-overlay.dtbo pdm-overlay.dts
sudo dtoverlay pdm-overlay.dtbo
```

---

## 验证配置

加载驱动后检查：

```bash
# 检查总线
ls /sys/bus/pdm/

# 检查设备
ls /sys/bus/pdm/devices/

# 检查字符设备
ls -l /dev/lpf/

# 查看内核日志
dmesg | grep -i pdm
```

预期输出示例：
```
[   10.123456] pdm_bus: PDM bus registered
[   10.234567] pdm-bus pdm-bus: PDM bus controller probed with 4 devices
[   10.345678] pdm pdm.0: MCU device probed successfully (interface=uart, id=0)
[   10.456789] pdm pdm.1: LED device probed successfully (control=gpio, id=0)
```

---

## 故障排查

### 驱动未加载
- 检查是否启用 `CONFIG_PDM_NEW_BUS`
- 确认内核模块已加载：`lsmod | grep pdm`

### 设备未创建
- 检查 DT compatible 字符串是否正确
- 查看 dmesg 错误信息
- 确认 `reg` 索引不重复

### 字符设备不存在
- 检查 `/sys/bus/pdm/devices/` 是否有设备
- 确认 udev 规则正常工作
- 手动创建：`mknod /dev/lpf/mcu0 c <major> <minor>`

### GPIO/PWM 资源未找到
- 确认板级 DT 中已定义对应的 GPIO/PWM
- 检查 pinctrl 配置
- 验证 consumer 名称匹配

---

## 下一步

Phase 5 完成后的工作：
1. **Phase 6**: 移除静态配置系统（`pdm_configs.ko`）
2. **Phase 7**: 删除旧的伪总线代码
3. **Phase 8**: 在实际硬件上测试

参考文档：
- `CONTINUE_HERE.md` - 重构进度指南
- `ARCH_REFACTOR_PLAN.md` - 完整重构计划
