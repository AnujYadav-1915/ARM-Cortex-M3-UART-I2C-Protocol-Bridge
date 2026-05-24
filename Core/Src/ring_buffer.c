/**
 ******************************************************************************
 * @file    ring_buffer.c
 * @brief   Single-Producer Single-Consumer (SPSC) Lock-Free Circular Buffer.
 *          Implements ring buffer logic with Cortex-M Data Memory Barriers.
 ******************************************************************************
 */

#include "ring_buffer.h"
#include "core_cm3.h"
#include <stddef.h>

void ring_buffer_init(RingBuffer *rb) {
    if (rb != NULL) {
        rb->head = 0U;
        rb->tail = 0U;
        for (uint32_t i = 0U; i < RING_BUFFER_SIZE; ++i) {
            rb->buffer[i] = 0U;
        }
        __DMB(); /* Ensure initialization is visible across all pipelines */
    }
}

bool ring_buffer_push(RingBuffer *rb, uint8_t data) {
    if (rb == NULL) {
        return false;
    }

    uint32_t current_head = rb->head;
    uint32_t current_tail = rb->tail; /* Read volatile once to prevent race */

    uint32_t next_head = (current_head + 1U) & RING_BUFFER_MASK;

    /* Buffer is full if the next head position meets the tail */
    if (next_head == current_tail) {
        return false;
    }

    /* Write data to the buffer slot */
    rb->buffer[current_head] = data;

    /**
     * Data Memory Barrier (DMB)
     * Ensures that the data is completely written to the buffer array
     * before the head pointer update is committed and becomes visible 
     * to the consumer. This prevents the CPU/compiler from reordering 
     * the write operations.
     */
    __DMB();

    /* Update head index */
    rb->head = next_head;

    return true;
}

bool ring_buffer_pop(RingBuffer *rb, uint8_t *data) {
    if (rb == NULL || data == NULL) {
        return false;
    }

    uint32_t current_head = rb->head; /* Read volatile once to prevent race */
    uint32_t current_tail = rb->tail;

    /* Buffer is empty if head and tail indices are equal */
    if (current_tail == current_head) {
        return false;
    }

    /* Retrieve data from the buffer slot */
    *data = rb->buffer[current_tail];

    /**
     * Data Memory Barrier (DMB)
     * Ensures that the data is fully read from the buffer array
     * before the tail pointer update is committed. This guarantees 
     * that the producer does not overwrite this slot before the consumer
     * has finished reading it.
     */
    __DMB();

    /* Update tail index */
    rb->tail = (current_tail + 1U) & RING_BUFFER_MASK;

    return true;
}

uint32_t ring_buffer_get_count(const RingBuffer *rb) {
    if (rb == NULL) {
        return 0U;
    }
    
    uint32_t head = rb->head;
    uint32_t tail = rb->tail;
    
    if (head >= tail) {
        return head - tail;
    } else {
        return (RING_BUFFER_SIZE - tail) + head;
    }
}

bool ring_buffer_is_full(const RingBuffer *rb) {
    if (rb == NULL) {
        return false;
    }
    return (((rb->head + 1U) & RING_BUFFER_MASK) == rb->tail);
}

bool ring_buffer_is_empty(const RingBuffer *rb) {
    if (rb == NULL) {
        return true;
    }
    return (rb->head == rb->tail);
}
