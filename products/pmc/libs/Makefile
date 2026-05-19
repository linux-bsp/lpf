# ============================================================================
# PMC 库层构建
# ============================================================================

# 必须在包含 Makefile.lib 之前定义
GENERATED_DIR ?= $(objtree)/include/generated

include $(srctree)/scripts/Makefile.lib

# 库列表
LIBS := libpmc

# 默认目标
.PHONY: all clean

all:
	@echo "Building PMC libraries"
	@for lib in $(LIBS); do \
		echo "Building $$lib..."; \
		$(MAKE) -C $(srctree)/products/pmc/libs/$$lib \
			srctree=$(srctree) \
			objtree=$(objtree) \
			GENERATED_DIR=$(GENERATED_DIR) || exit 1; \
	done
	@echo "PMC libraries built successfully"

# 清理
clean:
	@echo "Cleaning PMC libraries"
	@for lib in $(LIBS); do \
		$(MAKE) -C $(srctree)/products/pmc/libs/$$lib clean \
			srctree=$(srctree) \
			objtree=$(objtree); \
	done
