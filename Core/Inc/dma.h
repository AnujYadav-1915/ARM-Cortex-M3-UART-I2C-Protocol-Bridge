/**
 ******************************************************************************
 * @file    dma.h
 * @brief   Register-Level DMA Controller Driver Interface.
 *          Handles DMA controller configuration and interrupt control.
 ******************************************************************************
 */

#ifndef DMA_H
#define DMA_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initializes DMA1 peripheral clock.
 *         Ensures the AHB peripheral clock is supplied to DMA1.
 */
void dma_init(void);

/**
 * @brief  Enables NVIC interrupts for the specific DMA channels.
 *         - Channel 4: USART1 TX
 *         - Channel 6: I2C1 TX
 *         - Channel 7: I2C1 RX
 */
void dma_enable_interrupts(void);

#ifdef __cplusplus
}
#endif

#endif /* DMA_H */
