# ============================================================================
# 产品层构建
# ============================================================================

# 必须在包含 Makefile.lib 之前定义
GENERATED_DIR ?= $(objtree)/include/generated

include $(srctree)/scripts/Makefile.lib

# 产品列表
PRODUCTS := pmc

# 默认目标
.PHONY: all clean

all:
	@echo "Building products"
	@for product in $(PRODUCTS); do \
		echo "Building $$product..."; \
		$(MAKE) -C $(srctree)/products/$$product \
			srctree=$(srctree) \
			objtree=$(objtree) \
			GENERATED_DIR=$(GENERATED_DIR) || exit 1; \
	done
	@echo "All products built successfully"

# 清理
clean:
	@echo "Cleaning products"
	@for product in $(PRODUCTS); do \
		$(MAKE) -C $(srctree)/products/$$product clean \
			srctree=$(srctree) \
			objtree=$(objtree); \
	done
