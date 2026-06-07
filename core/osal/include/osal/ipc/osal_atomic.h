/************************************************************************
 * OSAL Atomic API
 *
 * Lock-free atomic operations for lock-free programming
 ************************************************************************/

#ifndef OSAL_ATOMIC_H
#define OSAL_ATOMIC_H

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
 * 原子类型定义
 *===========================================================================*/

/**
 * @brief 原子无符号32位整数类型
 *
 * 注意：使用 _Atomic 类型限定符确保原子操作的正确性
 * 避免类型转换导致的未定义行为
 */
typedef struct {
	_Atomic uint32_t value;
} osal_atomic_uint32_t;

/**
 * @brief 原子无符号64位整数类型
 *
 * 用于原子时间戳等需要64位精度的场景
 */
typedef struct {
	_Atomic uint64_t value;
} osal_atomic_uint64_t;

/**
 * @brief 原子布尔类型
 *
 * 用于线程间的布尔标志位（如运行状态、停止信号）
 * 内部使用_Atomic uint32_t实现（C11标准不保证_Atomic _Bool可用）
 */
typedef struct {
	_Atomic uint32_t value;  /* 0=false, 1=true */
} osal_atomic_bool_t;

/*===========================================================================
 * 32位原子操作 API
 *===========================================================================*/

/**
 * @brief 初始化原子变量
 * @param atomic 原子变量指针
 * @param value 初始值
 */
void OSAL_AtomicInit(osal_atomic_uint32_t *atomic, uint32_t value);

/**
 * @brief 原子加载（读取）
 * @param atomic 原子变量指针
 * @return 当前值
 */
uint32_t OSAL_AtomicLoad(const osal_atomic_uint32_t *atomic);

/**
 * @brief 原子存储（写入）
 * @param atomic 原子变量指针
 * @param value 要写入的值
 */
void OSAL_AtomicStore(osal_atomic_uint32_t *atomic, uint32_t value);

/**
 * @brief 原子加法（fetch_add）
 * @param atomic 原子变量指针
 * @param value 要增加的值
 * @return 增加前的值
 */
uint32_t OSAL_AtomicFetchAdd(osal_atomic_uint32_t *atomic, uint32_t value);

/**
 * @brief 原子减法（fetch_sub）
 * @param atomic 原子变量指针
 * @param value 要减少的值
 * @return 减少前的值
 */
uint32_t OSAL_AtomicFetchSub(osal_atomic_uint32_t *atomic, uint32_t value);

/**
 * @brief 原子自增（++）
 * @param atomic 原子变量指针
 * @return 自增后的值
 */
uint32_t OSAL_AtomicIncrement(osal_atomic_uint32_t *atomic);

/**
 * @brief 原子自减（--）
 * @param atomic 原子变量指针
 * @return 自减后的值
 */
uint32_t OSAL_AtomicDecrement(osal_atomic_uint32_t *atomic);

/**
 * @brief 原子比较并交换（CAS）
 *
 * 符合标准CAS语义：如果 *atomic == *expected，则设置 *atomic = desired 并返回 true
 * 否则将 *expected 更新为 *atomic 的实际值并返回 false
 *
 * @param atomic 原子变量指针
 * @param expected 期望值指针（输入/输出参数）
 *                 - 输入：期望的当前值
 *                 - 输出：如果CAS失败，返回实际的当前值
 * @param desired 目标值
 * @return true 交换成功，false 交换失败
 *
 * @note 标准用法示例（重试循环）：
 *       uint32_t expected = OSAL_AtomicLoad(&atomic);
 *       do {
 *           uint32_t desired = expected + 1;
 *       } while (!OSAL_AtomicCompareExchange(&atomic, &expected, desired));
 */
bool OSAL_AtomicCompareExchange(osal_atomic_uint32_t *atomic, uint32_t *expected, uint32_t desired);

/*===========================================================================
 * 64位原子操作 API
 *===========================================================================*/

/**
 * @brief 初始化64位原子变量
 * @param atomic 原子变量指针
 * @param value 初始值
 */
void OSAL_AtomicInit64(osal_atomic_uint64_t *atomic, uint64_t value);

/**
 * @brief 64位原子加载（读取）
 * @param atomic 原子变量指针
 * @return 当前值
 */
uint64_t OSAL_AtomicLoad64(const osal_atomic_uint64_t *atomic);

/**
 * @brief 64位原子存储（写入）
 * @param atomic 原子变量指针
 * @param value 要写入的值
 */
void OSAL_AtomicStore64(osal_atomic_uint64_t *atomic, uint64_t value);

/**
 * @brief 64位原子加法（fetch_add）
 * @param atomic 原子变量指针
 * @param value 要增加的值
 * @return 增加前的值
 */
uint64_t OSAL_AtomicFetchAdd64(osal_atomic_uint64_t *atomic, uint64_t value);

/**
 * @brief 64位原子减法（fetch_sub）
 * @param atomic 原子变量指针
 * @param value 要减少的值
 * @return 减少前的值
 */
uint64_t OSAL_AtomicFetchSub64(osal_atomic_uint64_t *atomic, uint64_t value);

/**
 * @brief 64位原子自增（++）
 * @param atomic 原子变量指针
 * @return 自增后的值
 */
uint64_t OSAL_AtomicIncrement64(osal_atomic_uint64_t *atomic);

/**
 * @brief 64位原子自减（--）
 * @param atomic 原子变量指针
 * @return 自减后的值
 */
uint64_t OSAL_AtomicDecrement64(osal_atomic_uint64_t *atomic);

/**
 * @brief 64位原子比较并交换（CAS）
 *
 * 符合标准CAS语义：如果 *atomic == *expected，则设置 *atomic = desired 并返回 true
 * 否则将 *expected 更新为 *atomic 的实际值并返回 false
 *
 * @param atomic 原子变量指针
 * @param expected 期望值指针（输入/输出参数）
 *                 - 输入：期望的当前值
 *                 - 输出：如果CAS失败，返回实际的当前值
 * @param desired 目标值
 * @return true 交换成功，false 交换失败
 */
bool OSAL_AtomicCompareExchange64(osal_atomic_uint64_t *atomic, uint64_t *expected, uint64_t desired);

/*===========================================================================
 * 布尔原子操作
 *===========================================================================*/

/**
 * @brief 初始化布尔原子变量
 * @param atomic 原子变量指针
 * @param value 初始值
 */
void OSAL_AtomicInitBool(osal_atomic_bool_t *atomic, bool value);

/**
 * @brief 加载布尔原子变量
 * @param atomic 原子变量指针
 * @return 当前值
 */
bool OSAL_AtomicLoadBool(const osal_atomic_bool_t *atomic);

/**
 * @brief 存储布尔原子变量
 * @param atomic 原子变量指针
 * @param value 新值
 */
void OSAL_AtomicStoreBool(osal_atomic_bool_t *atomic, bool value);

/**
 * @brief 布尔原子比较并交换（CAS）
 *
 * 如果 *atomic == *expected，则设置 *atomic = desired 并返回 true
 * 否则将 *expected 更新为 *atomic 的实际值并返回 false
 *
 * @param atomic 原子变量指针
 * @param expected 期望值指针（输入/输出参数）
 * @param desired 目标值
 * @return true 交换成功，false 交换失败
 */
bool OSAL_AtomicCompareExchangeBool(osal_atomic_bool_t *atomic, bool *expected, bool desired);

#ifdef __cplusplus
}
#endif

#endif /* OSAL_ATOMIC_H */
