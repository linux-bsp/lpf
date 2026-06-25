# PDM 框架架构优化分析

基于对 PDM 框架代码的深入分析，从**代码分层、模块解耦、接口调用深度**等方面，整理出以下架构层面的可优化点：

## 一、代码分层问题

### 1. **Protocol 层职责不清晰**
```
当前 MCU 调用链（10层）：
userspace → ioctl → pdm_mcu_command() → pdm_mcu_claim_device() 
→ pdm_mcu_protocol_command() → pdm_mcu_protocol_cmd_xfer() 
→ pdm_mcu_protocol_xfer() → transport->xfer() → uart_write/read → kernel API
```

**问题**：
- `pdm_mcu_protocol.c` 同时承担了协议编解码、传输层调用、数据校验三重职责
- `pdm_mcu_protocol_command()` → `pdm_mcu_protocol_cmd_xfer()` → `pdm_mcu_protocol_xfer()` 三层包装过度

**建议**：
- 将 protocol 层拆分为独立的中间层，与 transport 层平级
- 简化函数调用链：`pdm_mcu_command() → protocol_encode() → transport->xfer() → protocol_decode()`
- 预期可减少 2-3 层调用深度

### 2. **Backend 注册机制过重**

当前 backend 通过 linker section (`__section("pdm_backend_entries")`) 实现静态注册，但：
- `pdm_backend_registry.c` + `pdm_backend_registry_start.c` + `pdm_backend_registry_stop.c` 三个文件只为管理一个注册表
- `pdm_driver_registry.c` 同样的模式重复一遍

**建议**：
- 合并 `*_registry_start/stop.c` 到主文件（这些文件只有 2-3 行代码）
- 考虑是否真的需要 driver 和 backend 两套注册机制，LED 驱动目前只用了 backend

## 二、模块解耦问题

### 3. **Core 层内部循环依赖**

```
当前依赖关系：
pdm_bus.h → pdm_device.h → pdm_ctl.h (uapi)
pdm_client.h → pdm_device.h
pdm_backend.h → pdm_ctl.h (uapi)
```

**问题**：
- `pdm_device.h` 直接 include `pdm/pdm_ctl.h`（uapi），导致内核数据结构依赖用户态 API 定义
- `pdm_device` 结构体中的 `type`、`capabilities`、`state` 字段使用 uapi 中的枚举值

**建议**：
- 在 `kernel/include/pdm/core/` 下定义内核专用的类型常量
- uapi 和 kernel 分别维护自己的常量定义，通过编译时断言保证一致性
- 打破 core → uapi 的直接依赖

### 4. **Peripheral 层对 Core 层的过度耦合**

查看 `pdm_led.c` 和 `pdm_mcu.c`：
- 两个驱动的 probe 函数结构几乎完全相同（读 DT、选 backend、注册 client）
- 都需要手动管理 `atomic_t device_count`
- 都需要实现相似的 `claim_device()` / `release_device()` 模式

**建议**：
- 在 core 层提供 `pdm_driver_helper.h`，封装通用的 probe/remove 流程
- 将 device_count 管理下沉到 `pdm_device.c`
- 提供统一的设备访问互斥机制（避免每个驱动都实现一遍 claim/release）

## 三、接口调用深度问题

### 5. **MCU 设备访问链路过长（10层）**

当前从用户态到硬件需要经过 10 层函数调用，主要冗余在：
- 多层 protocol wrapper（cmd → cmd_xfer → xfer）
- `pdm_mcu_claim_device()` + `pdm_mcu_update_state_locked()` 的状态管理分散
- UART backend 的 `write_bus()` + `read_bus()` 封装在 `xfer()` 之下又多一层

**建议**：
- 合并 protocol 层的多层包装
- 将状态管理逻辑内聚到一个函数
- 考虑让 transport ops 直接返回最终结果，减少中间转换

**优化后预期**：
```
优化后（6-7层）：
userspace → ioctl → pdm_mcu_command() → protocol_xfer() 
→ transport->xfer() → kernel API
```

### 6. **Backend 选择机制的间接性**

```c
// LED 驱动中
inst->ops = pdm_led_backend_select(pdm_dev->compatible);
    → pdm_backend_find(type, class, compatible)
        → 遍历 linker section
        → 匹配 of_device_id
        → 返回 entry->ops
```

**问题**：
- `pdm_led_backend_select()` 只是简单包装了 `pdm_backend_find()` + 类型转换
- MCU 驱动的 `pdm_mcu_transport_select()` 同样的模式

**建议**：
- 移除 `*_backend_select()` 这一层包装，直接使用 `pdm_backend_find()`
- 或者在 core 层提供泛型版本 `pdm_backend_select(type, class, compatible, ops_type)`

## 四、横向对比和整体建议

### 7. **LED 和 MCU 驱动的重复代码**

两个驱动的相似度极高：
```
代码行数：LED 418 行 vs MCU 459 行
结构相似度：
  - probe 流程 90% 相同
  - ioctl 分发逻辑 80% 相同
  - backend 选择机制 100% 相同
  - client 注册流程 100% 相同
```

**建议**：
- 提取通用 peripheral driver 模板到 `pdm/core/driver/pdm_peripheral_driver.h`
- 提供标准的 probe/remove/ioctl 框架，驱动只需填充特定的 ops 表

### 8. **Core 层子模块粒度过细**

```
core/bus/          - 1个 .c 文件
core/chardev/      - 2个 .c 文件  
core/device/       - 2个 .c 文件
core/diag/         - 3个 .c 文件
core/driver/       - 6个 .c 文件（但实际是 2组 registry）
```

**问题**：
- 总共 2036 行代码分散在 5 个子目录
- 部分子目录只有一个文件（如 bus）
- driver 子目录的 6 个文件实际上是两组重复模式

**建议**：
- 考虑按功能合并：`core/device_mgmt/`（bus + device）、`core/chardev/`、`core/registry/`（driver + backend）
- 或者采用扁平结构，所有 core 文件直接放在 `core/` 下（总共才 14 个 .c 文件）

## 五、架构层面的建议优先级

**高优先级（显著降低复杂度）**：
1. **简化 MCU protocol 层调用链**（减少 2-3 层）
2. **提取 peripheral 驱动通用框架**（消除大量重复代码）
3. **打破 core → uapi 的依赖**（提升模块独立性）

**中优先级（提升代码质量）**：
4. 合并 registry 相关的碎片文件
5. 移除冗余的 backend_select() 包装
6. 统一设备访问互斥机制

**低优先级（可选优化）**：
7. 调整 core 子目录结构
8. 优化 linker section 注册机制

---

## 六、统计数据

**代码规模**：
- PDM 框架总代码量：4610 行
- Core 层：2036 行（bus + device + chardev）
- Peripheral 层：2415 行（LED + MCU）
- 文件总数：32 个

**调用深度对比**：
- LED 设备操作：6 层（相对合理）
- MCU 设备操作：10 层（偏长，建议优化到 6-7 层）
- 设备枚举绑定：9 层（可接受）

**代码重复度**：
- LED 和 MCU 驱动主体结构相似度：~85%
- Backend 和 Driver 两套注册机制：模式 100% 重复

---

## 总结

当前 PDM 框架在分层和抽象上做得比较完整，但存在**过度抽象**（10 层调用）和**抽象不足**（驱动重复代码）并存的问题。建议优先解决调用链过长和驱动代码重复这两个核心问题，这将显著降低框架的理解难度和维护成本。
