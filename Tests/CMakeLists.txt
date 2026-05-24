# CMake configuration for building Host-based Unit Tests

add_executable(bridge_tests
    test_runner.c
    test_ring_buffer.c
    test_protocol.c
    ../Core/Src/ring_buffer.c
    ../Core/Src/protocol.c
)

# Set compile definitions for host environment
target_compile_definitions(bridge_tests PRIVATE MOCK_STM32_TESTS)

# Include directories
target_include_directories(bridge_tests PRIVATE
    .
    ../Core/Inc
    ../Drivers/CMSIS
)

# Register test with CMake testing suite
add_test(NAME bridge_unit_tests COMMAND bridge_tests)
