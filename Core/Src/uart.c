/**
 ******************************************************************************
 * @file    uart.c
 * @brief   Register-Level UART Driver Implementation.
 *          Implements register control for USART1 and DMA1 Channel 4.
 ******************************************************************************
 */

#include "uart.h"
#include "config.h"
#include "stm32f103xb.h"
#include "core_cm3.h"
#include <stddef.h>

/* Global Receive Ring Buffer */
static RingBuffer rx_buffer;

/* DMA TX Status Tracker */
static volatile bool dma_tx_busy = false;

void uart_init(void) {
    /* Initialize UART Receive Ring Buffer */
    ring_buffer_init(&rx_buffer);
    dma_tx_busy = false;

    /* 1. Enable Peripheral Clocks: USART1, GPIOA, and DMA1 */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPAEN;
    RCC->AHBENR  |= RCC_AHBENR_DMA1EN;

    /* 2. Configure GPIO Pins (PA9 - TX, PA10 - RX) */
    /* Clear CNF/MODE for Pin 9 and Pin 10 */
    uint32_t crh = GPIOA->CRH;
    crh &= ~((0xFU << 4U) | (0xFU << 8U));
    
    /* PA9 (TX): Alternate Function Push-Pull, Max speed 50MHz (CNF = 10, MODE = 11 -> 0xB) */
    crh |= (0xBU << 4U);
    
    /* PA10 (RX): Input Floating (CNF = 01, MODE = 00 -> 0x4) */
    crh |= (0x4U << 8U);
    GPIOA->CRH = crh;

    /* 3. Configure Baud Rate for USART1 (115200 @ 72 MHz)
     * Tx/Rx baud = f_CK / (16 * USARTDIV)
     * USARTDIV = 72000000 / (16 * 115200) = 39.0625
     * Divider Mantissa (Integer) = 39 = 0x27
     * Divider Fraction = 0.0625 * 16 = 1 = 0x1
     * BRR = (Mantissa << 4) | Fraction = 0x271
     */
    uint32_t usart_div = SYSTEM_CORE_CLOCK / (16U * USART1_BAUDRATE);
    uint32_t fraction = ((SYSTEM_CORE_CLOCK % (16U * USART1_BAUDRATE)) * 16U) / (16U * USART1_BAUDRATE);
    USART1->BRR = (usart_div << 4U) | (fraction & 0xFU);

    /* 4. Configure USART1 Control Registers
     * CR1: Enable UE (USART), TE (Tx), RE (Rx), and RXNEIE (Rx Interrupt Enable)
     */
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;

    /* Enable DMA Transmitter in USART1 */
    USART1->CR3 |= USART_CR3_DMAT;

    /* 5. Configure DMA1 Channel 4 (USART1 TX DMA Channel)
     * Set peripheral address register to USART1 Data Register (DR)
     */
    DMA1_Channel4->CPAR = (uint32_t)(&USART1->DR);

    /* 6. Enable Interrupts in NVIC for USART1 and DMA1 Channel 4
     * USART1 IRQn = 37. DMA1_Channel4 IRQn = 14.
     * ISER[0] covers IRQs 0 to 31, ISER[1] covers IRQs 32 to 63.
     */
#ifndef MOCK_STM32_TESTS
    /* Real NVIC configuration registers */
    *((volatile uint32_t*)(0xE000E100 + 0x00)) |= (1U << 14U); /* Enable IRQ 14 (DMA1_Channel4) */
    *((volatile uint32_t*)(0xE000E100 + 0x04)) |= (1U << (37U - 32U)); /* Enable IRQ 37 (USART1) */
#endif
}

RingBuffer *uart_get_rx_buffer(void) {
    return &rx_buffer;
}

void uart_transmit_bytes(const uint8_t *data, uint16_t length) {
    if (data == NULL || length == 0) {
        return;
    }
    
    for (uint16_t i = 0; i < length; ++i) {
        /* Wait for TXE (Transmit Data Register Empty) flag to be set */
        while (!(USART1->SR & USART_SR_TXE)) {
            __DMB();
        }
        /* Write byte to DR */
        USART1->DR = data[i];
    }
    
    /* Wait for TC (Transmission Complete) flag */
    while (!(USART1->SR & USART_SR_TC)) {
        __DMB();
    }
}

void uart_transmit_dma(const uint8_t *data, uint16_t length) {
    if (data == NULL || length == 0) {
        return;
    }

    /* Wait for any previous DMA transaction to complete */
    while (dma_tx_busy) {
        __DMB();
    }

    dma_tx_busy = true;

    /* Ensure DMA Channel is disabled before changing settings */
    DMA1_Channel4->CCR &= ~DMA_CCR_EN;

    /* Configure memory address register and data length */
    DMA1_Channel4->CMAR = (uint32_t)data;
    DMA1_Channel4->CNDTR = length;

    /* Clear DMA flags for Channel 4 in IFCR (Interrupt Flag Clear Register) */
    DMA1->IFCR = DMA_IFCR_CGIF4 | DMA_IFCR_CTCIF4;

    /* Configure DMA Control Register (CCR):
     * DIR = 1 (Memory-to-Peripheral)
     * MINC = 1 (Memory Increment)
     * PINC = 0 (Peripheral Increment Disable)
     * TCIE = 1 (Transfer Complete Interrupt Enable)
     * PL = 01 (Medium priority)
     * EN = 1 (Enable Channel)
     */
    DMA1_Channel4->CCR = DMA_CCR_DIR | DMA_CCR_MINC | DMA_CCR_TCIE | (0x1U << DMA_CCR_PL_Pos) | DMA_CCR_EN;
}

bool uart_is_dma_tx_busy(void) {
    return dma_tx_busy;
}

void uart_rx_isr(void) {
    /* Check RXNE (Receive Data Register Not Empty) status flag */
    if (USART1->SR & USART_SR_RXNE) {
        /* Read the received byte (clears RXNE flag) */
        uint8_t rx_data = (uint8_t)(USART1->DR & 0xFFU);
        
        /* Push the byte into our lock-free ring buffer */
        /* If the buffer is full, the byte is silently dropped, maintaining ISR safety */
        (void)ring_buffer_push(&rx_buffer, rx_data);
    }
}

void uart_dma_tx_isr(void) {
    /* Check Transfer Complete interrupt status flag for DMA1 Channel 4 */
    if (DMA1->ISR & DMA_ISR_TCIF4) {
        /* Clear DMA flag */
        DMA1->IFCR = DMA_IFCR_CTCIF4;
        
        /* Disable DMA Channel */
        DMA1_Channel4->CCR &= ~DMA_CCR_EN;
        
        /* Mark TX as free */
        dma_tx_busy = false;
        __DMB();
    }
}
