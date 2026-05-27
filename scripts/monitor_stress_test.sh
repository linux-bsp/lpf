#!/bin/bash
# =============================================================================
# 压力测试监控脚本
# =============================================================================

LOG_FILE="stress_test_300.log"
LOG_DIR="stress_test_logs"

if [ ! -f "${LOG_FILE}" ]; then
    echo "错误: 找不到日志文件 ${LOG_FILE}"
    exit 1
fi

echo "=========================================="
echo "EMS 压力测试监控"
echo "=========================================="
echo ""

# 显示最新的摘要日志
LATEST_SUMMARY=$(ls -t ${LOG_DIR}/summary_*.log 2>/dev/null | head -1)

if [ -n "${LATEST_SUMMARY}" ]; then
    echo "最新摘要日志: ${LATEST_SUMMARY}"
    echo ""
fi

# 实时统计
echo "实时统计:"
echo "----------------------------------------"

# 统计通过的测试
PASSED=$(grep -c "✓" "${LOG_FILE}" 2>/dev/null || echo 0)
echo "通过步骤: ${PASSED}"

# 统计失败的测试
FAILED=$(grep -c "✗" "${LOG_FILE}" 2>/dev/null || echo 0)
echo "失败步骤: ${FAILED}"

# 统计完成的迭代
ITERATIONS=$(grep -c "\[.*/.*/\]" "${LOG_FILE}" 2>/dev/null || echo 0)
echo "完成迭代: ${ITERATIONS}"

# 显示最后几行
echo ""
echo "最新输出:"
echo "----------------------------------------"
tail -20 "${LOG_FILE}"

echo ""
echo "=========================================="
echo "使用 'tail -f ${LOG_FILE}' 查看实时输出"
echo "使用 'watch -n 5 ./scripts/monitor_stress_test.sh' 持续监控"
echo "=========================================="
