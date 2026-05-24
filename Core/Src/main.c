/**
 ******************************************************************************
 * @file    main.c
 * @brief   Application entry point for the UART-I2C Protocol Bridge.
 *          Performs system configuration, initializes the driver stack, 
 *          and manages the core processing loop with interrupt-safe sleep.
 ******************************************************************************
 */

#include "config.h"
#include "dma.h"
#include "uart.h"
#include "i2c.h"
#include "protocol.h"
#include "stm32f103xb.h"
#include "core_cm3.h"

/* Standard register-level NVIC interrupt wrapper fallbacks for real target compilation */
#ifndef MOCK_STM32_TESTS

/* Global interrupt control macros */
#define __disable_irq()    __asm volatile ("cpsid i" : : : "memory")
#define __enable_irq()     __asm volatile ("cpsie i" : : : "memory")

/**
 * @brief  Vector interrupt handler for USART1.
 *         Invoked when a byte is received.
 */
void USART1_IRQHandler(void) {
    uart_rx_isr();
}

/**
 * @brief  Vector interrupt handler for DMA1 Channel 4.
 *         Invoked when UART Tx DMA block transfer is complete.
 */
void DMA1_Channel4_IRQHandler(void) {
    uart_dma_tx_isr();
}

/**
 * @brief  Vector interrupt handler for DMA1 Channel 6.
 *         Invoked when I2C1 Tx DMA block transfer is complete.
 */
void DMA1_Channel6_IRQHandler(void) {
    i2c_dma_tx_isr();
}

/**
 * @brief  Vector interrupt handler for DMA1 Channel 7.
 *         Invoked when I2C1 Rx DMA block transfer is complete.
 */
void DMA1_Channel7_IRQHandler(void) {
    i2c_dma_rx_isr();
}

#endif /* MOCK_STM32_TESTS */

int main(void) {
    /* 1. Initialize System Peripherals Clock and Low-Level DMA Control */
    dma_init();

    /* 2. Initialize Core Register Drivers */
    i2c_init();
    uart_init();
    
    /* 3. Enable Core Interrupt Channels in NVIC */
    dma_enable_interrupts();

    /* 4. Initialize Core Serial Protocol Parser */
    protocol_init();

    /* Send boot notification query status to verify connection */
    protocol_tx_response(STATUS_SUCCESS, NULL, 0U);

    /* 5. Main Asynchronous Processing Superloop */
    while (1) {
        /* Process all pending bytes from UART Rx buffer */
        protocol_process();

        /**
         * Low-Power Optimization with WFI (Wait For Interrupt)
         * To achieve maximum power savings, we want the Cortex-M3 to enter
         * low-power Sleep mode when there is no data to process.
         * 
         * To avoid race conditions (i.e. checking the buffer, then an interrupt
         * arriving, and then sleeping and missing the wake trigger), we use
         * Cortex-M "Interrupt-Safe Sleep":
         * 
         * 1. Disable interrupts globally (`cpsid i`)
         * 2. Re-evaluate sleep conditions safely
         * 3. Invoke `WFI` (Wait For Interrupt)
         * 
         * On Cortex-M, WFI will wake the core from sleep if an interrupt is pending,
         * even when interrupts are globally disabled!
         * 
         * 4. Re-enable interrupts (`cpsie i`) which executes the pending ISR immediately.
         */
#ifndef MOCK_STM32_TESTS
        __disable_irq();
        if (ring_buffer_is_empty(uart_get_rx_buffer()) && !uart_is_dma_tx_busy() && !i2c_is_dma_busy()) {
            __WFI(); /* Enter Sleep mode */
        }
        __enable_irq();
#else
        /* Simple simulated yield on host test runner */
        __DMB();
        break; /* Break the loop immediately in simulated environment */
#endif
    }

    return 0;
}
