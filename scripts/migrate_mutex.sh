#!/bin/bash
# Mutex API 自动迁移脚本
#
# 此脚本将旧的 OSAL Mutex API 迁移到新的 pthread_mutex_t API
#
# 使用方法：
#   ./migrate_mutex.sh <file.c>
#
# 迁移模式：
#   osal_mutex_t *mutex;                 -> pthread_mutex_t mutex;
#   OSAL_MutexCreate(&mutex);            -> OSAL_pthread_mutex_init(&mutex, NULL);
#   OSAL_MutexLock(mutex);               -> OSAL_pthread_mutex_lock(&mutex);
#   OSAL_MutexUnlock(mutex);             -> OSAL_pthread_mutex_unlock(&mutex);
#   OSAL_MutexDelete(mutex);             -> OSAL_pthread_mutex_destroy(&mutex);
#   OSAL_MutexTryLock(mutex);            -> OSAL_pthread_mutex_trylock(&mutex);

set -e

if [ $# -eq 0 ]; then
    echo "用法: $0 <file1.c> [file2.c ...]"
    echo ""
    echo "示例："
    echo "  $0 test_osal_mutex.c"
    echo "  $0 products/tests/unit/osal/*.c"
    exit 1
fi

for file in "$@"; do
    if [ ! -f "$file" ]; then
        echo "警告: 文件不存在: $file"
        continue
    fi

    echo "处理文件: $file"

    # 创建备份
    cp "$file" "${file}.bak"

    # 步骤 1: 替换类型声明
    # osal_mutex_t *mutex; -> pthread_mutex_t mutex;
    sed -i 's/osal_mutex_t \*\([a-zA-Z_][a-zA-Z0-9_]*\);/pthread_mutex_t \1;/g' "$file"

    # 步骤 2: 替换 API 调用
    # OSAL_MutexCreate(&mutex) -> OSAL_pthread_mutex_init(&mutex, NULL)
    sed -i 's/OSAL_MutexCreate(&\([a-zA-Z_][a-zA-Z0-9_]*\))/OSAL_pthread_mutex_init(\&\1, NULL)/g' "$file"

    # OSAL_MutexDelete(mutex) -> OSAL_pthread_mutex_destroy(&mutex)
    sed -i 's/OSAL_MutexDelete(\([a-zA-Z_][a-zA-Z0-9_]*\))/OSAL_pthread_mutex_destroy(\&\1)/g' "$file"

    # OSAL_MutexLock(mutex) -> OSAL_pthread_mutex_lock(&mutex)
    sed -i 's/OSAL_MutexLock(\([a-zA-Z_][a-zA-Z0-9_]*\))/OSAL_pthread_mutex_lock(\&\1)/g' "$file"

    # OSAL_MutexUnlock(mutex) -> OSAL_pthread_mutex_unlock(&mutex)
    sed -i 's/OSAL_MutexUnlock(\([a-zA-Z_][a-zA-Z0-9_]*\))/OSAL_pthread_mutex_unlock(\&\1)/g' "$file"

    # OSAL_MutexTryLock(mutex) -> OSAL_pthread_mutex_trylock(&mutex)
    sed -i 's/OSAL_MutexTryLock(\([a-zA-Z_][a-zA-Z0-9_]*\))/OSAL_pthread_mutex_trylock(\&\1)/g' "$file"

    # OSAL_MutexTimedLock(mutex, timeout) -> OSAL_pthread_mutex_timedlock(&mutex, timeout)
    sed -i 's/OSAL_MutexTimedLock(\([a-zA-Z_][a-zA-Z0-9_]*\),/OSAL_pthread_mutex_timedlock(\&\1,/g' "$file"

    echo "  ✓ 迁移完成: $file (备份: ${file}.bak)"
done

echo ""
echo "迁移完成！请检查修改并测试编译。"
echo "如果没有问题，可以删除 .bak 备份文件。"
