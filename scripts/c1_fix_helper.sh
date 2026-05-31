#!/bin/bash

# C-1 第一阶段修复辅助脚本
# 用于自动化检查和验证HAL层pthread到OSAL的迁移

set -e

PROJECT_ROOT="/home/wanguo/EMS"
cd "$PROJECT_ROOT"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "=========================================="
echo "C-1 第一阶段修复辅助工具"
echo "=========================================="
echo ""

# 函数：检查文件中的pthread使用
check_pthread_usage() {
    local file=$1
    local count=$(grep -c "pthread_mutex\|pthread_create" "$file" 2>/dev/null || echo 0)

    if [ $count -eq 0 ]; then
        echo -e "${GREEN}✓${NC} $(basename $file): 无pthread使用"
        return 0
    else
        echo -e "${RED}✗${NC} $(basename $file): 发现 $count 处pthread使用"
        return 1
    fi
}

# 函数：检查文件中的OSAL使用
check_osal_usage() {
    local file=$1
    local count=$(grep -c "OSAL_Mutex" "$file" 2>/dev/null || echo 0)

    if [ $count -gt 0 ]; then
        echo -e "${GREEN}✓${NC} $(basename $file): 使用 $count 处OSAL_Mutex"
        return 0
    else
        echo -e "${YELLOW}!${NC} $(basename $file): 未使用OSAL_Mutex"
        return 1
    fi
}

# 函数：生成修改报告
generate_report() {
    local file=$1
    echo ""
    echo "=== $(basename $file) 修改报告 ==="
    echo ""

    echo "pthread使用情况:"
    grep -n "pthread_mutex\|pthread_create" "$file" 2>/dev/null | head -10 || echo "  无"

    echo ""
    echo "OSAL使用情况:"
    grep -n "OSAL_Mutex" "$file" 2>/dev/null | head -10 || echo "  无"

    echo ""
}

# 主菜单
show_menu() {
    echo ""
    echo "请选择操作:"
    echo "1) 检查所有HAL文件的pthread使用情况"
    echo "2) 检查单个文件的修改状态"
    echo "3) 生成修改前后对比报告"
    echo "4) 验证编译"
    echo "5) 运行测试"
    echo "6) 退出"
    echo ""
    read -p "请输入选项 (1-6): " choice

    case $choice in
        1) check_all_files ;;
        2) check_single_file ;;
        3) generate_comparison ;;
        4) verify_build ;;
        5) run_tests ;;
        6) exit 0 ;;
        *) echo "无效选项"; show_menu ;;
    esac
}

# 检查所有文件
check_all_files() {
    echo ""
    echo "=== 检查所有HAL文件 ==="
    echo ""

    local files=(
        "core/hal/src/linux/hal_serial_linux.c"
        "core/hal/src/linux/hal_i2c_linux.c"
        "core/hal/src/linux/hal_spi_linux.c"
        "core/hal/src/linux/hal_gpio_linux.c"
    )

    local total=0
    local clean=0

    for file in "${files[@]}"; do
        if [ -f "$file" ]; then
            total=$((total + 1))
            if check_pthread_usage "$file"; then
                clean=$((clean + 1))
            fi
        fi
    done

    echo ""
    echo "总结: $clean/$total 个文件已完成迁移"

    show_menu
}

# 检查单个文件
check_single_file() {
    echo ""
    echo "可用文件:"
    echo "1) hal_serial_linux.c"
    echo "2) hal_i2c_linux.c"
    echo "3) hal_spi_linux.c"
    echo "4) hal_gpio_linux.c"
    echo ""
    read -p "请选择文件 (1-4): " file_choice

    case $file_choice in
        1) file="core/hal/src/linux/hal_serial_linux.c" ;;
        2) file="core/hal/src/linux/hal_i2c_linux.c" ;;
        3) file="core/hal/src/linux/hal_spi_linux.c" ;;
        4) file="core/hal/src/linux/hal_gpio_linux.c" ;;
        *) echo "无效选项"; check_single_file; return ;;
    esac

    generate_report "$file"
    check_pthread_usage "$file"
    check_osal_usage "$file"

    show_menu
}

# 生成对比报告
generate_comparison() {
    echo ""
    echo "=== 修改前后对比 ==="
    echo ""

    echo "pthread使用统计:"
    for file in core/hal/src/linux/hal_*.c; do
        if [ -f "$file" ]; then
            count=$(grep -c "pthread_mutex\|pthread_create" "$file" 2>/dev/null || echo 0)
            printf "  %-30s: %3d 处\n" "$(basename $file)" "$count"
        fi
    done

    echo ""
    echo "OSAL使用统计:"
    for file in core/hal/src/linux/hal_*.c; do
        if [ -f "$file" ]; then
            count=$(grep -c "OSAL_Mutex" "$file" 2>/dev/null || echo 0)
            printf "  %-30s: %3d 处\n" "$(basename $file)" "$count"
        fi
    done

    show_menu
}

# 验证编译
verify_build() {
    echo ""
    echo "=== 验证编译 ==="
    echo ""

    echo "开始编译..."
    if python3 build.py build 2>&1 | tail -20; then
        echo -e "${GREEN}✓ 编译成功${NC}"

        echo ""
        echo "检查符号..."
        if [ -f "_build/products/ccm/hal/libhal.so" ]; then
            pthread_count=$(nm _build/products/ccm/hal/libhal.so | grep -c "pthread" || echo 0)
            osal_count=$(nm _build/products/ccm/hal/libhal.so | grep -c "OSAL_Mutex" || echo 0)

            echo "  pthread符号: $pthread_count"
            echo "  OSAL_Mutex符号: $osal_count"

            if [ $pthread_count -eq 0 ]; then
                echo -e "${GREEN}✓ 无pthread符号残留${NC}"
            else
                echo -e "${RED}✗ 仍有pthread符号${NC}"
            fi
        fi
    else
        echo -e "${RED}✗ 编译失败${NC}"
    fi

    show_menu
}

# 运行测试
run_tests() {
    echo ""
    echo "=== 运行测试 ==="
    echo ""

    echo "运行HAL测试..."
    if python3 build.py test -- --filter=hal 2>&1 | tail -20; then
        echo -e "${GREEN}✓ 测试通过${NC}"
    else
        echo -e "${RED}✗ 测试失败${NC}"
    fi

    show_menu
}

# 启动
echo "欢迎使用C-1修复辅助工具"
echo ""
echo "此工具可以帮助你:"
echo "- 检查pthread到OSAL的迁移进度"
echo "- 验证编译和测试"
echo "- 生成修改报告"
echo ""

show_menu
