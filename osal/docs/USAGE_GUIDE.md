# OSAL 使用指南

本文档提供OSAL的实用示例和最佳实践。

## 目录

- [快速开始](#快速开始)
- [任务管理](#任务管理)
- [消息队列](#消息队列)
- [互斥锁](#互斥锁)
- [信号量](#信号量)
- [条件变量](#条件变量)
- [信号处理](#信号处理)
- [日志系统](#日志系统)
- [错误处理](#错误处理)
- [最佳实践](#最佳实践)
- [常见问题](#常见问题)

---

## 快速开始

### 最小示例

```c
#include "osal.h"

int main(void)
{
    /* OSAL作为用户态库，无需显式初始化 */
    
    LOG_INFO("Main", "应用启动");
    
    /* 使用OSAL接口 */
    OSAL_TaskDelay(1000);
    
    LOG_INFO("Main", "应用退出");
    return 0;
}
```

### 编译链接

```bash
# CMakeLists.txt
target_link_libraries(my_app osal)
```

---

## 任务管理

### 创建和删除任务

```c
#include "osal.h"

/* 任务入口函数 */
static void worker_task(void *arg)
{
    uint32_t counter = 0;
    
    LOG_INFO("Worker", "任务启动");
    
    /* 任务循环 - 必须检查退出标志 */
    while (!OSAL_TaskShouldShutdown())
    {
        LOG_INFO("Worker", "工作中: %u", counter++);
        OSAL_TaskDelay(1000);  /* 延时1秒 */
    }
    
    LOG_INFO("Worker", "任务退出");
}

int main(void)
{
    osal_id_t task_id;
    int32_t ret;
    
    /* 创建任务 */
    ret = OSAL_TaskCreate(&task_id, "WorkerTask",
                          worker_task, NULL,
                          16*1024,  /* 栈大小: 16KB */
                          100,      /* 优先级: 100 */
                          0);       /* 标志: 0 */
    if (ret != OS_SUCCESS) {
        LOG_ERROR("Main", "创建任务失败: %d", ret);
        return -1;
    }
    
    LOG_INFO("Main", "任务创建成功");
    
    /* 等待5秒 */
    OSAL_TaskDelay(5000);
    
    /* 删除任务 */
    ret = OSAL_TaskDelete(task_id);
    if (ret == OS_SUCCESS) {
        LOG_INFO("Main", "任务已删除");
    }
    
    return 0;
}
```

### 任务间传递参数

```c
typedef struct {
    uint32_t interval_ms;
    const char *name;
} task_config_t;

static void configurable_task(void *arg)
{
    task_config_t *config = (task_config_t *)arg;
    
    LOG_INFO(config->name, "任务启动，间隔: %u ms", config->interval_ms);
    
    while (!OSAL_TaskShouldShutdown())
    {
        /* 使用配置参数 */
        OSAL_TaskDelay(config->interval_ms);
    }
}

int main(void)
{
    task_config_t config = {
        .interval_ms = 500,
        .name = "FastTask"
    };
    
    osal_id_t task_id;
    OSAL_TaskCreate(&task_id, "ConfigTask",
                    configurable_task, &config,
                    16*1024, 100, 0);
    
    /* 注意: config必须在任务生命周期内有效 */
    OSAL_TaskDelay(5000);
    OSAL_TaskDelete(task_id);
    
    return 0;
}
```

---

## 消息队列

### 基本队列通信

```c
#include "osal.h"

#define QUEUE_DEPTH     10
#define MSG_SIZE        64

static osal_id_t g_queue_id;

/* 生产者任务 */
static void producer_task(void *arg)
{
    uint32_t counter = 0;
    
    while (!OSAL_TaskShouldShutdown())
    {
        char msg[MSG_SIZE];
        OSAL_Snprintf(msg, sizeof(msg), "Message #%u", counter++);
        
        /* 发送消息（超时1秒） */
        int32_t ret = OSAL_QueuePut(g_queue_id, msg, sizeof(msg), 1000);
        if (ret == OS_SUCCESS) {
            LOG_INFO("Producer", "发送: %s", msg);
        } else if (ret == OS_QUEUE_TIMEOUT) {
            LOG_WARN("Producer", "队列发送超时");
        }
        
        OSAL_TaskDelay(500);
    }
}

/* 消费者任务 */
static void consumer_task(void *arg)
{
    while (!OSAL_TaskShouldShutdown())
    {
        char msg[MSG_SIZE];
        uint32_t size;
        
        /* 接收消息（超时2秒） */
        int32_t ret = OSAL_QueueGet(g_queue_id, msg, sizeof(msg), &size, 2000);
        if (ret == OS_SUCCESS) {
            LOG_INFO("Consumer", "接收: %s (大小: %u)", msg, size);
        } else if (ret == OS_QUEUE_TIMEOUT) {
            LOG_WARN("Consumer", "队列接收超时");
        }
    }
}

int main(void)
{
    osal_id_t producer_id, consumer_id;
    
    /* 创建队列 */
    OSAL_QueueCreate(&g_queue_id, "MsgQueue", QUEUE_DEPTH, MSG_SIZE, 0);
    
    /* 创建生产者和消费者任务 */
    OSAL_TaskCreate(&producer_id, "Producer", producer_task, NULL, 16*1024, 100, 0);
    OSAL_TaskCreate(&consumer_id, "Consumer", consumer_task, NULL, 16*1024, 100, 0);
    
    /* 运行10秒 */
    OSAL_TaskDelay(10000);
    
    /* 清理 */
    OSAL_TaskDelete(producer_id);
    OSAL_TaskDelete(consumer_id);
    OSAL_QueueDelete(g_queue_id);
    
    return 0;
}
```

### 非阻塞队列操作

```c
/* 非阻塞发送 */
int32_t ret = OSAL_QueuePut(queue_id, data, size, 0);
if (ret == OS_QUEUE_FULL) {
    LOG_WARN("Worker", "队列已满，丢弃消息");
}

/* 非阻塞接收 */
ret = OSAL_QueueGet(queue_id, data, size, &copied, OS_CHECK);
if (ret == OS_QUEUE_EMPTY) {
    LOG_DEBUG("Worker", "队列为空");
}
```

---

## 互斥锁

### 保护共享资源

```c
#include "osal.h"

static osal_id_t g_mutex_id;
static uint32_t g_shared_counter = 0;

static void increment_task(void *arg)
{
    while (!OSAL_TaskShouldShutdown())
    {
        /* 获取锁 */
        int32_t ret = OSAL_MutexLock(g_mutex_id, 1000);
        if (ret == OS_SUCCESS) {
            /* 临界区 */
            g_shared_counter++;
            LOG_INFO("Task", "计数器: %u", g_shared_counter);
            
            /* 释放锁 */
            OSAL_MutexUnlock(g_mutex_id);
        } else {
            LOG_ERROR("Task", "获取锁超时");
        }
        
        OSAL_TaskDelay(100);
    }
}

int main(void)
{
    osal_id_t task1_id, task2_id;
    
    /* 创建互斥锁 */
    OSAL_MutexCreate(&g_mutex_id, "CounterMutex", 0);
    
    /* 创建两个任务同时访问共享资源 */
    OSAL_TaskCreate(&task1_id, "Task1", increment_task, NULL, 16*1024, 100, 0);
    OSAL_TaskCreate(&task2_id, "Task2", increment_task, NULL, 16*1024, 100, 0);
    
    OSAL_TaskDelay(5000);
    
    /* 清理 */
    OSAL_TaskDelete(task1_id);
    OSAL_TaskDelete(task2_id);
    OSAL_MutexDelete(g_mutex_id);
    
    LOG_INFO("Main", "最终计数: %u", g_shared_counter);
    
    return 0;
}
```

### 避免死锁

```c
/* 错误示例 - 可能死锁 */
void bad_example(void)
{
    OSAL_MutexLock(mutex1, OS_PEND);  /* 永久等待 */
    OSAL_MutexLock(mutex2, OS_PEND);  /* 如果mutex2被占用，死锁 */
    /* ... */
    OSAL_MutexUnlock(mutex2);
    OSAL_MutexUnlock(mutex1);
}

/* 正确示例 - 使用超时 */
void good_example(void)
{
    int32_t ret1 = OSAL_MutexLock(mutex1, 1000);
    if (ret1 != OS_SUCCESS) {
        LOG_ERROR("Worker", "获取mutex1超时");
        return;
    }
    
    int32_t ret2 = OSAL_MutexLock(mutex2, 1000);
    if (ret2 != OS_SUCCESS) {
        LOG_ERROR("Worker", "获取mutex2超时");
        OSAL_MutexUnlock(mutex1);  /* 释放已获取的锁 */
        return;
    }
    
    /* 临界区 */
    
    OSAL_MutexUnlock(mutex2);
    OSAL_MutexUnlock(mutex1);
}
```

---

## 信号量

信号量用于资源计数和线程同步，适合生产者-消费者模式和资源池管理。

### 生产者-消费者模式

```c
#include "osal.h"
#include <pthread.h>

#define BUFFER_SIZE 10

static osal_semaphore_t *empty_sem = NULL;  /* 空槽位计数 */
static osal_semaphore_t *full_sem = NULL;   /* 数据计数 */
static osal_mutex_t *buffer_mutex = NULL;   /* 缓冲区保护 */

static int32_t buffer[BUFFER_SIZE];
static uint32_t in_idx = 0;
static uint32_t out_idx = 0;
static volatile bool running = true;

/* 生产者线程 */
static void *producer_thread(void *arg)
{
    uint32_t data = 0;
    
    while (running) {
        /* 等待空槽位 */
        if (OSAL_SemaphoreWait(empty_sem) != OSAL_SUCCESS) {
            break;
        }
        
        /* 获取缓冲区锁 */
        OSAL_MutexLock(buffer_mutex);
        
        /* 生产数据 */
        buffer[in_idx] = data;
        LOG_INFO("Producer", "生产数据: %u (位置: %u)", data, in_idx);
        in_idx = (in_idx + 1) % BUFFER_SIZE;
        data++;
        
        /* 释放缓冲区锁 */
        OSAL_MutexUnlock(buffer_mutex);
        
        /* 增加数据计数 */
        OSAL_SemaphorePost(full_sem);
        
        OSAL_Sleep(100);  /* 模拟生产延时 */
    }
    
    return NULL;
}

/* 消费者线程 */
static void *consumer_thread(void *arg)
{
    while (running) {
        /* 等待数据 */
        if (OSAL_SemaphoreTimedWait(full_sem, 1000) != OSAL_SUCCESS) {
            continue;  /* 超时，继续等待 */
        }
        
        /* 获取缓冲区锁 */
        OSAL_MutexLock(buffer_mutex);
        
        /* 消费数据 */
        int32_t data = buffer[out_idx];
        LOG_INFO("Consumer", "消费数据: %d (位置: %u)", data, out_idx);
        out_idx = (out_idx + 1) % BUFFER_SIZE;
        
        /* 释放缓冲区锁 */
        OSAL_MutexUnlock(buffer_mutex);
        
        /* 增加空槽位计数 */
        OSAL_SemaphorePost(empty_sem);
        
        OSAL_Sleep(150);  /* 模拟消费延时 */
    }
    
    return NULL;
}

int main(void)
{
    pthread_t producer, consumer;
    
    /* 创建信号量 */
    OSAL_SemaphoreCreate(&empty_sem, BUFFER_SIZE);  /* 初始有 BUFFER_SIZE 个空槽位 */
    OSAL_SemaphoreCreate(&full_sem, 0);             /* 初始无数据 */
    OSAL_MutexCreate(&buffer_mutex);
    
    /* 创建线程 */
    pthread_create(&producer, NULL, producer_thread, NULL);
    pthread_create(&consumer, NULL, consumer_thread, NULL);
    
    LOG_INFO("Main", "生产者-消费者启动，按 Ctrl+C 退出");
    
    /* 运行 10 秒 */
    OSAL_Sleep(10000);
    
    /* 停止线程 */
    running = false;
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);
    
    /* 清理资源 */
    OSAL_SemaphoreDelete(empty_sem);
    OSAL_SemaphoreDelete(full_sem);
    OSAL_MutexDelete(buffer_mutex);
    
    LOG_INFO("Main", "程序退出");
    return 0;
}
```

### 资源池管理

```c
#include "osal.h"

#define MAX_CONNECTIONS 5

static osal_semaphore_t *conn_sem = NULL;

/* 获取连接 */
static int32_t acquire_connection(void)
{
    LOG_INFO("Worker", "等待连接...");
    
    /* 尝试获取连接（超时 3 秒） */
    int32_t ret = OSAL_SemaphoreTimedWait(conn_sem, 3000);
    if (ret == OSAL_SUCCESS) {
        LOG_INFO("Worker", "获取连接成功");
        return 0;
    } else if (ret == OSAL_ERR_TIMEOUT) {
        LOG_WARN("Worker", "连接池已满，获取超时");
        return -1;
    } else {
        LOG_ERROR("Worker", "获取连接失败: %d", ret);
        return -1;
    }
}

/* 释放连接 */
static void release_connection(void)
{
    OSAL_SemaphorePost(conn_sem);
    LOG_INFO("Worker", "释放连接");
}

/* 工作线程 */
static void *worker_thread(void *arg)
{
    int32_t worker_id = *(int32_t *)arg;
    
    for (int i = 0; i < 3; i++) {
        /* 获取连接 */
        if (acquire_connection() == 0) {
            /* 使用连接执行任务 */
            LOG_INFO("Worker%d", "执行任务 %d", worker_id, i + 1);
            OSAL_Sleep(1000);  /* 模拟任务执行 */
            
            /* 释放连接 */
            release_connection();
        }
        
        OSAL_Sleep(500);
    }
    
    return NULL;
}

int main(void)
{
    pthread_t workers[10];
    int32_t worker_ids[10];
    
    /* 创建信号量（最多 5 个并发连接） */
    OSAL_SemaphoreCreate(&conn_sem, MAX_CONNECTIONS);
    
    LOG_INFO("Main", "连接池大小: %d", MAX_CONNECTIONS);
    
    /* 创建 10 个工作线程竞争 5 个连接 */
    for (int i = 0; i < 10; i++) {
        worker_ids[i] = i + 1;
        pthread_create(&workers[i], NULL, worker_thread, &worker_ids[i]);
    }
    
    /* 等待所有线程完成 */
    for (int i = 0; i < 10; i++) {
        pthread_join(workers[i], NULL);
    }
    
    /* 清理 */
    OSAL_SemaphoreDelete(conn_sem);
    
    LOG_INFO("Main", "所有任务完成");
    return 0;
}
```

### 非阻塞尝试

```c
/* 非阻塞尝试获取信号量 */
int32_t ret = OSAL_SemaphoreTimedWait(sem, 0);
if (ret == OSAL_SUCCESS) {
    /* 获取成功，执行任务 */
    do_work();
    OSAL_SemaphorePost(sem);
} else {
    /* 资源不可用，跳过或稍后重试 */
    LOG_DEBUG("Worker", "资源忙，跳过");
}
```

---

## 条件变量

条件变量用于线程间的等待/通知机制，适合事件驱动和状态同步场景。

### 任务队列

```c
#include "osal.h"
#include <pthread.h>

#define MAX_TASKS 100

typedef struct {
    void (*func)(void *);
    void *arg;
} task_t;

static task_t task_queue[MAX_TASKS];
static uint32_t task_count = 0;
static osal_mutex_t *queue_mutex = NULL;
static osal_cond_t *queue_cond = NULL;
static volatile bool shutdown = false;

/* 添加任务 */
static void add_task(void (*func)(void *), void *arg)
{
    OSAL_MutexLock(queue_mutex);
    
    if (task_count < MAX_TASKS) {
        task_queue[task_count].func = func;
        task_queue[task_count].arg = arg;
        task_count++;
        
        LOG_INFO("Main", "添加任务，队列长度: %u", task_count);
        
        /* 通知工作线程 */
        OSAL_CondSignal(queue_cond);
    } else {
        LOG_WARN("Main", "任务队列已满");
    }
    
    OSAL_MutexUnlock(queue_mutex);
}

/* 工作线程 */
static void *worker_thread(void *arg)
{
    int32_t worker_id = *(int32_t *)arg;
    
    while (1) {
        OSAL_MutexLock(queue_mutex);
        
        /* 等待任务（使用循环防止虚假唤醒） */
        while (task_count == 0 && !shutdown) {
            LOG_DEBUG("Worker%d", "等待任务...", worker_id);
            OSAL_CondWait(queue_cond, queue_mutex);
        }
        
        /* 检查是否需要退出 */
        if (shutdown && task_count == 0) {
            OSAL_MutexUnlock(queue_mutex);
            break;
        }
        
        /* 取出任务 */
        task_t task = task_queue[0];
        for (uint32_t i = 0; i < task_count - 1; i++) {
            task_queue[i] = task_queue[i + 1];
        }
        task_count--;
        
        LOG_INFO("Worker%d", "执行任务，剩余: %u", worker_id, task_count);
        
        OSAL_MutexUnlock(queue_mutex);
        
        /* 执行任务（在锁外执行） */
        task.func(task.arg);
    }
    
    LOG_INFO("Worker%d", "线程退出", worker_id);
    return NULL;
}

/* 示例任务函数 */
static void example_task(void *arg)
{
    int32_t task_id = *(int32_t *)arg;
    LOG_INFO("Task", "执行任务 %d", task_id);
    OSAL_Sleep(500);  /* 模拟任务执行 */
}

int main(void)
{
    pthread_t workers[3];
    int32_t worker_ids[3];
    int32_t task_ids[10];
    
    /* 创建同步原语 */
    OSAL_MutexCreate(&queue_mutex);
    OSAL_CondCreate(&queue_cond);
    
    /* 创建工作线程 */
    for (int i = 0; i < 3; i++) {
        worker_ids[i] = i + 1;
        pthread_create(&workers[i], NULL, worker_thread, &worker_ids[i]);
    }
    
    LOG_INFO("Main", "任务队列启动，3 个工作线程");
    
    /* 添加任务 */
    for (int i = 0; i < 10; i++) {
        task_ids[i] = i + 1;
        add_task(example_task, &task_ids[i]);
        OSAL_Sleep(200);
    }
    
    /* 等待所有任务完成 */
    OSAL_Sleep(2000);
    
    /* 通知线程退出 */
    OSAL_MutexLock(queue_mutex);
    shutdown = true;
    OSAL_CondBroadcast(queue_cond);  /* 唤醒所有工作线程 */
    OSAL_MutexUnlock(queue_mutex);
    
    /* 等待线程退出 */
    for (int i = 0; i < 3; i++) {
        pthread_join(workers[i], NULL);
    }
    
    /* 清理 */
    OSAL_CondDelete(queue_cond);
    OSAL_MutexDelete(queue_mutex);
    
    LOG_INFO("Main", "程序退出");
    return 0;
}
```

### 事件通知

```c
#include "osal.h"

static osal_mutex_t *event_mutex = NULL;
static osal_cond_t *event_cond = NULL;
static volatile bool event_ready = false;
static int32_t event_data = 0;

/* 等待事件 */
static void *waiter_thread(void *arg)
{
    int32_t thread_id = *(int32_t *)arg;
    
    OSAL_MutexLock(event_mutex);
    
    LOG_INFO("Waiter%d", "等待事件...", thread_id);
    
    /* 等待事件（循环检查条件） */
    while (!event_ready) {
        OSAL_CondWait(event_cond, event_mutex);
    }
    
    LOG_INFO("Waiter%d", "收到事件，数据: %d", thread_id, event_data);
    
    OSAL_MutexUnlock(event_mutex);
    
    return NULL;
}

/* 触发事件 */
static void trigger_event(int32_t data)
{
    OSAL_MutexLock(event_mutex);
    
    /* 设置事件数据 */
    event_data = data;
    event_ready = true;
    
    LOG_INFO("Main", "触发事件，数据: %d", data);
    
    /* 唤醒所有等待线程 */
    OSAL_CondBroadcast(event_cond);
    
    OSAL_MutexUnlock(event_mutex);
}

int main(void)
{
    pthread_t waiters[5];
    int32_t thread_ids[5];
    
    /* 创建同步原语 */
    OSAL_MutexCreate(&event_mutex);
    OSAL_CondCreate(&event_cond);
    
    /* 创建等待线程 */
    for (int i = 0; i < 5; i++) {
        thread_ids[i] = i + 1;
        pthread_create(&waiters[i], NULL, waiter_thread, &thread_ids[i]);
    }
    
    LOG_INFO("Main", "5 个线程等待事件");
    
    /* 等待 2 秒后触发事件 */
    OSAL_Sleep(2000);
    trigger_event(42);
    
    /* 等待所有线程完成 */
    for (int i = 0; i < 5; i++) {
        pthread_join(waiters[i], NULL);
    }
    
    /* 清理 */
    OSAL_CondDelete(event_cond);
    OSAL_MutexDelete(event_mutex);
    
    LOG_INFO("Main", "程序退出");
    return 0;
}
```

### 超时等待

```c
/* 等待事件（超时 5 秒） */
OSAL_MutexLock(mutex);

while (!condition_met) {
    int32_t ret = OSAL_CondTimedWait(cond, mutex, 5000);
    if (ret == OSAL_ERR_TIMEOUT) {
        LOG_WARN("Worker", "等待事件超时");
        OSAL_MutexUnlock(mutex);
        return -1;
    }
}

/* 条件满足，继续处理 */
process_event();

OSAL_MutexUnlock(mutex);
```

### 条件变量最佳实践

```c
/* ✅ 正确 - 循环检查条件（防止虚假唤醒） */
OSAL_MutexLock(mutex);
while (!condition) {
    OSAL_CondWait(cond, mutex);
}
/* 处理 */
OSAL_MutexUnlock(mutex);

/* ❌ 错误 - 不检查条件 */
OSAL_MutexLock(mutex);
OSAL_CondWait(cond, mutex);  /* 可能虚假唤醒 */
/* 处理 */
OSAL_MutexUnlock(mutex);

/* ✅ 正确 - 修改状态后通知 */
OSAL_MutexLock(mutex);
condition = true;
OSAL_CondSignal(cond);  /* 先修改状态，再通知 */
OSAL_MutexUnlock(mutex);

/* ❌ 错误 - 通知前未持有锁 */
condition = true;  /* 数据竞争 */
OSAL_CondSignal(cond);
```

---

## 信号处理

### 优雅退出

```c
#include "osal.h"

static volatile bool g_running = true;

/* 信号处理函数 */
static void signal_handler(int32_t sig)
{
    if (sig == OS_SIGNAL_INT || sig == OS_SIGNAL_TERM) {
        LOG_INFO("Main", "收到退出信号");
        g_running = false;
    }
}

int main(void)
{
    /* 注册信号处理 */
    OSAL_SignalRegister(OS_SIGNAL_INT, signal_handler);   /* Ctrl+C */
    OSAL_SignalRegister(OS_SIGNAL_TERM, signal_handler);  /* kill */
    
    LOG_INFO("Main", "应用启动，按Ctrl+C退出");
    
    /* 主循环 */
    while (g_running)
    {
        /* 执行工作 */
        OSAL_TaskDelay(1000);
    }
    
    LOG_INFO("Main", "应用退出");
    return 0;
}
```

---

## 日志系统

### 日志级别

```c
/* DEBUG - 调试信息 */
LOG_DEBUG("Module", "变量值: %d", value);

/* INFO - 一般信息 */
LOG_INFO("Module", "任务启动成功");

/* WARN - 警告信息 */
LOG_WARN("Module", "队列接近满载: %u/%u", count, depth);

/* ERROR - 错误信息 */
LOG_ERROR("Module", "打开文件失败: %s", path);

/* FATAL - 致命错误 */
LOG_FATAL("Module", "内存分配失败，程序退出");
```

### 日志输出格式

```
[2026-04-26 10:30:45.123] [INFO] [Worker] 任务启动成功
[2026-04-26 10:30:46.456] [ERROR] [CAN] 发送失败: -1
```

### 简单打印

```c
/* 不带时间戳和级别的简单打印 */
OSAL_Printf("========================================\n");
OSAL_Printf("  应用版本: %s\n", APP_VERSION);
OSAL_Printf("  OSAL版本: %s\n", OS_GetVersionString());
OSAL_Printf("========================================\n");
```

---

## 错误处理

### 检查返回值

```c
/* 所有OSAL函数都返回int32状态码 */
int32_t ret = OSAL_TaskCreate(&task_id, "MyTask", task_entry, NULL,
                               16*1024, 100, 0);
if (ret != OS_SUCCESS) {
    /* 获取错误描述 */
    const char *err_str = OSAL_GetErrorString(ret);
    LOG_ERROR("Main", "创建任务失败: %s (%d)", err_str, ret);
    return -1;
}
```

### 常见错误码

```c
OS_SUCCESS              /* 成功 */
OS_ERROR                /* 通用错误 */
OS_INVALID_POINTER      /* 无效指针 */
OS_ERR_INVALID_ID       /* 无效ID */
OS_ERR_NAME_TOO_LONG    /* 名称过长 */
OS_ERR_NO_FREE_IDS      /* 无可用ID */
OS_ERR_NAME_TAKEN       /* 名称已存在 */
OS_SEM_TIMEOUT          /* 超时 */
OS_QUEUE_EMPTY          /* 队列为空 */
OS_QUEUE_FULL           /* 队列已满 */
OS_QUEUE_TIMEOUT        /* 队列超时 */
OS_ERR_NO_MEMORY        /* 内存不足 */
```

---

## 最佳实践

### 1. 任务编写规范

```c
/* ✅ 正确 - 检查退出标志 */
static void good_task(void *arg)
{
    while (!OSAL_TaskShouldShutdown())
    {
        /* 执行工作 */
        OSAL_TaskDelay(100);
    }
    /* 清理资源 */
}

/* ❌ 错误 - 无限循环 */
static void bad_task(void *arg)
{
    while (1)  /* 无法优雅退出 */
    {
        /* 执行工作 */
        OSAL_TaskDelay(100);
    }
}
```

### 2. 资源管理

```c
/* ✅ 正确 - 成对创建和删除 */
osal_id_t task_id;
OSAL_TaskCreate(&task_id, "MyTask", task_entry, NULL, 16*1024, 100, 0);
/* 使用任务 */
OSAL_TaskDelete(task_id);  /* 必须删除 */

/* ✅ 正确 - 错误时清理 */
osal_id_t queue_id, task_id;

if (OSAL_QueueCreate(&queue_id, "Queue", 10, 64, 0) != OS_SUCCESS) {
    return -1;
}

if (OSAL_TaskCreate(&task_id, "Task", task_entry, NULL, 16*1024, 100, 0) != OS_SUCCESS) {
    OSAL_QueueDelete(queue_id);  /* 清理已创建的资源 */
    return -1;
}
```

### 3. 使用超时

```c
/* ✅ 正确 - 使用超时避免永久阻塞 */
int32_t ret = OSAL_MutexLock(mutex_id, 1000);  /* 1秒超时 */
if (ret == OS_SEM_TIMEOUT) {
    LOG_ERROR("Worker", "获取锁超时，可能死锁");
    return;
}

/* ❌ 错误 - 永久等待可能导致死锁 */
OSAL_MutexLock(mutex_id, OS_PEND);  /* 危险 */
```

### 4. 日志使用

```c
/* ✅ 正确 - 使用LOG宏 */
LOG_INFO("Module", "任务启动");
LOG_ERROR("Module", "错误: %d", ret);

/* ❌ 错误 - 直接使用printf */
printf("任务启动\n");  /* 不推荐 */
```

### 5. 命名规范

```c
/* ✅ 正确 - 描述性名称 */
OSAL_TaskCreate(&task_id, "WorkerTask", ...);
OSAL_QueueCreate(&queue_id, "MsgQueue", ...);
OSAL_MutexCreate(&mutex_id, "DataMutex", ...);

/* ❌ 错误 - 无意义名称 */
OSAL_TaskCreate(&task_id, "Task1", ...);
OSAL_QueueCreate(&queue_id, "Q", ...);
```

---

## 常见问题

### Q1: 任务无法退出？

**原因**: 任务循环未检查`OSAL_TaskShouldShutdown()`

**解决**:
```c
while (!OSAL_TaskShouldShutdown())  /* 必须检查 */
{
    /* 任务逻辑 */
}
```

### Q2: 队列操作失败？

**原因**: 队列已满或已空

**解决**:
```c
int32_t ret = OSAL_QueuePut(queue_id, data, size, 1000);
if (ret == OS_QUEUE_TIMEOUT) {
    LOG_WARN("Worker", "队列满，消息被丢弃");
}
```

### Q3: 互斥锁超时？

**原因**: 可能死锁或锁持有时间过长

**解决**:
- 检查是否忘记释放锁
- 减少临界区代码
- 检查锁的获取顺序

### Q4: 内存泄漏？

**原因**: 资源未释放

**解决**:
```c
/* 确保所有创建的资源都被删除 */
OSAL_TaskDelete(task_id);
OSAL_QueueDelete(queue_id);
OSAL_MutexDelete(mutex_id);
```

### Q5: 如何调试？

**方法**:
1. 使用LOG_DEBUG输出调试信息
2. 检查返回值和错误码
3. 使用GDB调试
4. 查看日志文件

---

## 完整示例

参考 [sample_app](../../apps/sample_app/README.md) 获取完整的应用示例。

## 相关文档

- [架构设计](ARCHITECTURE.md)
- [API参考](API_REFERENCE.md)
- [模块概述](README.md)
