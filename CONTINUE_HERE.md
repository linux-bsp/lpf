# PDM 重构 - 继续点指南

**当前状态**: Phase 5 部分完成 (35%)  
**最后提交**: 9de1a5c  
**下一步**: 完成 MCU/LED 驱动的 probe 实现

---

## 🎯 立即继续的任务

### Task 1: 完成 MCU 驱动 probe 实现

**文件**: `kernel/pdm-core/peripheral/mcu/pdm_mcu_service.c`  
**位置**: 第 753 行附近，`pdm_mcu_probe_new()` 函数

**需要做的事情**:

```c
static int pdm_mcu_probe_new(struct pdm_device *pdm_dev)
{
    struct device_node *np = pdm_dev->dev.of_node;
    const char *uart_device = NULL;
    const char *can_device = NULL;
    u32 baudrate = 0;
    int ret;
    
    // 1. 从 Device Tree 读取配置
    if (of_property_read_string(np, "uart-device", &uart_device) == 0) {
        // UART transport
        of_property_read_u32(np, "baudrate", &baudrate);
        // TODO: 初始化 UART transport
    } else if (of_property_read_string(np, "can-device", &can_device) == 0) {
        // CAN transport
        u32 bitrate = 0;
        of_property_read_u32(np, "bitrate", &bitrate);
        // TODO: 初始化 CAN transport
    }
    
    // 2. 调用现有的初始化逻辑
    // 可以复用 pdm_mcu_init_from_entry() 的代码
    // 或者重构为共享函数
    
    // 3. 创建字符设备 /dev/pdm/mcuX
    // 调用 pdm_mcu_chrdev_register_instance()
    
    // 4. 保存设备数据
    pdm_device_set_drvdata(pdm_dev, ctx);
    
    return 0;
}
```

**参考现有代码**:
- `pdm_mcu_init_from_entry()` (line ~233) - 现有初始化逻辑
- `pdm_mcu_probe()` (找不到，可能在 config_driver 中)

---

### Task 2: 完成 LED 驱动 probe 实现

**文件**: `kernel/pdm-core/peripheral/led/pdm_led_service.c`  
**位置**: 第 460 行附近，`pdm_led_probe_new()` 函数

**需要做的事情**:

```c
static int pdm_led_probe_new(struct pdm_device *pdm_dev)
{
    struct device_node *np = pdm_dev->dev.of_node;
    struct gpio_desc *gpio = NULL;
    struct pwm_device *pwm = NULL;
    int ret;
    
    // 1. 从 Device Tree 读取配置
    // GPIO LED
    gpio = devm_gpiod_get_optional(&pdm_dev->dev, "gpio", GPIOD_OUT_LOW);
    if (gpio) {
        // TODO: 初始化 GPIO LED
    }
    
    // PWM LED
    pwm = devm_pwm_get(&pdm_dev->dev, NULL);
    if (!IS_ERR(pwm)) {
        // TODO: 初始化 PWM LED
    }
    
    // 2. 创建字符设备 /dev/pdm/ledX
    
    // 3. 保存设备数据
    pdm_device_set_drvdata(pdm_dev, ctx);
    
    return 0;
}
```

---

## 📋 详细步骤（按优先级）

### Step 1: 创建测试用 Device Tree (1小时)

创建 `arch/arm/boot/dts/imx6ull-pdm-test.dts`:

```dts
/dts-v1/;
/include/ "imx6ull.dtsi"

/ {
    model = "i.MX6ULL PDM Test Board";
    compatible = "fsl,imx6ull";
    
    pdm_bus: pdm@0 {
        compatible = "vendor,pdm-bus";
        #address-cells = <1>;
        #size-cells = <0>;
        
        mcu0: mcu@0 {
            compatible = "vendor,pdm-mcu-uart";
            reg = <0>;
            uart-device = "/dev/ttyS2";
            baudrate = <115200>;
            status = "okay";
        };
        
        led0: led@0 {
            compatible = "vendor,pdm-led-gpio";
            reg = <0>;
            gpios = <&gpio1 5 GPIO_ACTIVE_HIGH>;
            status = "okay";
        };
    };
};
```

### Step 2: 实现 MCU probe (2-3小时)

**关键点**:
1. 从 DT 读取 transport 配置
2. 调用现有的 transport 初始化代码
3. 创建字符设备
4. 注册 proc/debugfs 接口

**代码位置**:
- Transport 初始化: `pdm_mcu_transport.c`
- 字符设备: `pdm_mcu_chrdev.c`

### Step 3: 实现 LED probe (1-2小时)

**关键点**:
1. 从 DT 读取 GPIO 或 PWM 配置
2. 初始化硬件
3. 创建字符设备

**代码位置**:
- GPIO 初始化: `pdm_led_gpio.c`
- PWM 初始化: `pdm_led_pwm.c`

### Step 4: 编译测试 (1小时)

```bash
# 启用新总线
# 编辑 .config: CONFIG_PDM_NEW_BUS=y

make menuconfig
# 选择 PDM Core -> [*] Use new Linux bus_type implementation

ARCH=x86_64 make modules
# 应该编译通过

# 如果有硬件，测试：
insmod osal.ko
insmod pdm_core.ko
# 不再需要 pdm_configs.ko！

# 检查总线
ls /sys/bus/pdm/
ls /sys/bus/pdm/devices/
ls /sys/bus/pdm/drivers/
```

### Step 5: 完成 Phase 6-8 (14小时)

参考 `ARCH_REFACTOR_PLAN.md` 中的详细说明。

---

## 🔧 有用的命令

### 查看当前状态
```bash
git status
git log --oneline -5
cat FINAL_REPORT.md
```

### 编译测试
```bash
make ubuntu_x86_modules_defconfig
ARCH=x86_64 make modules
```

### 查找代码位置
```bash
# 找现有的 MCU 初始化代码
grep -n "pdm_mcu_init" kernel/pdm-core/peripheral/mcu/*.c

# 找字符设备注册代码
grep -n "misc_register\|cdev" kernel/pdm-core/peripheral/mcu/*.c
```

---

## 📚 重要参考

### 内部代码
- `pdm_mcu_init_from_entry()` - 现有 MCU 初始化逻辑
- `pdm_mcu_transport_get()` - Transport 选择
- `pdm_mcu_chrdev.c` - 字符设备实现
- `pdm_led_gpio.c` / `pdm_led_pwm.c` - LED 硬件操作

### Linux 内核参考
- `of_property_read_string()` - 读取 DT 字符串
- `of_property_read_u32()` - 读取 DT 整数
- `devm_*` 系列函数 - 设备资源管理
- `pdm_device_set_drvdata()` - 保存私有数据

### PDM 参考项目
- `/home/wanguo/Github/pdm/src/` - 参考实现

---

## ⚠️ 注意事项

1. **保持向后兼容**: 不要破坏旧代码（`CONFIG_PDM_NEW_BUS=n` 时）

2. **条件编译**: 所有新代码都在 `#ifdef CONFIG_PDM_NEW_BUS` 中

3. **测试**: 每次修改后编译验证

4. **提交频率**: 建议每完成一个小功能就提交

5. **文档更新**: 完成后更新 `FINAL_REPORT.md`

---

## 🎯 完成标准

Phase 5 完成的标志：

- [ ] MCU probe 函数完整实现
- [ ] LED probe 函数完整实现
- [ ] 从 DT 读取配置正常工作
- [ ] 字符设备创建成功
- [ ] 编译通过（`CONFIG_PDM_NEW_BUS=y`）
- [ ] 可选：在真实硬件上测试

---

## 📞 需要帮助？

如果遇到问题，参考：

1. **ARCH_REFACTOR_PLAN.md** - 详细架构计划
2. **FINAL_REPORT.md** - 进度总结
3. **Git log** - 查看之前的提交
4. **现有代码** - 参考旧的实现

---

**预计剩余时间**: 18-25 小时  
**当前完成**: 35%  
**下一个里程碑**: Phase 5 完成 (→ 50%)

**加油！基础架构已经非常牢固了！** 🚀
