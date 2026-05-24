cmake_minimum_required(VERSION 3.15)
project(ARM_CortexM3_UART_I2C_Bridge C)

# Set standard C flags
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Define build configuration options
option(BUILD_TESTS "Build host-based unit tests" ON)

# Configure host-based simulation testing or cross-compilation target
if(CMAKE_SYSTEM_NAME STREQUAL "Generic")
    # Cross-compilation settings (Target: ARM Cortex-M3)
    message(STATUS "Cross-compiling for ARM Cortex-M3 Target...")
    
    # Compiler flags for ARM Cortex-M3
    add_compile_options(
        -mcpu=cortex-m3
        -mthumb
        -mfloat-abi=soft
        -Wall
        -Wextra
        -O2
        -g3
        -ffunction-sections
        -fdata-sections
        --specs=nano.specs
    )
    
    # Add target executable/library here if desired
    # For bare-metal, we usually build a static library or an ELF executable
    add_library(m3_bridge_core STATIC
        Core/Src/ring_buffer.c
        Core/Src/uart.c
        Core/Src/dma.c
        Core/Src/i2c.c
        Core/Src/protocol.c
    )
    target_include_directories(m3_bridge_core PUBLIC
        Core/Inc
        Drivers/CMSIS
    )
else()
    # Host-based build configuration for simulation & testing
    message(STATUS "Configuring for Host Build (Simulation & Testing)...")
    
    if(BUILD_TESTS)
        enable_testing()
        add_subdirectory(Tests)
    endif()
endif()
