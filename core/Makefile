# ============================================================================
# 核心模块构建协调器
# ============================================================================

# 模块列表（按依赖顺序）
MODULES := osal hal pcl pdl acl

.PHONY: all $(MODULES)

all: $(MODULES)

# 递归构建每个模块
$(MODULES):
	@echo "Building core/$@..."
	@$(MAKE) -C $(srctree)/core/$@ \
		BUILD_DIR=$(objtree)/core/$@ \
		srctree=$(srctree) \
		objtree=$(objtree)

# 依赖关系（确保编译顺序）
hal: osal
pcl: hal
pdl: pcl
acl: pdl
