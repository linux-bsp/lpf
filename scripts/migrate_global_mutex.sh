#!/bin/bash
# 手动迁移全局 Mutex 变量的脚本

file=$1

if [ ! -f "$file" ]; then
    echo "文件不存在: $file"
    exit 1
fi

echo "处理全局变量迁移: $file"

# 备份
cp "$file" "${file}.bak2"

# 1. 替换全局静态变量声明 
# static osal_mutex_t *xxx = NULL; -> static pthread_mutex_t xxx = PTHREAD_MUTEX_INITIALIZER;
sed -i 's/static osal_mutex_t \*\([a-zA-Z_][a-zA-Z0-9_]*\) = NULL;/static pthread_mutex_t \1 = PTHREAD_MUTEX_INITIALIZER;/g' "$file"

# 2. 替换 Create（全局变量）
# OSAL_MutexCreate(&xxx) -> (已静态初始化，删除或注释)
# 这个需要手动处理，因为逻辑可能需要调整

# 3. 替换 Lock/Unlock（全局变量，添加取地址符）
sed -i 's/OSAL_MutexLock(\([a-zA-Z_][a-zA-Z0-9_]*\))/OSAL_pthread_mutex_lock(\&\1)/g' "$file"
sed -i 's/OSAL_MutexUnlock(\([a-zA-Z_][a-zA-Z0-9_]*\))/OSAL_pthread_mutex_unlock(\&\1)/g' "$file"

# 4. 替换 Delete（全局变量）
sed -i 's/OSAL_MutexDelete(\([a-zA-Z_][a-zA-Z0-9_]*\))/OSAL_pthread_mutex_destroy(\&\1)/g' "$file"

# 5. 替换 Create（需要改为 init）
sed -i 's/OSAL_MutexCreate(&\([a-zA-Z_][a-zA-Z0-9_]*\))/OSAL_pthread_mutex_init(\&\1, NULL)/g' "$file"

echo "完成: $file"
