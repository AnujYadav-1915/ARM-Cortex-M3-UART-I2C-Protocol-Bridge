/**
 ******************************************************************************
 * @file    dma.c
 * @brief   Register-Level DMA Controller Driver Implementation.
 ******************************************************************************
 */

#include "dma.h"
#include "stm32f103xb.h"

void dma_init(void) {
    /* Enable the DMA1 clock on AHB bus */
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
}

void dma_enable_interrupts(void) {
#ifndef MOCK_STM32_TESTS
    /* NVIC Interrupt Set-Enable Registers (0xE000E100)
     * DMA1 Channel 4: IRQ 14 (ISER[0] bit 14)
     * DMA1 Channel 6: IRQ 16 (ISER[0] bit 16)
     * DMA1 Channel 7: IRQ 17 (ISER[0] bit 17)
     */
    volatile uint32_t *nvic_iser0 = (volatile uint32_t *)(0xE000E100 + 0x00);
    *nvic_iser0 |= (1U << 14U) | (1U << 16U) | (1U << 17U);
#endif
}
