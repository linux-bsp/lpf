# AConfig 优化方案与迁移指南

## 概述

AConfig 已完成优化，采用分阶段迁移策略，确保向后兼容。

### 优化目标

1. **分层清晰**：业务功能定义移到产品层
2. **内存高效**：稀疏数组替代密集数组，节省 90%+ 内存
3. **查找高效**：设备索引引用替代字符串查找，O(n) → O(1)
4. **维护简便**：失效映射内嵌，配置集中管理

## 架构变化

### 旧架构

```
core/aconfig/
├── include/aconfig/
│   ├── aconfig.h
│   ├── aconfig_types.h
│   ├── aconfig_tc.h          # 业务功能枚举（错误位置）
│   └── aconfig_tm.h          # 业务功能枚举（错误位置）
└── src/
    └── aconfig_api.c

products/ccm/configs/
└── h200/
    └── aconfig/
        ├── config.c           # 密集数组配置
        └── invalidation.c     # 独立的失效映射
```

### 新架构

```
core/aconfig/                  # 核心层：只提供框架
├── include/aconfig/
│   ├── aconfig.h
│   ├── aconfig_types.h        # 通用数据结构
│   ├── aconfig_tc.h           # 兼容性包装
│   └── aconfig_tm.h           # 兼容性包装
└── src/
    └── aconfig_api.c          # 支持新旧两种格式

products/ccm/                  # 产品层：业务功能定义
├── include/ccm/
│   ├── ccm_tc_functions.h     # CCM 遥控功能（正确位置）
│   └── ccm_tm_functions.h     # CCM 遥测功能（正确位置）
└── configs/
    └── h200/
        └── aconfig/
            ├── config_legacy.c      # 旧格式（兼容）
            └── config_optimized.c   # 新格式（推荐）
```

## 数据结构对比

### 旧格式：密集数组

```c
/* 必须预分配整个枚举空间 */
static const aconfig_tc_config_t g_tc_table[ACONFIG_TC_FUNC_MAX] = {
    [ACONFIG_TC_POWER_ON] = {
        .function_id = ACONFIG_TC_POWER_ON,
        .device_type = ACONFIG_DEVICE_BMC,
        .device_name = "payload_bmc",  // 字符串查找 O(n)
        .enabled = true
    },
    // ... 其他 998 个元素都是空的，浪费内存
};

static const aconfig_config_table_t g_table = {
    .tc_table = g_tc_table,
    .tc_count = ACONFIG_TC_FUNC_MAX,  // 1000
    .inv_map = g_inv_map,             // 独立的失效映射
    .inv_count = 10
};
```

**问题**：
- ❌ 内存浪费：即使只有 10 个功能，也要分配 1000 个元素
- ❌ 查找效率低：字符串比较查找设备
- ❌ 配置分散：失效映射在独立文件中

### 新格式：稀疏数组

```c
/* 只定义实际使用的功能 */
static const uint32_t power_on_affects[] = {CCM_TM_POWER_STATUS};

static const aconfig_tc_entry_t g_tc_entries[] = {
    {
        .function_id = CCM_TC_POWER_ON,
        .config = {
            .function_id = CCM_TC_POWER_ON,
            .device = {.type = PCONFIG_DEVICE_BMC, .index = 0},  // 索引引用 O(1)
            .invalidated_tm_ids = power_on_affects,  // 失效映射内嵌
            .invalidated_tm_count = 1,
            .enabled = true
        }
    }
    // 只定义实际使用的功能
};

static const aconfig_config_table_t g_table = {
    .tc_entries = g_tc_entries,
    .tc_count = 1  // 实际数量
};
```

**优势**：
- ✅ 内存节省：只存储实际功能（节省 90%+）
- ✅ 查找高效：设备索引引用 O(1)
- ✅ 配置集中：失效映射内嵌

## 迁移步骤

### 步骤 1：更新头文件包含

**旧代码：**
```c
#include "aconfig.h"
#include "aconfig_tc.h"  // 会有 warning
#include "aconfig_tm.h"  // 会有 warning
```

**新代码：**
```c
#include "aconfig.h"
#include "ccm_tc_functions.h"  // 产品层头文件
#include "ccm_tm_functions.h"  // 产品层头文件
```

### 步骤 2：转换失效映射

**旧格式：独立文件**
```c
// aconfig_invalidation.c
static uint32_t power_on_affects[] = {ACONFIG_TM_POWER_STATUS};

static const aconfig_invalidation_map_t g_inv_map[] = {
    {
        .source_tm_id = ACONFIG_TC_POWER_ON,
        .affected_tm_ids = power_on_affects,
        .affected_count = 1
    }
};
```

**新格式：内嵌到配置**
```c
// 直接在配置文件中定义
static const uint32_t tc_power_on_affects[] = {CCM_TM_POWER_STATUS};

// 在 TC 配置中引用
.config = {
    .invalidated_tm_ids = tc_power_on_affects,
    .invalidated_tm_count = 1
}
```

### 步骤 3：转换遥控配置

**旧格式：密集数组**
```c
static const aconfig_tc_config_t g_tc_table[ACONFIG_TC_FUNC_MAX] = {
    [ACONFIG_TC_POWER_ON] = {
        .function_id = ACONFIG_TC_POWER_ON,
        .device_type = ACONFIG_DEVICE_BMC,
        .device_name = "payload_bmc",
        .enabled = true
    }
};
```

**新格式：稀疏数组**
```c
static const aconfig_tc_entry_t g_tc_entries[] = {
    {
        .function_id = CCM_TC_POWER_ON,  // 注意：使用 CCM_ 前缀
        .config = {
            .function_id = CCM_TC_POWER_ON,
            .device = {.type = PCONFIG_DEVICE_BMC, .index = 0},
            .invalidated_tm_ids = tc_power_on_affects,
            .invalidated_tm_count = 1,
            .enabled = true,
            .user_context = NULL
        }
    }
};
```

### 步骤 4：转换遥测配置

**旧格式：**
```c
static const aconfig_tm_config_t g_tm_table[ACONFIG_TM_FUNC_MAX] = {
    [ACONFIG_TM_VOLTAGE_5V] = {
        .function_id = ACONFIG_TM_VOLTAGE_5V,
        .device_type = ACONFIG_DEVICE_BMC,
        .device_name = "payload_bmc",
        .data_validity_ms = 4000,
        .background_update_period_ms = 2000,
        .enabled = true
    }
};
```

**新格式：**
```c
static const aconfig_tm_entry_t g_tm_entries[] = {
    {
        .function_id = CCM_TM_VOLTAGE_5V,  // 注意：使用 CCM_ 前缀
        .config = {
            .function_id = CCM_TM_VOLTAGE_5V,
            .device = {.type = PCONFIG_DEVICE_BMC, .index = 0},
            .poll_period_ms = 2000,         // 改名
            .validity_period_ms = 4000,     // 改名
            .enabled = true,
            .user_context = NULL
        }
    }
};
```

### 步骤 5：转换配置表

**旧格式：**
```c
static const aconfig_config_table_t g_table = {
    .name = "CCM_H200",
    .tc_table = g_tc_table,
    .tc_count = ACONFIG_TC_FUNC_MAX,
    .tm_table = g_tm_table,
    .tm_count = ACONFIG_TM_FUNC_MAX,
    .inv_map = g_inv_map,
    .inv_count = 10
};

// 注册
ACONFIG_RegisterTable(&g_table);
```

**新格式：**
```c
static const aconfig_config_table_t g_table = {
    .name = "CCM_H200",
    .hwid_count = 0,
    .hwid_list = NULL,
    .tc_entries = g_tc_entries,
    .tc_count = sizeof(g_tc_entries) / sizeof(aconfig_tc_entry_t),
    .tm_entries = g_tm_entries,
    .tm_count = sizeof(g_tm_entries) / sizeof(aconfig_tm_entry_t)
};

// 注册（API 相同）
ACONFIG_RegisterTable(&g_table);
```

### 步骤 6：设备索引映射

新格式使用设备索引替代设备名称。需要建立映射关系：

**映射表：**
```
设备名称          →  (设备类型, 索引)
"payload_bmc"     →  (PCONFIG_DEVICE_BMC, 0)
"power_mcu"       →  (PCONFIG_DEVICE_MCU, 0)
"main_fpga"       →  (PCONFIG_DEVICE_FPGA, 0)
"temp_sensor"     →  (PCONFIG_DEVICE_SENSOR, 0)
```

索引对应 PConfig 中设备的数组位置。

## 兼容性支持

当前版本**同时支持新旧两种格式**，可以平滑迁移：

### 使用旧格式（兼容性 API）

```c
// 使用旧版数据结构
static const aconfig_tc_config_legacy_t g_tc_table[ACONFIG_TC_FUNC_MAX] = {...};
static const aconfig_config_table_legacy_t g_table_legacy = {...};

// 使用兼容性注册函数
ACONFIG_RegisterTableLegacy(&g_table_legacy);
```

### 使用新格式（推荐）

```c
// 使用新版数据结构
static const aconfig_tc_entry_t g_tc_entries[] = {...};
static const aconfig_config_table_t g_table = {...};

// 使用标准注册函数
ACONFIG_RegisterTable(&g_table);
```

### 应用层无需修改

无论使用哪种格式，**应用层 API 完全相同**：

```c
// 查询接口保持不变
const aconfig_tc_config_t *cfg = ACONFIG_GetTcConfig(CCM_TC_POWER_ON);
bool enabled = ACONFIG_IsTcEnabled(CCM_TC_POWER_ON);

// 自动识别新旧格式
```

## 迁移时间表

### 阶段 1：兼容性支持（当前）✅

- 核心层支持新旧两种格式
- 旧配置继续工作
- 新代码可以使用新格式

### 阶段 2：逐步迁移（1-2个月）

- 新平台使用新格式
- 旧平台保持兼容格式
- 文档更新

### 阶段 3：完全迁移（3-6个月）

- 所有平台迁移到新格式
- 删除兼容性 API
- 删除旧版数据结构

## 示例代码

### 完整示例

参考文件：
- **新格式示例**：`products/ccm/configs/platforms/h200_100p_am625/aconfig/h200_aconfig_optimized.c`
- **旧格式示例**：`products/ccm/configs/platforms/h200_100p_am625/aconfig/ccm_aconfig_config.c`

### 快速对比

| 方面 | 旧格式 | 新格式 |
|------|--------|--------|
| 头文件 | `aconfig_tc.h` | `ccm_tc_functions.h` |
| 数组类型 | 密集数组 | 稀疏数组 |
| 设备引用 | 字符串名称 | 类型+索引 |
| 失效映射 | 独立文件 | 内嵌配置 |
| 内存占用 | ~40KB | ~4KB |
| 查找效率 | O(n) | O(1) |
| 注册API | `ACONFIG_RegisterTableLegacy` | `ACONFIG_RegisterTable` |
| 应用API | 相同 | 相同 |

## 常见问题

### Q: 现有代码需要修改吗？

**A:** 应用层代码**无需修改**。只需更新平台配置文件。

### Q: 旧配置还能用吗？

**A:** 可以。通过 `ACONFIG_RegisterTableLegacy()` 注册即可。

### Q: 何时必须迁移？

**A:** 没有强制时间。建议新平台使用新格式，旧平台可以保持兼容格式。

### Q: 如何测试新格式？

**A:** 参考 `h200_aconfig_optimized.c` 示例，创建新格式配置并测试。

### Q: 设备索引如何确定？

**A:** 索引对应 PConfig 中设备在数组中的位置。第一个设备索引为 0。

### Q: 新格式性能提升多少？

**A:** 内存节省 90%+，设备查找从 O(n) 提升到 O(1)。

## 总结

AConfig 优化采用**分阶段、向后兼容**的迁移策略：

✅ **现在**：两种格式并存，应用层无需修改  
🔄 **未来**：逐步迁移到新格式  
🎯 **目标**：更清晰的分层、更高效的性能、更简便的维护

---

*最后更新：2024-06*
