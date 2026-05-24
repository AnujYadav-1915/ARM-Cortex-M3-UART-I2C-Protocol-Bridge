/**
 ******************************************************************************
 * @file    test_ring_buffer.c
 * @brief   Unit Tests for Lock-Free Circular Buffer.
 ******************************************************************************
 */

#include "ring_buffer.h"
#include <stdio.h>
#include <assert.h>

void test_ring_buffer_basic(void) {
    RingBuffer rb;
    ring_buffer_init(&rb);

    printf("[TEST] ring_buffer_basic: Running...\n");

    /* Initial state verification */
    assert(ring_buffer_is_empty(&rb));
    assert(!ring_buffer_is_full(&rb));
    assert(ring_buffer_get_count(&rb) == 0U);

    /* Push elements and check count */
    assert(ring_buffer_push(&rb, 0xA1));
    assert(!ring_buffer_is_empty(&rb));
    assert(ring_buffer_get_count(&rb) == 1U);

    assert(ring_buffer_push(&rb, 0xB2));
    assert(ring_buffer_get_count(&rb) == 2U);

    /* Pop verification (FIFO) */
    uint8_t data = 0;
    assert(ring_buffer_pop(&rb, &data));
    assert(data == 0xA1);
    assert(ring_buffer_get_count(&rb) == 1U);

    assert(ring_buffer_pop(&rb, &data));
    assert(data == 0xB2);
    assert(ring_buffer_is_empty(&rb));

    printf("[TEST] ring_buffer_basic: PASSED\n");
}

void test_ring_buffer_overflow(void) {
    RingBuffer rb;
    ring_buffer_init(&rb);

    printf("[TEST] ring_buffer_overflow: Running...\n");

    /* Capacity is 256. Lock-free SPSC buffer holds N-1 (255) elements.
     * Write 255 items to fill the buffer.
     */
    for (uint32_t i = 0U; i < RING_BUFFER_SIZE - 1U; ++i) {
        assert(ring_buffer_push(&rb, (uint8_t)(i & 0xFFU)));
    }

    assert(ring_buffer_is_full(&rb));
    assert(ring_buffer_get_count(&rb) == RING_BUFFER_SIZE - 1U);

    /* The 256th push should fail */
    assert(!ring_buffer_push(&rb, 0xFF));

    /* Pop all and verify contents */
    uint8_t val;
    for (uint32_t i = 0U; i < RING_BUFFER_SIZE - 1U; ++i) {
        assert(ring_buffer_pop(&rb, &val));
        assert(val == (uint8_t)(i & 0xFFU));
    }

    assert(ring_buffer_is_empty(&rb));
    printf("[TEST] ring_buffer_overflow: PASSED\n");
}

void test_ring_buffer_wrap_around(void) {
    RingBuffer rb;
    ring_buffer_init(&rb);

    printf("[TEST] ring_buffer_wrap_around: Running...\n");

    /* Fill buffer with 100 elements, pop 50, then write 150 more to force a wrap-around */
    for (uint8_t i = 0; i < 100; ++i) {
        assert(ring_buffer_push(&rb, i));
    }
    
    uint8_t val;
    for (uint8_t i = 0; i < 50; ++i) {
        assert(ring_buffer_pop(&rb, &val));
        assert(val == i);
    }
    
    assert(ring_buffer_get_count(&rb) == 50U);

    /* Push 150 more, indices will wrap around */
    for (uint16_t i = 0; i < 150; ++i) {
        assert(ring_buffer_push(&rb, (uint8_t)(i & 0xFF)));
    }

    assert(ring_buffer_get_count(&rb) == 200U);

    /* Pop remaining original elements (50 to 99) */
    for (uint8_t i = 50; i < 100; ++i) {
        assert(ring_buffer_pop(&rb, &val));
        assert(val == i);
    }

    /* Pop wrapped elements (0 to 149) */
    for (uint8_t i = 0; i < 150; ++i) {
        assert(ring_buffer_pop(&rb, &val));
        assert(val == i);
    }

    assert(ring_buffer_is_empty(&rb));
    printf("[TEST] ring_buffer_wrap_around: PASSED\n");
}

void run_ring_buffer_tests(void) {
    test_ring_buffer_basic();
    test_ring_buffer_overflow();
    test_ring_buffer_wrap_around();
}
