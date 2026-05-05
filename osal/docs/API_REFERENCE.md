# OSAL API 参考手册

本文档提供OSAL所有公共API的详细说明。

## 目录

- [IPC模块 - 进程间通信](#ipc模块---进程间通信)
  - [任务管理](#任务管理)
  - [消息队列](#消息队列)
  - [互斥锁](#互斥锁)
  - [信号量](#信号量)
  - [条件变量](#条件变量)
  - [原子操作](#原子操作)
- [SYS模块 - 系统调用](#sys模块---系统调用)
  - [时钟](#时钟)
  - [信号处理](#信号处理)
  - [文件操作](#文件操作)
  - [Select](#select)
  - [环境变量](#环境变量)
  - [时间操作](#时间操作)
- [NET模块 - 网络](#net模块---网络)
  - [Socket](#socket)
  - [Termios](#termios)
- [LIB模块 - 标准库](#lib模块---标准库)
  - [字符串操作](#字符串操作)
  - [内存管理](#内存管理)
  - [错误处理](#错误处理)
- [UTIL模块 - 工具](#util模块---工具)
  - [日志系统](#日志系统)
  - [版本信息](#版本信息)

---

## IPC模块 - 进程间通信

### 任务管理

#### OSAL_TaskCreate

创建新任务。

```c
int32_t OSAL_TaskCreate(osal_id_t *task_id,
                        const char *task_name,
                        osal_task_entry entry_point,
                        void *arg,
                        uint32_t stack_size,
                        uint32_t priority,
                        uint32_t flags);
```

**参数**：
- `task_id` - [输出] 任务ID
- `task_name` - 任务名称（最大20字符）
- `entry_point` - 任务入口函数
- `arg` - 传递给任务的参数
- `stack_size` - 栈大小（字节）
- `priority` - 优先级（0-255，数值越大优先级越高）
- `flags` - 标志位（保留，传0）

**返回值**：
- `OS_SUCCESS` - 成功
- `OS_INVALID_POINTER` - 无效指针
- `OS_ERR_NAME_TOO_LONG` - 名称过长
- `OS_ERR_NO_FREE_IDS` - 无可用任务槽
- `OS_ERR_NAME_TAKEN` - 名称已存在
- `OS_ERROR` - 创建失败

**示例**：
```c
void my_task(void *arg)
{
    while (!OSAL_TaskShouldShutdown()) {
        /* 任务逻辑 */
        OSAL_TaskDelay(1000);
    }
}

osal_id_t task_id;
int32_t ret = OSAL_TaskCreate(&task_id, "MyTask", my_task, NULL,
                               16*1024, 100, 0);
if (ret != OS_SUCCESS) {
    LOG_ERROR("Main", "创建任务失败: %d", ret);
}
```

#### OSAL_TaskDelete

删除任务。

```c
int32_t OSAL_TaskDelete(osal_id_t task_id);
```

**参数**：
- `task_id` - 任务ID

**返回值**：
- `OS_SUCCESS` - 成功
- `OS_ERR_INVALID_ID` - 无效任务ID
- `OS_ERROR` - 删除失败

**注意**：
- 删除前会设置shutdown标志并等待任务退出（5秒超时）
- 任务应在循环中检查`OSAL_TaskShouldShutdown()`
- 超时后任务会被detach而非强制取消

#### OSAL_TaskShouldShutdown

检查任务是否应该退出。

```c
bool OSAL_TaskShouldShutdown(void);
```

**返回值**：
- `true` - 应该退出
- `false` - 继续运行

**示例**：
```c
void worker_task(void *arg)
{
    while (!OSAL_TaskShouldShutdown()) {
        /* 执行工作 */
        OSAL_TaskDelay(100);
    }
    /* 清理资源 */
}
```

#### OSAL_TaskDelay

任务延时。

```c
int32_t OSAL_TaskDelay(uint32_t milliseconds);
```

**参数**：
- `milliseconds` - 延时时间（毫秒）

**返回值**：
- `OS_SUCCESS` - 成功

#### OSAL_TaskGetId

获取当前任务ID。

```c
osal_id_t OSAL_TaskGetId(void);
```

**返回值**：
- 当前任务ID
- 0 - 如果不在OSAL任务上下文中

#### OSAL_TaskGetIdByName

根据名称查找任务ID。

```c
int32_t OSAL_TaskGetIdByName(osal_id_t *task_id, const char *task_name);
```

**参数**：
- `task_id` - [输出] 任务ID
- `task_name` - 任务名称

**返回值**：
- `OS_SUCCESS` - 成功
- `OS_INVALID_POINTER` - 无效指针
- `OS_ERR_NAME_NOT_FOUND` - 未找到任务

---

### 消息队列

#### OSAL_QueueCreate

创建消息队列。

```c
int32_t OSAL_QueueCreate(osal_id_t *queue_id,
                         const char *queue_name,
                         uint32_t queue_depth,
                         uint32_t data_size,
                         uint32_t flags);
```

**参数**：
- `queue_id` - [输出] 队列ID
- `queue_name` - 队列名称（最大20字符）
- `queue_depth` - 队列深度（最大消息数）
- `data_size` - 单个消息大小（字节）
- `flags` - 标志位（保留，传0）

**返回值**：
- `OS_SUCCESS` - 成功
- `OS_INVALID_POINTER` - 无效指针
- `OS_ERR_NAME_TOO_LONG` - 名称过长
- `OS_QUEUE_INVALID_SIZE` - 无效大小
- `OS_ERR_NO_FREE_IDS` - 无可用队列槽
- `OS_ERR_NAME_TAKEN` - 名称已存在
- `OS_ERR_NO_MEMORY` - 内存不足

**限制**：
- `queue_depth` ≤ 10000
- `data_size` ≤ 65536

**示例**：
```c
osal_id_t queue_id;
int32_t ret = OSAL_QueueCreate(&queue_id, "MsgQueue", 10, 64, 0);
if (ret != OS_SUCCESS) {
    LOG_ERROR("Main", "创建队列失败: %d", ret);
}
```

#### OSAL_QueueDelete

删除消息队列。

```c
int32_t OSAL_QueueDelete(osal_id_t queue_id);
```

**参数**：
- `queue_id` - 队列ID

**返回值**：
- `OS_SUCCESS` - 成功
- `OS_ERR_INVALID_ID` - 无效队列ID

**注意**：
- 删除会唤醒所有等待线程
- 使用引用计数，最后一个使用者释放资源

#### OSAL_QueuePut

向队列发送消息。

```c
int32_t OSAL_QueuePut(osal_id_t queue_id,
                      const void *data,
                      uint32_t size,
                      uint32_t timeout);
```

**参数**：
- `queue_id` - 队列ID
- `data` - 消息数据
- `size` - 消息大小（必须≤创建时的data_size）
- `timeout` - 超时时间（毫秒），0表示非阻塞，OS_PEND表示永久等待

**返回值**：
- `OS_SUCCESS` - 成功
- `OS_INVALID_POINTER` - 无效指针
- `OS_ERR_INVALID_ID` - 无效队列ID
- `OS_QUEUE_INVALID_SIZE` - 消息过大
- `OS_QUEUE_FULL` - 队列已满（非阻塞模式）
- `OS_QUEUE_TIMEOUT` - 超时

**示例**：
```c
char msg[64] = "Hello";
int32_t ret = OSAL_QueuePut(queue_id, msg, sizeof(msg), 1000);
if (ret == OS_QUEUE_TIMEOUT) {
    LOG_WARN("Worker", "队列发送超时");
}
```

#### OSAL_QueueGet

从队列接收消息。

```c
int32_t OSAL_QueueGet(osal_id_t queue_id,
                      void *data,
                      uint32_t size,
                      uint32_t *size_copied,
                      int32_t timeout);
```

**参数**：
- `queue_id` - 队列ID
- `data` - [输出] 接收缓冲区
- `size` - 缓冲区大小
- `size_copied` - [输出] 实际复制的字节数（可为NULL）
- `timeout` - 超时时间（毫秒），OS_CHECK表示非阻塞，OS_PEND表示永久等待

**返回值**：
- `OS_SUCCESS` - 成功
- `OS_INVALID_POINTER` - 无效指针
- `OS_ERR_INVALID_ID` - 无效队列ID
- `OS_QUEUE_EMPTY` - 队列为空（非阻塞模式）
- `OS_QUEUE_TIMEOUT` - 超时

**示例**：
```c
char msg[64];
uint32_t size;
int32_t ret = OSAL_QueueGet(queue_id, msg, sizeof(msg), &size, 5000);
if (ret == OS_SUCCESS) {
    LOG_INFO("Stats", "接收消息: %s (大小: %u)", msg, size);
}
```

#### OSAL_QueueGetIdByName

根据名称查找队列ID。

```c
int32_t OSAL_QueueGetIdByName(osal_id_t *queue_id, const char *queue_name);
```

**参数**：
- `queue_id` - [输出] 队列ID
- `queue_name` - 队列名称

**返回值**：
- `OS_SUCCESS` - 成功
- `OS_INVALID_POINTER` - 无效指针
- `OS_ERR_NAME_NOT_FOUND` - 未找到队列

---

### 互斥锁

#### OSAL_MutexCreate

创建互斥锁。

```c
int32_t OSAL_MutexCreate(osal_id_t *mutex_id,
                         const char *mutex_name,
                         uint32_t options);
```

**参数**：
- `mutex_id` - [输出] 互斥锁ID
- `mutex_name` - 互斥锁名称（最大20字符）
- `options` - 选项（保留，传0）

**返回值**：
- `OS_SUCCESS` - 成功
- `OS_INVALID_POINTER` - 无效指针
- `OS_ERR_NAME_TOO_LONG` - 名称过长
- `OS_ERR_NO_FREE_IDS` - 无可用互斥锁槽
- `OS_ERR_NAME_TAKEN` - 名称已存在
- `OS_ERROR` - 创建失败

#### OSAL_MutexDelete

删除互斥锁。

```c
int32_t OSAL_MutexDelete(osal_id_t mutex_id);
```

**参数**：
- `mutex_id` - 互斥锁ID

**返回值**：
- `OS_SUCCESS` - 成功
- `OS_ERR_INVALID_ID` - 无效互斥锁ID

#### OSAL_MutexLock

获取互斥锁。

```c
int32_t OSAL_MutexLock(osal_id_t mutex_id, int32_t timeout);
```

**参数**：
- `mutex_id` - 互斥锁ID
- `timeout` - 超时时间（毫秒），OS_PEND表示永久等待

**返回值**：
- `OS_SUCCESS` - 成功
- `OS_ERR_INVALID_ID` - 无效互斥锁ID
- `OS_SEM_TIMEOUT` - 超时（可能死锁）

**注意**：
- 内置5秒死锁检测
- 支持递归锁（同一线程可多次获取）

**示例**：
```c
int32_t ret = OSAL_MutexLock(mutex_id, 1000);
if (ret == OS_SUCCESS) {
    /* 临界区代码 */
    OSAL_MutexUnlock(mutex_id);
} else {
    LOG_ERROR("Worker", "获取锁超时");
}
```

#### OSAL_MutexUnlock

释放互斥锁。

```c
int32_t OSAL_MutexUnlock(osal_id_t mutex_id);
```

**参数**：
- `mutex_id` - 互斥锁ID

**返回值**：
- `OS_SUCCESS` - 成功
- `OS_ERR_INVALID_ID` - 无效互斥锁ID

#### OSAL_MutexGetIdByName

根据名称查找互斥锁ID。

```c
int32_t OSAL_MutexGetIdByName(osal_id_t *mutex_id, const char *mutex_name);
```

**参数**：
- `mutex_id` - [输出] 互斥锁ID
- `mutex_name` - 互斥锁名称

**返回值**：
- `OS_SUCCESS` - 成功
- `OS_INVALID_POINTER` - 无效指针
- `OS_ERR_NAME_NOT_FOUND` - 未找到互斥锁

---

### 信号量

OSAL 提供 POSIX 信号量的简化封装，支持计数信号量和超时等待。

#### OSAL_SemaphoreCreate

创建信号量。

```c
int32_t OSAL_SemaphoreCreate(osal_semaphore_t **sem, uint32_t initial_value);
```

**参数**：
- `sem` - [输出] 信号量句柄指针
- `initial_value` - 初始值（0 - INT32_MAX）

**返回值**：
- `OSAL_SUCCESS` - 成功
- `OSAL_ERR_INVALID_POINTER` - sem 为 NULL
- `OSAL_ERR_INVALID_SEM_VALUE` - initial_value 超出范围
- `OSAL_ERR_GENERIC` - 系统调用失败

**示例**：
```c
osal_semaphore_t *sem = NULL;
int32_t ret = OSAL_SemaphoreCreate(&sem, 1);
if (ret == OSAL_SUCCESS) {
    /* 使用信号量 */
    OSAL_SemaphoreDelete(sem);
}
```

#### OSAL_SemaphoreDelete

销毁信号量。

```c
int32_t OSAL_SemaphoreDelete(osal_semaphore_t *sem);
```

**参数**：
- `sem` - 信号量句柄

**返回值**：
- `OSAL_SUCCESS` - 成功
- `OSAL_ERR_INVALID_POINTER` - sem 为 NULL
- `OSAL_ERR_GENERIC` - 系统调用失败

#### OSAL_SemaphoreWait

等待信号量（阻塞）。

```c
int32_t OSAL_SemaphoreWait(osal_semaphore_t *sem);
```

**参数**：
- `sem` - 信号量句柄

**返回值**：
- `OSAL_SUCCESS` - 成功
- `OSAL_ERR_INVALID_POINTER` - sem 为 NULL
- `OSAL_ERR_GENERIC` - 系统调用失败

**注意**：
- 如果信号量值为 0，线程将阻塞直到其他线程调用 Post
- 成功后信号量值减 1

#### OSAL_SemaphoreTimedWait

等待信号量（超时）。

```c
int32_t OSAL_SemaphoreTimedWait(osal_semaphore_t *sem, uint32_t timeout_ms);
```

**参数**：
- `sem` - 信号量句柄
- `timeout_ms` - 超时时间（毫秒），0 表示非阻塞

**返回值**：
- `OSAL_SUCCESS` - 成功
- `OSAL_ERR_INVALID_POINTER` - sem 为 NULL
- `OSAL_ERR_TIMEOUT` - 超时
- `OSAL_ERR_GENERIC` - 系统调用失败

**示例**：
```c
/* 非阻塞尝试 */
if (OSAL_SemaphoreTimedWait(sem, 0) == OSAL_SUCCESS) {
    /* 获取成功 */
}

/* 超时等待 */
if (OSAL_SemaphoreTimedWait(sem, 1000) == OSAL_ERR_TIMEOUT) {
    LOG_WARN("Worker", "等待信号量超时");
}
```

#### OSAL_SemaphorePost

释放信号量。

```c
int32_t OSAL_SemaphorePost(osal_semaphore_t *sem);
```

**参数**：
- `sem` - 信号量句柄

**返回值**：
- `OSAL_SUCCESS` - 成功
- `OSAL_ERR_INVALID_POINTER` - sem 为 NULL
- `OSAL_ERR_GENERIC` - 系统调用失败

**注意**：
- 信号量值加 1
- 如果有线程在等待，将唤醒一个线程

**生产者-消费者示例**：
```c
/* 生产者 */
void producer(void *arg) {
    osal_semaphore_t *sem = (osal_semaphore_t *)arg;
    
    while (running) {
        produce_data();
        OSAL_SemaphorePost(sem);  /* 通知消费者 */
    }
}

/* 消费者 */
void consumer(void *arg) {
    osal_semaphore_t *sem = (osal_semaphore_t *)arg;
    
    while (running) {
        OSAL_SemaphoreWait(sem);  /* 等待数据 */
        consume_data();
    }
}
```

---

### 条件变量

OSAL 提供 POSIX 条件变量的简化封装，用于线程间的等待/通知机制。

#### OSAL_CondCreate

创建条件变量。

```c
int32_t OSAL_CondCreate(osal_cond_t **cond);
```

**参数**：
- `cond` - [输出] 条件变量句柄指针

**返回值**：
- `OSAL_SUCCESS` - 成功
- `OSAL_ERR_INVALID_POINTER` - cond 为 NULL
- `OSAL_ERR_GENERIC` - 系统调用失败

#### OSAL_CondDelete

销毁条件变量。

```c
int32_t OSAL_CondDelete(osal_cond_t *cond);
```

**参数**：
- `cond` - 条件变量句柄

**返回值**：
- `OSAL_SUCCESS` - 成功
- `OSAL_ERR_INVALID_POINTER` - cond 为 NULL
- `OSAL_ERR_GENERIC` - 系统调用失败

#### OSAL_CondWait

等待条件变量（阻塞）。

```c
int32_t OSAL_CondWait(osal_cond_t *cond, osal_mutex_t *mutex);
```

**参数**：
- `cond` - 条件变量句柄
- `mutex` - 互斥锁句柄（必须已锁定）

**返回值**：
- `OSAL_SUCCESS` - 成功
- `OSAL_ERR_INVALID_POINTER` - cond 或 mutex 为 NULL
- `OSAL_ERR_GENERIC` - 系统调用失败

**注意**：
- 调用前必须持有 mutex
- 函数会原子地释放 mutex 并进入等待状态
- 被唤醒后会重新获取 mutex

#### OSAL_CondTimedWait

等待条件变量（超时）。

```c
int32_t OSAL_CondTimedWait(osal_cond_t *cond, osal_mutex_t *mutex, uint32_t timeout_ms);
```

**参数**：
- `cond` - 条件变量句柄
- `mutex` - 互斥锁句柄（必须已锁定）
- `timeout_ms` - 超时时间（毫秒）

**返回值**：
- `OSAL_SUCCESS` - 成功
- `OSAL_ERR_INVALID_POINTER` - cond 或 mutex 为 NULL
- `OSAL_ERR_TIMEOUT` - 超时
- `OSAL_ERR_GENERIC` - 系统调用失败

#### OSAL_CondSignal

唤醒一个等待线程。

```c
int32_t OSAL_CondSignal(osal_cond_t *cond);
```

**参数**：
- `cond` - 条件变量句柄

**返回值**：
- `OSAL_SUCCESS` - 成功
- `OSAL_ERR_INVALID_POINTER` - cond 为 NULL
- `OSAL_ERR_GENERIC` - 系统调用失败

**注意**：
- 如果有多个线程在等待，只唤醒一个
- 如果没有线程在等待，调用无效果

#### OSAL_CondBroadcast

唤醒所有等待线程。

```c
int32_t OSAL_CondBroadcast(osal_cond_t *cond);
```

**参数**：
- `cond` - 条件变量句柄

**返回值**：
- `OSAL_SUCCESS` - 成功
- `OSAL_ERR_INVALID_POINTER` - cond 为 NULL
- `OSAL_ERR_GENERIC` - 系统调用失败

**注意**：
- 唤醒所有等待该条件变量的线程
- 适用于多个线程等待同一条件的场景

**使用示例**：
```c
/* 共享数据 */
static int32_t data_ready = 0;
static osal_mutex_t *mutex = NULL;
static osal_cond_t *cond = NULL;

/* 生产者线程 */
void producer(void *arg) {
    OSAL_MutexLock(mutex);
    data_ready = 1;
    OSAL_CondSignal(cond);  /* 通知消费者 */
    OSAL_MutexUnlock(mutex);
}

/* 消费者线程 */
void consumer(void *arg) {
    OSAL_MutexLock(mutex);
    while (!data_ready) {
        OSAL_CondWait(cond, mutex);  /* 等待数据就绪 */
    }
    /* 处理数据 */
    data_ready = 0;
    OSAL_MutexUnlock(mutex);
}
```

**最佳实践**：
1. 始终在循环中检查条件（防止虚假唤醒）
2. 条件变量必须与互斥锁配合使用
3. 修改共享状态前必须持有互斥锁
4. Signal 用于单个等待者，Broadcast 用于多个等待者

---

### 原子操作

OSAL提供C11标准原子操作的封装。

```c
#include <stdatomic.h>

/* 原子类型 */
atomic_int counter = 0;
atomic_uint flags = 0;

/* 原子操作 */
atomic_fetch_add(&counter, 1);      /* counter++ */
atomic_fetch_sub(&counter, 1);      /* counter-- */
int value = atomic_load(&counter);  /* 读取 */
atomic_store(&counter, 10);         /* 写入 */
```

---

## SYS模块 - 系统调用

### 时钟

#### OSAL_GetLocalTime

获取本地时间。

```c
int32_t OSAL_GetLocalTime(os_time_t *time_struct);
```

**参数**：
- `time_struct` - [输出] 时间结构

**返回值**：
- `OS_SUCCESS` - 成功
- `OS_INVALID_POINTER` - 无效指针

### 信号处理

#### OSAL_SignalRegister

注册信号处理函数。

```c
int32_t OSAL_SignalRegister(int32_t signal, osal_signal_handler_t handler);
```

**参数**：
- `signal` - 信号类型（OS_SIGNAL_INT, OS_SIGNAL_TERM, OS_SIGNAL_HUP）
- `handler` - 信号处理函数

**返回值**：
- `OS_SUCCESS` - 成功
- `OS_INVALID_POINTER` - 无效指针
- `OS_ERROR` - 注册失败

**示例**：
```c
static void signal_handler(int32_t sig)
{
    if (sig == OS_SIGNAL_INT) {
        LOG_INFO("Main", "收到Ctrl+C信号");
        g_running = false;
    }
}

OSAL_SignalRegister(OS_SIGNAL_INT, signal_handler);
```

#### OSAL_SignalUnregister

注销信号处理函数。

```c
int32_t OSAL_SignalUnregister(int32_t signal);
```

**参数**：
- `signal` - 信号类型

**返回值**：
- `OS_SUCCESS` - 成功
- `OS_ERROR` - 注销失败

### 文件操作

#### OSAL_FileOpen

打开文件。

```c
int32_t OSAL_FileOpen(int32_t *fd, const char *path, int32_t flags, int32_t mode);
```

**参数**：
- `fd` - [输出] 文件描述符
- `path` - 文件路径
- `flags` - 打开标志（OS_READ, OS_WRITE, OS_CREATE等）
- `mode` - 文件权限（八进制，如0644）

**返回值**：
- `OS_SUCCESS` - 成功
- `OS_INVALID_POINTER` - 无效指针
- `OS_ERROR` - 打开失败

#### OSAL_FileClose

关闭文件。

```c
int32_t OSAL_FileClose(int32_t fd);
```

**参数**：
- `fd` - 文件描述符

**返回值**：
- `OS_SUCCESS` - 成功
- `OS_ERROR` - 关闭失败

#### OSAL_FileRead

读取文件。

```c
int32_t OSAL_FileRead(int32_t fd, void *buffer, uint32_t size);
```

**参数**：
- `fd` - 文件描述符
- `buffer` - [输出] 读取缓冲区
- `size` - 读取大小

**返回值**：
- 实际读取的字节数
- `OS_ERROR` - 读取失败

#### OSAL_FileWrite

写入文件。

```c
int32_t OSAL_FileWrite(int32_t fd, const void *buffer, uint32_t size);
```

**参数**：
- `fd` - 文件描述符
- `buffer` - 写入数据
- `size` - 写入大小

**返回值**：
- 实际写入的字节数
- `OS_ERROR` - 写入失败

### Select

#### OSAL_SelectSingle

单文件描述符select。

```c
int32_t OSAL_SelectSingle(int32_t fd, uint32_t timeout_ms);
```

**参数**：
- `fd` - 文件描述符
- `timeout_ms` - 超时时间（毫秒）

**返回值**：
- `OS_SUCCESS` - 有数据可读
- `OS_ERROR_TIMEOUT` - 超时
- `OS_ERROR` - 错误

### 环境变量

#### OSAL_Getenv

获取环境变量。

```c
const char* OSAL_Getenv(const char *name);
```

**参数**：
- `name` - 环境变量名

**返回值**：
- 环境变量值
- `NULL` - 未找到

#### OSAL_Setenv

设置环境变量。

```c
int32_t OSAL_Setenv(const char *name, const char *value, int32_t overwrite);
```

**参数**：
- `name` - 环境变量名
- `value` - 环境变量值
- `overwrite` - 是否覆盖（1=覆盖，0=不覆盖）

**返回值**：
- `OS_SUCCESS` - 成功
- `OS_ERROR` - 失败

---

## NET模块 - 网络

### Socket

#### OSAL_SocketOpen

创建socket。

```c
int32_t OSAL_SocketOpen(int32_t *sock_id, int32_t domain, int32_t type);
```

**参数**：
- `sock_id` - [输出] socket ID
- `domain` - 地址族（OS_SOCKET_DOMAIN_INET, OS_SOCKET_DOMAIN_INET6）
- `type` - socket类型（OS_SOCKET_STREAM, OS_SOCKET_DGRAM）

**返回值**：
- `OS_SUCCESS` - 成功
- `OS_INVALID_POINTER` - 无效指针
- `OS_ERROR` - 创建失败

#### OSAL_SocketClose

关闭socket。

```c
int32_t OSAL_SocketClose(int32_t sock_id);
```

**参数**：
- `sock_id` - socket ID

**返回值**：
- `OS_SUCCESS` - 成功
- `OS_ERROR` - 关闭失败

#### OSAL_SocketBind

绑定socket。

```c
int32_t OSAL_SocketBind(int32_t sock_id, const os_sockaddr_t *addr);
```

**参数**：
- `sock_id` - socket ID
- `addr` - 地址结构

**返回值**：
- `OS_SUCCESS` - 成功
- `OS_ERROR` - 绑定失败

#### OSAL_SocketConnect

连接socket。

```c
int32_t OSAL_SocketConnect(int32_t sock_id, const os_sockaddr_t *addr, int32_t timeout);
```

**参数**：
- `sock_id` - socket ID
- `addr` - 目标地址
- `timeout` - 超时时间（毫秒）

**返回值**：
- `OS_SUCCESS` - 成功
- `OS_ERROR_TIMEOUT` - 超时
- `OS_ERROR` - 连接失败

---

## LIB模块 - 标准库

### 字符串操作

```c
uint32_t OSAL_Strlen(const char *str);
int32_t OSAL_Strcmp(const char *s1, const char *s2);
char* OSAL_Strcpy(char *dest, const char *src);
char* OSAL_Strncpy(char *dest, const char *src, uint32_t n);
int32_t OSAL_Snprintf(char *str, uint32_t size, const char *format, ...);
```

### 内存管理

```c
void* OSAL_Malloc(uint32_t size);
void OSAL_Free(void *ptr);
void* OSAL_Memset(void *s, int32_t c, uint32_t n);
void* OSAL_Memcpy(void *dest, const void *src, uint32_t n);
```

### 错误处理

#### OSAL_GetErrorString

获取错误描述。

```c
const char* OSAL_GetErrorString(int32_t error_code);
```

**参数**：
- `error_code` - 错误码

**返回值**：
- 错误描述字符串

**错误码列表**（共37个）：
```c
OS_SUCCESS              =  0   /* 成功 */
OS_ERROR                = -1   /* 通用错误 */
OS_INVALID_POINTER      = -2   /* 无效指针 */
OS_ERR_INVALID_ID       = -3   /* 无效ID */
OS_ERR_NAME_TOO_LONG    = -4   /* 名称过长 */
OS_ERR_NO_FREE_IDS      = -5   /* 无可用ID */
OS_ERR_NAME_TAKEN       = -6   /* 名称已存在 */
OS_SEM_TIMEOUT          = -7   /* 信号量超时 */
OS_QUEUE_EMPTY          = -8   /* 队列为空 */
OS_QUEUE_FULL           = -9   /* 队列已满 */
OS_QUEUE_TIMEOUT        = -10  /* 队列超时 */
OS_QUEUE_INVALID_SIZE   = -11  /* 队列大小无效 */
OS_ERR_NO_MEMORY        = -12  /* 内存不足 */
/* ... 更多错误码 ... */
```

---

## UTIL模块 - 工具

### 日志系统

#### LOG宏

```c
LOG_DEBUG(module, fmt, ...)   /* DEBUG级别 */
LOG_INFO(module, fmt, ...)    /* INFO级别 */
LOG_WARN(module, fmt, ...)    /* WARN级别 */
LOG_ERROR(module, fmt, ...)   /* ERROR级别 */
LOG_FATAL(module, fmt, ...)   /* FATAL级别 */
```

**参数**：
- `module` - 模块名称（字符串）
- `fmt` - 格式化字符串
- `...` - 可变参数

**示例**：
```c
LOG_INFO("Worker", "任务启动");
LOG_ERROR("Worker", "发送失败: %d", ret);
LOG_WARN("Stats", "队列接收超时");
```

#### OSAL_Printf

简单打印（无格式）。

```c
void OSAL_Printf(const char *format, ...);
```

**参数**：
- `format` - 格式化字符串
- `...` - 可变参数

**示例**：
```c
OSAL_Printf("应用版本: %s\n", APP_VERSION);
```

### 版本信息

#### OS_GetVersionString

获取OSAL版本字符串。

```c
const char* OS_GetVersionString(void);
```

**返回值**：
- 版本字符串（如"OSAL v1.0.0"）

**示例**：
```c
OSAL_Printf("OSAL版本: %s\n", OS_GetVersionString());
```

---

## 常量定义

### 超时常量

```c
#define OS_PEND     -1   /* 永久等待 */
#define OS_CHECK     0   /* 非阻塞检查 */
```

### 资源限制

```c
#define OS_MAX_TASKS        64   /* 最大任务数 */
#define OS_MAX_QUEUES       64   /* 最大队列数 */
#define OS_MAX_MUTEXES      64   /* 最大互斥锁数 */
#define OS_MAX_API_NAME     20   /* 最大名称长度 */
```

### 信号类型

```c
#define OS_SIGNAL_INT   1   /* SIGINT (Ctrl+C) */
#define OS_SIGNAL_TERM  2   /* SIGTERM */
#define OS_SIGNAL_HUP   3   /* SIGHUP */
```

---

## 类型定义

```c
typedef char        char;      /* 字符串类型 */
typedef int8_t      int8;       /* 8位有符号整数 */
typedef uint8_t     uint8;      /* 8位无符号整数 */
typedef int16_t     int16;      /* 16位有符号整数 */
typedef uint16_t    uint16;     /* 16位无符号整数 */
typedef int32_t     int32;      /* 32位有符号整数 */
typedef uint32_t    uint32;     /* 32位无符号整数 */
typedef int64_t     int64;      /* 64位有符号整数 */
typedef uint64_t    uint64;     /* 64位无符号整数 */
typedef bool        bool;       /* 布尔类型 */
typedef uint32_t    osal_id_t;  /* OSAL对象ID */
```

---

## 相关文档

- [架构设计](ARCHITECTURE.md)
- [使用指南](USAGE_GUIDE.md)
- [模块概述](README.md)
