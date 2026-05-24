/**
 ******************************************************************************
 * @file    ring_buffer.h
 * @brief   Single-Producer Single-Consumer (SPSC) Lock-Free Circular Buffer.
 *          Provides a thread-safe data queue suitable for transferring data 
 *          from high-priority ISRs to main-thread processing loops without
 *          disabling interrupts or using locking primitives.
 ******************************************************************************
 */

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Circular Buffer Capacity (Must be a power of 2) */
#define RING_BUFFER_SIZE    256U
#define RING_BUFFER_MASK    (RING_BUFFER_SIZE - 1U)

typedef struct {
    uint8_t buffer[RING_BUFFER_SIZE];
    volatile uint32_t head;  /* Written only by Producer (ISR) */
    volatile uint32_t tail;  /* Written only by Consumer (Main Thread) */
} RingBuffer;

/**
 * @brief  Initializes the circular buffer.
 * @param  rb: Pointer to RingBuffer structure.
 */
void ring_buffer_init(RingBuffer *rb);

/**
 * @brief  Pushes a byte into the circular buffer.
 *         Called exclusively by the Producer (e.g., UART Rx ISR).
 * @param  rb: Pointer to RingBuffer structure.
 * @param  data: Byte to write.
 * @return true on success, false if the buffer is full (overflow).
 */
bool ring_buffer_push(RingBuffer *rb, uint8_t data);

/**
 * @brief  Pops a byte from the circular buffer.
 *         Called exclusively by the Consumer (e.g., Main Thread).
 * @param  rb: Pointer to RingBuffer structure.
 * @param  data: Pointer where popped byte will be written.
 * @return true on success, false if the buffer is empty.
 */
bool ring_buffer_pop(RingBuffer *rb, uint8_t *data);

/**
 * @brief  Returns the number of elements currently stored in the buffer.
 * @param  rb: Pointer to RingBuffer structure.
 * @return Number of elements.
 */
uint32_t ring_buffer_get_count(const RingBuffer *rb);

/**
 * @brief  Checks if the buffer is completely full.
 * @param  rb: Pointer to RingBuffer structure.
 * @return true if full, false otherwise.
 */
bool ring_buffer_is_full(const RingBuffer *rb);

/**
 * @brief  Checks if the buffer is empty.
 * @param  rb: Pointer to RingBuffer structure.
 * @return true if empty, false otherwise.
 */
bool ring_buffer_is_empty(const RingBuffer *rb);

#ifdef __cplusplus
}
#endif

#endif /* RING_BUFFER_H */
