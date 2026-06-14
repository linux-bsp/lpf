#!/bin/bash
# migrate_mutex_struct.sh
# 用于迁移结构体成员中的 Mutex（更智能的版本）

set -e

if [ $# -eq 0 ]; then
    echo "用法: $0 <file1.c> [file2.c ...]"
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

    # 步骤 1: 替换结构体成员声明
    # osal_mutex_t *mutex; -> pthread_mutex_t mutex;
    sed -i 's/osal_mutex_t \*mutex;/pthread_mutex_t mutex;/g' "$file"

    # 步骤 2: 替换 Create (结构体成员)
    # OSAL_MutexCreate(&ctx->mutex) -> OSAL_pthread_mutex_init(&ctx->mutex, NULL)
    sed -i 's/OSAL_MutexCreate(&\([a-zA-Z_][a-zA-Z0-9_]*->\)mutex)/OSAL_pthread_mutex_init(\&\1mutex, NULL)/g' "$file"

    # 步骤 3: 替换 Lock (结构体成员，添加取地址符)
    # OSAL_MutexLock(ctx->mutex) -> OSAL_pthread_mutex_lock(&ctx->mutex)
    sed -i 's/OSAL_MutexLock(\([a-zA-Z_][a-zA-Z0-9_]*->\)mutex)/OSAL_pthread_mutex_lock(\&\1mutex)/g' "$file"

    # 步骤 4: 替换 Unlock (结构体成员，添加取地址符)
    # OSAL_MutexUnlock(ctx->mutex) -> OSAL_pthread_mutex_unlock(&ctx->mutex)
    sed -i 's/OSAL_MutexUnlock(\([a-zA-Z_][a-zA-Z0-9_]*->\)mutex)/OSAL_pthread_mutex_unlock(\&\1mutex)/g' "$file"

    # 步骤 5: 替换 Delete (结构体成员，添加取地址符)
    # OSAL_MutexDelete(ctx->mutex) -> OSAL_pthread_mutex_destroy(&ctx->mutex)
    sed -i 's/OSAL_MutexDelete(\([a-zA-Z_][a-zA-Z0-9_]*->\)mutex)/OSAL_pthread_mutex_destroy(\&\1mutex)/g' "$file"

    # 步骤 6: 替换 TryLock (如果有)
    sed -i 's/OSAL_MutexTryLock(\([a-zA-Z_][a-zA-Z0-9_]*->\)mutex)/OSAL_pthread_mutex_trylock(\&\1mutex)/g' "$file"

    echo "  ✓ 迁移完成: $file (备份: ${file}.bak)"
done

echo ""
echo "迁移完成！请检查修改并测试编译。"
echo "如果没有问题，可以删除 .bak 备份文件。"
