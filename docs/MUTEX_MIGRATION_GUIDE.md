# Mutex 迁移指南

## 当前状态

### 已完成
- ✅ Mutex API 重构为 pthread_mutex_t 薄封装
- ✅ 测试代码迁移完成（test_osal_mutex.c, test_osal_cond.c）
- ✅ 编译通过，所有测试正常

### 待完成
- ⚠️ 应用代码迁移（253 处）

## 迁移模式

### 模式 1：局部变量（简单）

```c
// 旧代码
osal_mutex_t *mutex = NULL;
OSAL_MutexCreate(&mutex);
OSAL_MutexLock(mutex);
OSAL_MutexUnlock(mutex);
OSAL_MutexDelete(mutex);

// 新代码
pthread_mutex_t mutex;
OSAL_pthread_mutex_init(&mutex, NULL);
OSAL_pthread_mutex_lock(&mutex);
OSAL_pthread_mutex_unlock(&mutex);
OSAL_pthread_mutex_destroy(&mutex);
```

**工具**：使用 `scripts/migrate_mutex.sh` 自动转换

### 模式 2：结构体成员（复杂）

```c
// 旧代码
typedef struct {
    osal_mutex_t *mutex;  // 指针
    // ...
} context_t;

context_t *ctx = malloc(sizeof(context_t));
OSAL_MutexCreate(&ctx->mutex);
OSAL_MutexLock(ctx->mutex);
OSAL_MutexUnlock(ctx->mutex);
OSAL_MutexDelete(ctx->mutex);

// 新代码
typedef struct {
    pthread_mutex_t mutex;  // 直接嵌入，不是指针
    // ...
} context_t;

context_t *ctx = malloc(sizeof(context_t));
OSAL_pthread_mutex_init(&ctx->mutex, NULL);
OSAL_pthread_mutex_lock(&ctx->mutex);
OSAL_pthread_mutex_unlock(&ctx->mutex);
OSAL_pthread_mutex_destroy(&ctx->mutex);
```

**步骤**：
1. 修改结构体定义：`osal_mutex_t *mutex` → `pthread_mutex_t mutex`
2. 修改初始化：`OSAL_MutexCreate(&ctx->mutex)` → `OSAL_pthread_mutex_init(&ctx->mutex, NULL)`
3. 修改使用：`OSAL_MutexLock(ctx->mutex)` → `OSAL_pthread_mutex_lock(&ctx->mutex)`
4. 修改销毁：`OSAL_MutexDelete(ctx->mutex)` → `OSAL_pthread_mutex_destroy(&ctx->mutex)`

**工具**：需要手动处理或定制脚本

### 模式 3：递归锁

```c
// 旧代码
osal_mutex_attr_t *attr;
osal_mutex_t *mutex;
OSAL_MutexAttrCreate(&attr);
OSAL_MutexAttrSetType(attr, OSAL_MUTEX_RECURSIVE);
OSAL_MutexCreateEx(&mutex, attr);
OSAL_MutexAttrDestroy(attr);

// 新代码
pthread_mutexattr_t attr;
pthread_mutex_t mutex;
OSAL_pthread_mutexattr_init(&attr);
OSAL_pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
OSAL_pthread_mutex_init(&mutex, &attr);
OSAL_pthread_mutexattr_destroy(&attr);
```

## 待迁移文件列表

### HAL 层（~80 uses）
| 文件 | 使用次数 | 模式 | 优先级 |
|------|---------|------|--------|
| hal_gpio_linux.c | 32 | 结构体成员 | 高 |
| hal_spi_linux.c | 16 | 结构体成员 | 中 |
| hal_i2c_linux.c | 11 | 结构体成员 | 中 |
| hal_can_linux.c | 11 | 结构体成员 | 中 |
| hal_serial_linux.c | 10 | 结构体成员 | 中 |

### PDL 层（~150 uses）
| 文件 | 使用次数 | 模式 | 优先级 |
|------|---------|------|--------|
| pdl_ccm.c | 51 | 结构体成员 | 高 |
| pdl_satellite.c | 27 | 结构体成员 | 高 |
| pdl_bmc.c | 22 | 结构体成员 | 中 |
| pdl_mcu.c | 13 | 结构体成员 | 中 |
| pdl_bmc_ipmi.c | 12 | 结构体成员 | 中 |
| pdl_ccm_eth.c | 6 | 结构体成员 | 低 |
| pdl_mcu_can.c | 6 | 结构体成员 | 低 |
| pdl_bmc_redfish.c | 6 | 结构体成员 | 低 |
| pdl_mcu_serial.c | 5 | 结构体成员 | 低 |

### 应用层（~23 uses）
| 文件 | 使用次数 | 模式 | 优先级 |
|------|---------|------|--------|
| libccm_ipc.c | 10 | 结构体成员 | 高 |
| pconfig_api.c | 14 | 结构体成员 | 中 |

## 迁移策略

### 选项 A：渐进式迁移（推荐）
1. **阶段 1**：先迁移测试代码（✅ 已完成）
2. **阶段 2**：迁移独立模块（优先级高的 HAL/PDL）
3. **阶段 3**：迁移应用代码
4. **阶段 4**：删除旧 API 和兼容层

**优点**：
- 风险可控，每次只改少量文件
- 可以逐个验证功能
- 容易回滚

**缺点**：
- 时间较长（预计 6-8 小时）

### 选项 B：大爆炸迁移
1. 创建定制的批量转换脚本
2. 一次性转换所有文件
3. 修复编译错误
4. 全面测试

**优点**：
- 快速完成（预计 3-4 小时）

**缺点**：
- 风险高，可能引入大量问题
- 难以定位问题
- 回滚困难

### 选项 C：兼容层过渡
1. 保留旧 API 作为兼容包装
2. 新代码使用新 API
3. 旧代码逐步迁移
4. 最终删除兼容层

**优点**：
- 风险最低
- 可以无限期并存

**缺点**：
- 代码混乱（两套 API 共存）
- 维护成本高
- 不彻底

## 推荐方案：选项 A（渐进式迁移）

### 执行步骤

#### 第 1 步：创建结构体成员迁移脚本

```bash
#!/bin/bash
# migrate_mutex_struct.sh
# 用于迁移结构体成员中的 Mutex

file=$1

# 1. 替换结构体成员声明
sed -i 's/osal_mutex_t \*mutex;/pthread_mutex_t mutex;/g' "$file"

# 2. 替换 Create (注意取地址符的变化)
sed -i 's/OSAL_MutexCreate(&\(ctx->\|[a-z_]*->\)mutex)/OSAL_pthread_mutex_init(\&\1mutex, NULL)/g' "$file"

# 3. 替换 Lock (添加取地址符)
sed -i 's/OSAL_MutexLock(\(ctx->\|[a-z_]*->\)mutex)/OSAL_pthread_mutex_lock(\&\1mutex)/g' "$file"

# 4. 替换 Unlock (添加取地址符)
sed -i 's/OSAL_MutexUnlock(\(ctx->\|[a-z_]*->\)mutex)/OSAL_pthread_mutex_unlock(\&\1mutex)/g' "$file"

# 5. 替换 Delete (添加取地址符)
sed -i 's/OSAL_MutexDelete(\(ctx->\|[a-z_]*->\)mutex)/OSAL_pthread_mutex_destroy(\&\1mutex)/g' "$file"
```

#### 第 2 步：按优先级迁移

**第一批（高优先级，验证模式）**：
```bash
./migrate_mutex_struct.sh core/hal/src/linux/gpio/hal_gpio_linux.c
./migrate_mutex_struct.sh core/pdl/src/pdl_ccm/pdl_ccm.c
# 编译测试
make
# 功能测试
```

**第二批（中优先级）**：
```bash
for f in core/hal/src/linux/spi/*.c core/pdl/src/pdl_satellite/*.c; do
    ./migrate_mutex_struct.sh "$f"
done
make
```

**第三批（低优先级）**：
```bash
find core/pdl -name "*.c" -exec grep -l "osal_mutex_t" {} \; | \
    xargs -I {} ./migrate_mutex_struct.sh {}
make
```

## 验证清单

每批迁移后都要验证：

- [ ] 编译通过（无错误和警告）
- [ ] 单元测试通过
- [ ] 集成测试通过（如果有）
- [ ] 功能测试（手动或自动）

## 回滚策略

每批迁移前：
```bash
git checkout -b mutex-migration-batch-N
# 进行迁移
git add -A
git commit -m "refactor: migrate batch N mutex to pthread API"
# 如果有问题
git checkout master
```

## 预计时间

| 阶段 | 时间 | 说明 |
|------|------|------|
| 脚本开发 | 0.5h | 创建和测试迁移脚本 |
| 第一批迁移 | 1h | 高优先级文件 + 验证 |
| 第二批迁移 | 2h | 中优先级文件 + 验证 |
| 第三批迁移 | 2h | 低优先级文件 + 验证 |
| 集成测试 | 1h | 全面功能测试 |
| 清理和文档 | 0.5h | 删除旧 API，更新文档 |
| **总计** | **7h** | |

## 后续任务

Mutex 迁移完成后，继续其他模块：
1. Thread 模块（120 uses）
2. Atomic 模块（220 uses）
3. Shm 模块（80 uses）
4. Signal 模块（35 uses）
5. Sched 模块（60 uses）
