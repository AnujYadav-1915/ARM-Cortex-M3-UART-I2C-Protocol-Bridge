# Makefile for ARM Cortex-M3 UART-I2C Protocol Bridge

BUILD_DIR = build
TEST_BIN = $(BUILD_DIR)/bridge_tests

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 -DMOCK_STM32_TESTS
INCLUDES = -ICore/Inc -IDrivers/CMSIS -ITests

SRCS = Core/Src/ring_buffer.c \
       Core/Src/protocol.c \
       Tests/test_runner.c \
       Tests/test_ring_buffer.c \
       Tests/test_protocol.c

.PHONY: all build test clean help

all: build

build:
	@echo "==> Creating build directory..."
	@mkdir -p $(BUILD_DIR)
	@echo "==> Compiling host simulation and unit tests using $(CC)..."
	@$(CC) $(CFLAGS) $(INCLUDES) $(SRCS) -o $(TEST_BIN)
	@echo "==> Build successful! Binary: $(TEST_BIN)"

test: build
	@echo "==> Running host unit tests..."
	@$(TEST_BIN)

clean:
	@echo "==> Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)

help:
	@echo "Available targets:"
	@echo "  build    - Compile host-based simulation and unit tests using GCC"
	@echo "  test     - Compile and run all unit tests on host"
	@echo "  clean    - Remove build artifacts"
	@echo "  help     - Show this help message"
