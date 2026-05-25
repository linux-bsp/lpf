# =============================================================================
# EMS 非递归 Make 构建系统
# =============================================================================
# 版本: 2.0
# 创建日期: 2026-05-25

# 默认目标
.DEFAULT_GOAL := all

# 包含 Kconfig 生成的配置
-include .config
-include include/config/auto.conf

# 包含辅助函数和规则
include scripts/functions.mk
include scripts/rules.mk

# =============================================================================
# 配置依赖（自动生成 autoconf.h）
# =============================================================================

include/config/auto.conf: .config scripts/kconfig/conf
	@$(MAKE) -C scripts/kconfig syncconfig srctree=$(CURDIR)

# 全局目标列表
ALL_TARGETS :=

# =============================================================================
# Core 模块（根据 Kconfig 配置包含）
# =============================================================================

ifeq ($(CONFIG_OSAL),y)
    include core/osal/module.mk
endif

ifeq ($(CONFIG_HAL),y)
    include core/hal/module.mk
endif

ifeq ($(CONFIG_PCL),y)
    include core/pcl/module.mk
endif

ifeq ($(CONFIG_PDL),y)
    include core/pdl/module.mk
endif

ifeq ($(CONFIG_ACL),y)
    include core/acl/module.mk
endif

# =============================================================================
# Products 模块
# =============================================================================

# 暂时为空，阶段 3 会添加

# =============================================================================
# 主目标
# =============================================================================

.PHONY: all
all: include/config/auto.conf $(ALL_TARGETS)
	@echo "  BUILD   EMS $(VERSION)"

# =============================================================================
# Kconfig 目标（保持和递归 Make 相同）
# =============================================================================

menuconfig: scripts/kconfig/mconf
	@$< Kconfig

%_defconfig: configs/%_defconfig scripts/kconfig/conf
	@scripts/kconfig/conf --defconfig=$< Kconfig

olddefconfig: scripts/kconfig/conf
	@$< --olddefconfig Kconfig

scripts/kconfig/conf scripts/kconfig/mconf:
	@$(MAKE) -C scripts/kconfig

# =============================================================================
# 版本信息
# =============================================================================

VERSION := 1.0.0
export VERSION
