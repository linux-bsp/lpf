# EMS Test Suite Makefile
# Simple standalone build system for tests

CC = gcc
CFLAGS = -Wall -Wextra -g -O0 -std=c99
CFLAGS += -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -D_GNU_SOURCE

# Include paths
INCLUDES = -I../core/osal/include
INCLUDES += -I../core/hal/include
INCLUDES += -I../core/pconfig/include
INCLUDES += -I../core/pconfig/include/api
INCLUDES += -I../core/pdl/include
INCLUDES += -I../core/prl/include
INCLUDES += -I../core/aconfig/include
INCLUDES += -I../core/types/include
INCLUDES += -Iinclude

# Library paths
LDFLAGS = -L../_build/products/ccm/core/osal
LDFLAGS += -L../_build/products/ccm/core/hal
LDFLAGS += -L../_build/products/ccm/core/pconfig
LDFLAGS += -L../_build/products/ccm/core/pdl
LDFLAGS += -L../_build/products/ccm/core/prl
LDFLAGS += -L../_build/products/ccm/core/aconfig

# Libraries
LIBS = -losal -lhal -lpconfig -lpdl -lprl -laconfig -lpthread -lrt -lm

# Source files
CORE_SRCS = core/main.c core/test_registry.c core/test_runner.c core/test_menu.c

# Unit test sources
OSAL_TEST_SRCS = $(wildcard unit/osal/*.c)
HAL_TEST_SRCS = $(wildcard unit/hal/*.c)
PDL_TEST_SRCS = $(wildcard unit/pdl/*.c)
PRL_TEST_SRCS = $(wildcard unit/prl/*.c)
PCL_TEST_SRCS = $(wildcard unit/pcl/*.c)
ACONFIG_TEST_SRCS = $(wildcard unit/aconfig/*.c)
PCONFIG_TEST_SRCS = $(wildcard unit/pconfig/*.c)

ALL_TEST_SRCS = $(OSAL_TEST_SRCS) $(HAL_TEST_SRCS) $(PDL_TEST_SRCS) $(PRL_TEST_SRCS) $(PCL_TEST_SRCS) $(ACONFIG_TEST_SRCS) $(PCONFIG_TEST_SRCS)

# Object files
CORE_OBJS = $(CORE_SRCS:.c=.o)
TEST_OBJS = $(ALL_TEST_SRCS:.c=.o)

# Output
TARGET = ems-test

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(CORE_OBJS) $(TEST_OBJS)
	@echo "Linking $@..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
	@echo "Build complete: $@"

%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "Cleaning..."
	rm -f $(CORE_OBJS) $(TEST_OBJS) $(TARGET)
	@echo "Clean complete."

run: $(TARGET)
	@echo "Running tests..."
	./$(TARGET)

help:
	@echo "EMS Test Suite"
	@echo ""
	@echo "Targets:"
	@echo "  all     - Build test executable (default)"
	@echo "  clean   - Remove build artifacts"
	@echo "  run     - Build and run all tests"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  make              # Build tests"
	@echo "  make run          # Build and run tests"
	@echo "  make clean        # Clean build"
