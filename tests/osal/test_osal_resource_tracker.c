/************************************************************************
 * OSAL资源跟踪器测试
 ************************************************************************/

#include "tests_core.h"
#include "test_assert.h"
#include "test_registry.h"
#include "osal.h"

/* 测试用例1: 任务资源跟踪 - 已移除 osal_task 模块 */
TEST_CASE(test_resource_task_tracking)
{
    TEST_SKIP_IF(true, "osal_task module has been removed");
}

/* 测试用例2: 队列资源跟踪 */
TEST_CASE(test_resource_queue_tracking)
{
    osal_id_t queue_id = 0;
    osal_resource_stats_t stats_before = {0};
    osal_resource_stats_t stats_after = {0};

    /* 获取初始统计 */
    int32_t ret = OSAL_ResourceGetStats(OSAL_RESOURCE_TYPE_QUEUE, &stats_before);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    /* 创建队列 */
    ret = OSAL_QueueCreate(&queue_id, "TrackQueue", 10, sizeof(uint32_t), 0);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    /* 获取创建后统计 */
    ret = OSAL_ResourceGetStats(OSAL_RESOURCE_TYPE_QUEUE, &stats_after);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
    TEST_ASSERT_EQUAL(stats_before.total_created + 1, stats_after.total_created);
    TEST_ASSERT_EQUAL(stats_before.current_count + 1, stats_after.current_count);

    /* 删除队列 */
    ret = OSAL_QueueDelete(queue_id);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    /* 获取删除后统计 */
    osal_resource_stats_t stats_final = {0};
    ret = OSAL_ResourceGetStats(OSAL_RESOURCE_TYPE_QUEUE, &stats_final);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
    TEST_ASSERT_EQUAL(stats_before.total_deleted + 1, stats_final.total_deleted);
}

/* 测试用例3: 互斥锁资源跟踪 */
TEST_CASE(test_resource_mutex_tracking)
{
    osal_id_t mutex_id = 0;
    osal_resource_stats_t stats_before = {0};
    osal_resource_stats_t stats_after = {0};

    /* 获取初始统计 */
    int32_t ret = OSAL_ResourceGetStats(OSAL_RESOURCE_TYPE_MUTEX, &stats_before);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    /* 创建互斥锁 */
    ret = OSAL_MutexCreate(&mutex_id, "TrackMutex", 0);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    /* 获取创建后统计 */
    ret = OSAL_ResourceGetStats(OSAL_RESOURCE_TYPE_MUTEX, &stats_after);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
    TEST_ASSERT_EQUAL(stats_before.total_created + 1, stats_after.total_created);
    TEST_ASSERT_EQUAL(stats_before.current_count + 1, stats_after.current_count);

    /* 删除互斥锁 */
    ret = OSAL_MutexDelete(mutex_id);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    /* 获取删除后统计 */
    osal_resource_stats_t stats_final = {0};
    ret = OSAL_ResourceGetStats(OSAL_RESOURCE_TYPE_MUTEX, &stats_final);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);
    TEST_ASSERT_EQUAL(stats_before.total_deleted + 1, stats_final.total_deleted);
}

/* 测试用例4: 泄漏检测 */
TEST_CASE(test_resource_leak_detection)
{
    osal_id_t queue_id = 0;

    /* 重置统计 */
    OSAL_ResourceResetStats();

    /* 获取初始泄漏数量 */
    uint32_t initial_leak_count = OSAL_ResourceCheckLeaks();

    /* 创建队列但不删除（模拟泄漏） */
    int32_t ret = OSAL_QueueCreate(&queue_id, "LeakyQueue", 10, sizeof(uint32_t), 0);
    TEST_ASSERT_EQUAL(OSAL_SUCCESS, ret);

    /* 检查泄漏，应该比初始值多1 */
    uint32_t leak_count = OSAL_ResourceCheckLeaks();
    TEST_ASSERT_EQUAL(initial_leak_count + 1, leak_count);

    /* 清理 */
    OSAL_QueueDelete(queue_id);

    /* 再次检查，应该恢复到初始值 */
    leak_count = OSAL_ResourceCheckLeaks();
    TEST_ASSERT_EQUAL(initial_leak_count, leak_count);
}

/* 测试用例5: 统计报告 */
TEST_CASE(test_resource_print_report)
{
    /* 重置统计 */
    OSAL_ResourceResetStats();

    /* 创建一些资源 */
    osal_id_t queue_id = 0;
    osal_id_t mutex_id = 0;

    OSAL_QueueCreate(&queue_id, "ReportQueue", 10, sizeof(uint32_t), 0);
    OSAL_MutexCreate(&mutex_id, "ReportMutex", 0);

    /* 打印报告 */
    OSAL_ResourcePrintReport();

    /* 清理 */
    OSAL_QueueDelete(queue_id);
    OSAL_MutexDelete(mutex_id);
}

/* 注册测试套件 - 自动注册 */
TEST_SUITE_BEGIN(osal_resource_tracker, "osal", "OSAL")
    TEST_CASE_REF(test_resource_task_tracking)
    TEST_CASE_REF(test_resource_queue_tracking)
    TEST_CASE_REF(test_resource_mutex_tracking)
    TEST_CASE_REF(test_resource_leak_detection)
    TEST_CASE_REF(test_resource_print_report)
TEST_SUITE_END(osal_resource_tracker, "osal", "OSAL")

