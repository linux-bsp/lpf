#!/bin/bash

# ============================================================================
# EMS 构建脚本 - 经典 CMake 命令
# ============================================================================

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# 默认配置
BUILD_TYPE="Release"
BUILD_DIR="build"
CLEAN=0
TOOLCHAIN=""
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# 帮助信息
show_help() {
    cat << EOF
用法: $0 [选项]

选项:
    -h, --help          显示此帮助信息
    -d, --debug         Debug 模式编译（默认 Release）
    -c, --clean         清理构建目录
    -a, --arch ARCH     交叉编译架构 (arm32/arm64/riscv64)
    -j, --jobs N        并行编译任务数（默认：CPU核心数）
    -o, --output DIR    构建输出目录（默认：build）

示例:
    $0                  # Release 模式编译
    $0 -d               # Debug 模式编译
    $0 -c               # 清理构建
    $0 -a arm32         # ARM32 交叉编译
    $0 -d -o build-dbg  # Debug 模式，输出到 build-dbg

等效的 CMake 命令:
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j\$(nproc)

交叉编译:
    cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/arm32-linux-gnueabihf.cmake
    cmake --build build

详细文档: docs/BUILD_GUIDE.md

EOF
}

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -c|--clean)
            CLEAN=1
            shift
            ;;
        -a|--arch)
            case $2 in
                arm32)
                    TOOLCHAIN="cmake/toolchains/arm32-linux-gnueabihf.cmake"
                    ;;
                arm64)
                    TOOLCHAIN="cmake/toolchains/aarch64-linux-gnu.cmake"
                    ;;
                riscv64)
                    TOOLCHAIN="cmake/toolchains/riscv64-linux-gnu.cmake"
                    ;;
                *)
                    echo -e "${RED}错误: 不支持的架构 '$2'${NC}"
                    echo "支持的架构: arm32, arm64, riscv64"
                    exit 1
                    ;;
            esac
            shift 2
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        -o|--output)
            BUILD_DIR="$2"
            shift 2
            ;;
        *)
            echo -e "${RED}错误: 未知选项 $1${NC}"
            show_help
            exit 1
            ;;
    esac
done

# 清理构建
if [ $CLEAN -eq 1 ]; then
    echo -e "${YELLOW}清理构建目录...${NC}"
    rm -rf "$BUILD_DIR"
    echo -e "${GREEN}清理完成${NC}"
    exit 0
fi

# 构建 CMake 配置命令
CMAKE_ARGS="-B $BUILD_DIR -DCMAKE_BUILD_TYPE=$BUILD_TYPE"
if [ -n "$TOOLCHAIN" ]; then
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN"
fi

# 配置
echo -e "${GREEN}配置构建 (类型: $BUILD_TYPE)...${NC}"
if [ -n "$TOOLCHAIN" ]; then
    echo -e "工具链: ${YELLOW}$TOOLCHAIN${NC}"
fi
cmake $CMAKE_ARGS

# 编译
echo -e "${GREEN}开始编译 (并行任务: $JOBS)...${NC}"
cmake --build "$BUILD_DIR" -j "$JOBS"

# 显示结果
echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}编译完成！${NC}"
echo -e "${GREEN}========================================${NC}"
echo -e "构建类型: ${YELLOW}$BUILD_TYPE${NC}"
echo -e "输出目录: ${YELLOW}$BUILD_DIR${NC}"
echo ""

# 显示可执行文件
if [ -d "$BUILD_DIR/bin" ]; then
    echo -e "可执行文件:"
    ls -lh "$BUILD_DIR/bin" | tail -n +2 | awk '{printf "  \033[0;32m%s\033[0m (%s)\n", $9, $5}'
fi

# 显示库文件
if [ -d "$BUILD_DIR/lib" ]; then
    echo ""
    echo -e "库文件:"
    ls -lh "$BUILD_DIR/lib" | tail -n +2 | awk '{printf "  \033[0;32m%s\033[0m (%s)\n", $9, $5}'
fi
