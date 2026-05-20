#!/bin/bash
# ============================================================================
# EMS 头文件安装脚本 - 参考 Linux 内核 scripts/headers_install.sh
# ============================================================================

set -e

INSTALL_HDR_PATH="${1:-/usr/local}"
SRCDIR="${2:-.}"

echo "Installing headers to: ${INSTALL_HDR_PATH}/include"

# 创建目标目录
mkdir -p "${INSTALL_HDR_PATH}/include"

# 复制头文件（保持目录结构）
rsync -a --include='*/' --include='*.h' --exclude='*' \
    "${SRCDIR}/include/" "${INSTALL_HDR_PATH}/include/"

# 复制生成的配置头文件
if [ -f "${SRCDIR}/include/generated/autoconf.h" ]; then
    mkdir -p "${INSTALL_HDR_PATH}/include/generated"
    cp "${SRCDIR}/include/generated/autoconf.h" \
       "${INSTALL_HDR_PATH}/include/generated/"
fi

echo "Headers installed successfully."
