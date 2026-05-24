/**
 ******************************************************************************
 * @file    uart.h
 * @brief   Register-Level UART Driver Interface.
 *          Handles interrupt-driven reception into a lock-free circular
 *          buffer and asynchronous transmission offloaded to the DMA.
 ******************************************************************************
 */

#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdbool.h>
#include "ring_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initializes USART1 peripheral.
 *         Configures:
 *         - GPIO pins (PA9 TX as AF Push-Pull, PA10 RX as Input Floating)
 *         - Baud rate register based on config settings
 *         - Receiver and Transmitter mode
 *         - Rx Interrupt Enable (RXNEIE)
 *         - NVIC Interrupt priority and enable
 */
void uart_init(void);

/**
 * @brief  Retrieves a pointer to the UART Rx lock-free circular buffer.
 * @return Pointer to RingBuffer.
 */
RingBuffer *uart_get_rx_buffer(void);

/**
 * @brief  Transmits data using DMA (Asynchronous / Non-blocking).
 *         Configures DMA1 Channel 4 to read from the provided data buffer
 *         and write to USART1 DR register, reducing ISR overhead.
 * @param  data: Pointer to the buffer containing data to transmit.
 * @param  length: Number of bytes to transmit.
 */
void uart_transmit_dma(const uint8_t *data, uint16_t length);

/**
 * @brief  Transmits data using CPU polling (Synchronous / Blocking).
 *         Mainly used for tiny payloads or fallback debug.
 * @param  data: Pointer to the buffer.
 * @param  length: Length of data.
 */
void uart_transmit_bytes(const uint8_t *data, uint16_t length);

/**
 * @brief  Queries if the DMA-driven transmission is currently busy.
 * @return true if busy, false if idle/finished.
 */
bool uart_is_dma_tx_busy(void);

/**
 * @brief  UART Receive Interrupt Service Routine logic.
 *         Should be invoked inside USART1_IRQHandler.
 *         Reads the received byte from USART1->DR and pushes it directly 
 *         to the lock-free circular buffer.
 */
void uart_rx_isr(void);

/**
 * @brief  UART DMA Transmit Complete Interrupt Service Routine.
 *         Should be invoked inside DMA1_Channel4_IRQHandler.
 *         Clears DMA flags and marks Tx transmission as finished.
 */
void uart_dma_tx_isr(void);

#ifdef __cplusplus
}
#endif

#endif /* UART_H */
