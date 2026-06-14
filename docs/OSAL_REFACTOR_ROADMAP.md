# OSAL 重构路线图

## 目标
将 OSAL 从高层封装改为薄封装层，遵循 OSAL_xxx 命名规范（xxx 为小写）

## 已完成工作

### 阶段 1: 命名规范化和基础重构（会话 1）

#### 1. 命名规范化
- ✅ **util 模块**（26 API）：版本、日志 API 全部小写化
- ✅ **lib 模块**（18 API）：errno、stdio、heap、flock API 小写化
- ✅ **sys 模块**（15 API）：时间、进程相关 API 小写化

#### 2. 架构重构
- ✅ **sys/Process 模块**：删除 6 个高层 API，添加 4 个 POSIX 薄封装
  - 删除：ProcessCreate/Kill/Wait/Exists/GetId/GetParentId
  - 添加：getppid, execvp, fork, waitpid
  - 更新 supervisor 代码使用标准 POSIX 接口

#### 3. 提交记录（会话 1）
```
fce656d - refactor: rename OSAL_Printf to OSAL_printf
214ae12 - refactor(osal/util): rename util module APIs to lowercase
2c70e86 - refactor(osal/lib): rename lib module APIs to lowercase
5bbf10a - refactor(osal/sys): rename sys module APIs to lowercase (part 1)
4858508 - refactor(osal/sys): remove Process high-level API, use POSIX directly
3f9d6c8 - docs: add OSAL refactoring roadmap
```

### 阶段 2: IPC 模块重构（会话 2）

#### 1. 完成的 IPC 模块
- ✅ **Semaphore 模块**（51 uses）- POSIX sem_t 薄封装
- ✅ **Cond 模块**（42 uses）- pthread_cond_t 薄封装  
- ✅ **Rwlock 模块**（32 uses）- pthread_rwlock_t 薄封装
- ✅ **Mutex 模块**（366 uses）- pthread_mutex_t 薄封装（仅 API）

#### 2. 提交记录（会话 2）
```
436c42c - refactor(osal/ipc): refactor Semaphore to POSIX thin wrapper
e162daf - refactor(osal/ipc): refactor Cond to POSIX thin wrapper
4313188 - refactor(osal/ipc): refactor Rwlock to POSIX thin wrapper
138ef0f - refactor(osal/ipc): refactor Mutex to POSIX thin wrapper (API only)
```

## 当前状态

### 已完成（约 40%）
- 命名规范化：59 个 API
- 架构重构：Process + 4 个 IPC 模块（API 层面）
- 总共提交：10 个 commits

### 待完成工作

#### 关键任务：Mutex 迁移（高优先级）⚠️
Mutex API 已重构，但 366 处调用点需要迁移：

**迁移策略**：
1. **测试代码**（~100 uses）：
   - test_osal_mutex.c - 直接更新为新 API
   - test_osal_cond.c - 依赖 Mutex，需要同步更新
   - 其他测试 - 逐个更新

2. **应用代码**（~266 uses）：
   - libccm IPC 代码（~50 uses）
   - PDL 驱动代码（~100 uses）
   - CCM 应用代码（~116 uses）

**迁移模式**：
```c
// 旧代码
osal_mutex_t *mutex;
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

**预计工作量**：8-12 小时

#### 剩余 IPC 模块

5. **Thread 模块**（120 uses）
   - 需要合并 sys/osal_thread.h 和 ipc 相关
   - 改为 pthread_* 薄封装
   - API: create, join, detach, self, exit
   - ThreadAttr: init, destroy, set* (栈大小、调度等)

6. **Atomic 模块**（220 uses）
   - 评估是否完全移除，直接用 C11 _Atomic 或 GCC __sync_*
   - 或者提供极薄的封装用于跨平台

7. **Shm 模块**（80 uses）
   - 改为 shm_open/mmap/munmap 薄封装
   - API: open, close, map, unmap, unlink

8. **Shm_cache 模块**（17 uses）
   - 评估是否为应用层逻辑
   - 可能移到 HAL 或 PDL 层，或完全删除

#### 阶段 3: sys 模块剩余部分

9. **Signal 模块**（35 uses）
   - 改为 sigaction/sigprocmask/sigwait 薄封装
   - 删除：SignalRegister/Block/Unblock/Ignore/Default
   - 添加：sigaction, sigprocmask, sigwait, signal

10. **Sched 模块**（60 uses）
    - 改为 sched_* 和 pthread_setschedparam 薄封装
    - 删除：SchedGetPolicy/SetPolicy/GetPriority/SetPriority 等
    - 添加：sched_getscheduler, sched_setscheduler, sched_getparam 等

#### 阶段 4: 验证和测试
11. 更新所有测试用例
12. 验证 CCM 应用功能
13. 性能基准测试
14. 文档更新

## 预计工作量

| 阶段 | 模块数 | API 数 | 使用次数 | 状态 | 预计剩余时间 |
|------|--------|--------|----------|------|-------------|
| 阶段 1 | 3 | 59 | ~200 | ✅ 完成 | - |
| 阶段 2 (IPC API) | 4 | ~30 | ~491 | ✅ 完成 | - |
| **Mutex 迁移** | - | - | **366** | ⚠️ 待迁移 | **8-12 小时** |
| 阶段 2 (剩余) | 4 | ~40 | ~437 | 待开始 | 8-12 小时 |
| 阶段 3 (sys) | 2 | ~20 | ~95 | 待开始 | 3-5 小时 |
| 阶段 4 (验证) | - | - | - | 待开始 | 2-3 小时 |
| **总计** | **13** | **~149** | **~1589** | **40% 完成** | **21-32 小时** |

## 技术决策

### Mutex/Thread/Semaphore 重构方案
```c
// 旧方案（高层封装，隐藏 pthread 类型）
osal_mutex_t *mutex;
OSAL_MutexCreate(&mutex);
OSAL_MutexLock(mutex);
OSAL_MutexUnlock(mutex);
OSAL_MutexDelete(mutex);

// 新方案（薄封装，直接暴露 pthread 类型）
pthread_mutex_t mutex;
OSAL_pthread_mutex_init(&mutex, NULL);
OSAL_pthread_mutex_lock(&mutex);
OSAL_pthread_mutex_unlock(&mutex);
OSAL_pthread_mutex_destroy(&mutex);
```

**优点**：
- 符合 POSIX 标准，易于移植
- 代码更清晰，减少抽象层
- 性能更好（减少间接调用和内存分配）

**缺点**：
- API 变化较大，需要大量代码修改
- pthread 类型暴露给用户代码

### Atomic 重构方案选择

**方案 A：完全移除 OSAL_Atomic，使用 C11 _Atomic**
```c
// 旧代码
OSAL_AtomicLoad(&counter);

// 新代码
_Atomic int counter;
atomic_load(&counter);
```

**方案 B：保留极薄封装，兼容非 C11 编译器**
```c
#ifdef __STDC_VERSION__ >= 201112L
  #define OSAL_atomic_load(ptr) atomic_load(ptr)
#else
  #define OSAL_atomic_load(ptr) __sync_fetch_and_add(ptr, 0)
#endif
```

**推荐**：方案 B，提供最小封装以保证跨平台兼容性

## 执行建议

1. **分批次进行**：每次处理 1-2 个模块，完成后验证编译并提交
2. **优先测试代码**：先更新测试用例，确保新 API 正确
3. **保持向后兼容**：可以暂时保留旧 API 作为 deprecated wrapper
4. **文档先行**：每个模块重构前，先写清楚新旧 API 映射关系

## 风险评估

| 风险 | 严重性 | 缓解措施 |
|------|--------|----------|
| 大量代码需要修改 | 高 | 使用脚本批量替换，分阶段验证 |
| 测试覆盖率不足 | 中 | 补充单元测试，运行集成测试 |
| API 语义变化 | 中 | 仔细审查每个 API 的行为差异 |
| 性能回退 | 低 | 运行性能基准测试 |

## 后续行动

**下次会话优先级**：
1. **Mutex 迁移**（最关键）- 更新 366 处调用点
2. Thread 模块重构
3. Atomic 模块评估和重构
4. 完成剩余 IPC 模块
