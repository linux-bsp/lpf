#!/bin/bash
# =============================================================================
# 等待压力测试完成并显示结果
# =============================================================================

LOG_FILE="stress_test_300.log"
CHECK_INTERVAL=30  # 每30秒检查一次

echo "等待压力测试完成..."
echo "日志文件: ${LOG_FILE}"
echo ""

while true; do
    # 检查进程是否还在运行
    if ! pgrep -f "stress_test.sh 300" > /dev/null; then
        echo "测试进程已结束"
        break
    fi

    # 获取当前进度
    CURRENT=$(grep -oE "\[[0-9]+/300\]" "${LOG_FILE}" 2>/dev/null | tail -1 | grep -oE "^[0-9]+" || echo "0")
    PASSED=$(grep -c '✓' "${LOG_FILE}" 2>/dev/null || echo "0")
    FAILED=$(grep -c '✗' "${LOG_FILE}" 2>/dev/null || echo "0")

    echo "[$(date '+%H:%M:%S')] 进度: ${CURRENT}/300 | 通过: ${PASSED} | 失败: ${FAILED}"

    sleep ${CHECK_INTERVAL}
done

echo ""
echo "=========================================="
echo "测试完成！正在生成报告..."
echo "=========================================="
echo ""

# 显示最终结果
if grep -q "压力测试完成" "${LOG_FILE}"; then
    grep -A 20 "压力测试完成" "${LOG_FILE}"
else
    echo "警告: 未找到完成标记，测试可能异常终止"
    tail -30 "${LOG_FILE}"
fi

echo ""
echo "完整日志: ${LOG_FILE}"
echo "摘要报告: $(ls -t stress_test_logs/summary_*.log | head -1)"
