# ACL层重构迁移指南

## 重构概述

### 问题
原ACL层职责混乱，既包含静态配置，又包含运行时数据管理（遥测缓存），违反了单一职责原则。

### 解决方案
1. **ACL层**：只保留纯配置数据和查询接口（只读）
2. **OSAL层**：新增通用共享内存缓存服务（`osal_shm_cache`）
3. **APP层**：负责创建和管理缓存实例

## 架构变更

### 旧架构
```
ACL层（混乱）
├── 静态配置（acl_config.h）
└── 运行时数据管理（acl_telemetry_cache.c）
    ├── 共享内存管理
    ├── 互斥锁管理
    └── 缓存读写
```

### 新架构
```
OSAL层（通用IPC机制）
└── osal_shm_cache
    ├── 共享内存管理
    ├── 进程间互斥锁
    └── 通用缓存服务

ACL层（纯配置）
├── 静态配置表
└── 只读查询接口

APP层（业务逻辑）
└── 使用OSAL缓存服务
    ├── 创建缓存实例
    ├── 读写缓存数据
    └── 管理缓存生命周期
```

## API变更对照表

### 旧API（acl_telemetry_cache.h）
```c
// 初始化缓存（ACL层管理）
int32_t ACL_TelemetryCache_Init(void);

// 写入缓存
int32_t ACL_TelemetryCache_Write(uint32_t tm_id, const uint8_t *data, uint32_t data_len);

// 读取缓存
int32_t ACL_TelemetryCache_Read(uint32_t tm_id, telemetry_response_t *response);

// 失效缓存
int32_t ACL_TelemetryCache_Invalidate(uint32_t tm_id);
```

### 新API（osal_shm_cache.h + acl_api_v2.h）
```c
// OSAL层：通用缓存服务
osal_id_t cache_id;
OSAL_CacheCreate("tm_cache", TM_FUNC_MAX, &cache_id);  // APP层创建
OSAL_CacheWrite(cache_id, tm_id, data, data_len, validity_ms);
OSAL_CacheRead(cache_id, tm_id, &result);
OSAL_CacheInvalidate(cache_id, tm_id);

// ACL层：只读配置查询
const acl_tm_config_t *cfg = ACL_GetTmConfig(tm_id);
const acl_tc_config_t *tc_cfg = ACL_GetTcConfig(tc_id);
```

## 迁移步骤

### 步骤1：更新头文件引用
```c
// 旧代码
#include "acl_telemetry_cache.h"

// 新代码
#include "osal_shm_cache.h"  // 缓存服务
#include "acl_api_v2.h"      // 配置查询
```

### 步骤2：初始化缓存（APP层）
```c
// 旧代码（ACL层初始化）
ACL_TelemetryCache_Init();

// 新代码（APP层初始化）
osal_id_t g_tm_cache_id;

int32_t init_telemetry_cache(void)
{
    int32_t ret;
    
    // 创建共享内存缓存
    ret = OSAL_CacheCreate("pmc_tm_cache", TM_FUNC_MAX, &g_tm_cache_id);
    if (ret != OSAL_SUCCESS) {
        LOG_ERROR("APP", "创建遥测缓存失败: %d", ret);
        return ret;
    }
    
    LOG_INFO("APP", "遥测缓存创建成功，ID=%u", g_tm_cache_id);
    return OSAL_SUCCESS;
}
```

### 步骤3：更新写入代码（Telemetry进程）
```c
// 旧代码
int32_t update_telemetry(uint32_t tm_id, const uint8_t *data, uint32_t len)
{
    return ACL_TelemetryCache_Write(tm_id, data, len);
}

// 新代码
int32_t update_telemetry(uint32_t tm_id, const uint8_t *data, uint32_t len)
{
    const acl_tm_config_t *cfg;
    
    // 1. 查询配置（ACL层）
    cfg = ACL_GetTmConfig(tm_id);
    if (cfg == NULL || !cfg->enabled) {
        return OSAL_ERR_INVALID_PARAM;
    }
    
    // 2. 写入缓存（OSAL层）
    return OSAL_CacheWrite(g_tm_cache_id, tm_id, data, len, cfg->validity_ms);
}
```

### 步骤4：更新读取代码（Telecommand进程）
```c
// 旧代码
int32_t read_telemetry(uint32_t tm_id, telemetry_response_t *response)
{
    return ACL_TelemetryCache_Read(tm_id, response);
}

// 新代码
int32_t read_telemetry(uint32_t tm_id, osal_cache_result_t *result)
{
    const acl_tm_config_t *cfg;
    
    // 1. 查询配置（ACL层）
    cfg = ACL_GetTmConfig(tm_id);
    if (cfg == NULL || !cfg->enabled) {
        return OSAL_ERR_INVALID_PARAM;
    }
    
    // 2. 读取缓存（OSAL层）
    return OSAL_CacheRead(g_tm_cache_id, tm_id, result);
}
```

### 步骤5：更新失效代码（Telecommand进程）
```c
// 旧代码
int32_t invalidate_affected_telemetry(uint32_t tc_id)
{
    // 硬编码的失效列表
    uint32_t tm_ids[] = { TM_FUNC_SERVER_POWER_STATE };
    return ACL_TelemetryCache_InvalidateBatch(tm_ids, 1);
}

// 新代码
int32_t invalidate_affected_telemetry(acl_tc_function_t tc_id)
{
    const acl_tm_function_t *tm_ids;
    uint32_t count;
    int32_t ret;
    
    // 1. 查询失效映射（ACL层）
    ret = ACL_GetInvalidatedTelemetry(tc_id, &tm_ids, &count);
    if (ret != OSAL_SUCCESS || count == 0) {
        return OSAL_SUCCESS;  // 无需失效
    }
    
    // 2. 批量失效缓存（OSAL层）
    return OSAL_CacheInvalidateBatch(g_tm_cache_id, tm_ids, count);
}
```

### 步骤6：清理资源
```c
// 旧代码
ACL_TelemetryCache_Deinit();

// 新代码
OSAL_CacheDelete(g_tm_cache_id);
```

## 数据结构变更

### 旧结构（telemetry_response_t）
```c
typedef struct {
    uint32_t tm_id;
    uint8_t data[256];
    uint32_t data_len;
    uint64_t timestamp_us;
    uint32_t age_ms;
    acl_tm_status_t freshness;  // ACL层定义
    uint32_t checksum;
} telemetry_response_t;
```

### 新结构（osal_cache_result_t）
```c
typedef struct {
    uint32_t entry_id;              // 通用ID（不限于遥测）
    uint8_t data[256];
    uint32_t data_len;
    uint64_t timestamp_us;
    uint32_t age_ms;
    osal_cache_status_t status;     // OSAL层定义
    uint32_t checksum;
} osal_cache_result_t;
```

### 状态枚举映射
```c
// 旧枚举（ACL层）
typedef enum {
    ACL_TM_STATUS_INVALID = 0,
    ACL_TM_STATUS_FRESH,
    ACL_TM_STATUS_STALE
} acl_tm_status_t;

// 新枚举（OSAL层）
typedef enum {
    OSAL_CACHE_STATUS_INVALID = 0,
    OSAL_CACHE_STATUS_FRESH,
    OSAL_CACHE_STATUS_STALE
} osal_cache_status_t;

// 直接映射，无需转换
```

## 优势

### 1. 职责清晰
- **OSAL层**：通用IPC机制，可复用
- **ACL层**：纯配置，易于测试
- **APP层**：业务逻辑，灵活控制

### 2. 可复用性
```c
// 缓存服务不仅用于遥测，还可用于其他场景
osal_id_t config_cache_id;
OSAL_CacheCreate("config_cache", 100, &config_cache_id);

osal_id_t status_cache_id;
OSAL_CacheCreate("status_cache", 50, &status_cache_id);
```

### 3. 可测试性
```c
// ACL层单元测试（无需mock共享内存）
void test_acl_get_tm_config(void)
{
    ACL_Init();
    const acl_tm_config_t *cfg = ACL_GetTmConfig(TM_FUNC_SERVER_CPU_TEMP);
    assert(cfg != NULL);
    assert(cfg->validity_ms == 2000);
    ACL_Deinit();
}

// OSAL缓存单元测试（独立测试）
void test_osal_cache_write_read(void)
{
    osal_id_t cache_id;
    OSAL_CacheCreate("test_cache", 10, &cache_id);
    
    uint8_t data[] = {1, 2, 3, 4};
    OSAL_CacheWrite(cache_id, 0, data, 4, 1000);
    
    osal_cache_result_t result;
    OSAL_CacheRead(cache_id, 0, &result);
    assert(result.data_len == 4);
    
    OSAL_CacheDelete(cache_id);
}
```

### 4. 多产品支持
```c
// 不同产品使用不同的ACL配置
#ifdef PRODUCT_H200
    #include "acl_h200_config.h"
#elif defined(PRODUCT_DEMO)
    #include "acl_demo_config.h"
#endif

// 但都使用相同的OSAL缓存服务
OSAL_CacheCreate("tm_cache", TM_FUNC_MAX, &cache_id);
```

## 兼容性

### 过渡期方案
如果需要保持旧代码兼容，可以提供适配层：

```c
// acl_telemetry_cache_compat.h（兼容层）
#include "osal_shm_cache.h"
#include "acl_api_v2.h"

extern osal_id_t g_compat_cache_id;

static inline int32_t ACL_TelemetryCache_Init(void)
{
    return OSAL_CacheCreate("tm_cache", TM_FUNC_MAX, &g_compat_cache_id);
}

static inline int32_t ACL_TelemetryCache_Write(uint32_t tm_id,
                                                const uint8_t *data,
                                                uint32_t data_len)
{
    const acl_tm_config_t *cfg = ACL_GetTmConfig(tm_id);
    if (cfg == NULL) return OSAL_ERR_INVALID_PARAM;
    return OSAL_CacheWrite(g_compat_cache_id, tm_id, data, data_len, cfg->validity_ms);
}

// ... 其他兼容函数
```

## 检查清单

迁移完成后，检查以下项目：

- [ ] ACL层不再包含运行时数据管理代码
- [ ] ACL层不再创建共享内存、互斥锁等资源
- [ ] APP层负责创建和管理缓存实例
- [ ] 所有缓存操作使用OSAL接口
- [ ] 所有配置查询使用ACL接口
- [ ] 单元测试更新并通过
- [ ] 集成测试验证功能正常
- [ ] 性能测试确认<50μs读取延迟
- [ ] 文档更新（架构图、API文档）

## 常见问题

### Q1: 为什么要移到OSAL层？
**A**: OSAL层是通用的操作系统抽象层，共享内存缓存是通用的IPC机制，不应该绑定到特定业务（遥测）。移到OSAL层后可以复用于其他场景。

### Q2: 性能会受影响吗？
**A**: 不会。新实现使用相同的POSIX共享内存和互斥锁机制，性能相同。实际上，由于减少了间接调用，性能可能略有提升。

### Q3: 需要修改多少代码？
**A**: 主要修改APP层的初始化和调用代码，约100-200行。ACL层和OSAL层是新增代码，不影响现有功能。

### Q4: 如何验证迁移正确？
**A**: 
1. 运行单元测试
2. 运行集成测试（端到端遥控遥测）
3. 性能测试（确认<2ms应答）
4. 长时间稳定性测试（72小时）

## 总结

这次重构解决了ACL层职责混乱的问题，使架构更加清晰：
- **OSAL层**：通用服务，可复用
- **ACL层**：纯配置，易测试
- **APP层**：业务逻辑，灵活控制

符合单一职责原则和分层架构设计原则。
