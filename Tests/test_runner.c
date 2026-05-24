/**
 ******************************************************************************
 * @file    test_runner.c
 * @brief   Unit Test Runner and Mock Drivers for host-based simulation.
 ******************************************************************************
 */

#include "mock_stm32.h"
#include "uart.h"
#include "i2c.h"
#include "protocol.h"
#include <stdio.h>
#include <string.h>

/* Mock registers initialization */
RCC_TypeDef   mock_RCC;
GPIO_TypeDef  mock_GPIOA;
GPIO_TypeDef  mock_GPIOB;
USART_TypeDef mock_USART1;
I2C_TypeDef   mock_I2C1;
DMA_TypeDef   mock_DMA1;

/* Capture variables for UART DMA Transmits */
uint8_t last_dma_tx_buf[PROTOCOL_MAX_PAYLOAD + 8];
uint16_t last_dma_tx_len = 0;

/* Mock UART Rx Buffer */
static RingBuffer mock_rx_buffer;

/* Simulated DMA States */
static bool mock_i2c_dma_busy_state = false;
static I2C_Status mock_i2c_last_dma_status = I2C_STATUS_SUCCESS;

void mock_reset_peripherals(void) {
    memset(&mock_RCC, 0, sizeof(mock_RCC));
    memset(&mock_GPIOA, 0, sizeof(mock_GPIOA));
    memset(&mock_GPIOB, 0, sizeof(mock_GPIOB));
    memset(&mock_USART1, 0, sizeof(mock_USART1));
    memset(&mock_I2C1, 0, sizeof(mock_I2C1));
    memset(&mock_DMA1, 0, sizeof(mock_DMA1));
    
    mock_i2c_dma_busy_state = false;
    mock_i2c_last_dma_status = I2C_STATUS_SUCCESS;
}

/* =================================================================********* */
/*                         Mock Driver Implementations                        */
/* =================================================================********* */

void uart_init(void) {
    ring_buffer_init(&mock_rx_buffer);
}

RingBuffer *uart_get_rx_buffer(void) {
    return &mock_rx_buffer;
}

void uart_transmit_dma(const uint8_t *data, uint16_t length) {
    if (data != NULL && length > 0 && length < sizeof(last_dma_tx_buf)) {
        memcpy(last_dma_tx_buf, data, length);
        last_dma_tx_len = length;
    }
}

void uart_transmit_bytes(const uint8_t *data, uint16_t length) {
    uart_transmit_dma(data, length);
}

bool uart_is_dma_tx_busy(void) {
    return false; /* Instantly completed in mock simulation */
}

void uart_rx_isr(void) {}
void uart_dma_tx_isr(void) {}

/* I2C Mock Drivers */
void i2c_init(void) {
    mock_i2c_dma_busy_state = false;
    mock_i2c_last_dma_status = I2C_STATUS_SUCCESS;
}

I2C_Status i2c_write_polling(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t length) {
    (void)dev_addr; (void)reg_addr; (void)data; (void)length;
    return I2C_STATUS_SUCCESS;
}

I2C_Status i2c_read_polling(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t length) {
    (void)dev_addr; (void)reg_addr; (void)length;
    if (data != NULL) {
        /* Return test pattern for reading */
        for (uint16_t i = 0; i < length; ++i) {
            data[i] = (uint8_t)(i + 1);
        }
    }
    return I2C_STATUS_SUCCESS;
}

I2C_Status i2c_write_dma(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t length) {
    (void)dev_addr; (void)reg_addr; (void)data; (void)length;
    mock_i2c_last_dma_status = I2C_STATUS_SUCCESS;
    mock_i2c_dma_busy_state = false;
    return I2C_STATUS_SUCCESS;
}

I2C_Status i2c_read_dma(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t length) {
    (void)dev_addr; (void)reg_addr;
    if (data != NULL) {
        for (uint16_t i = 0; i < length; ++i) {
            data[i] = (uint8_t)(i + 1);
        }
    }
    mock_i2c_last_dma_status = I2C_STATUS_SUCCESS;
    mock_i2c_dma_busy_state = false;
    return I2C_STATUS_SUCCESS;
}

bool i2c_is_dma_busy(void) {
    return mock_i2c_dma_busy_state;
}

I2C_Status i2c_get_last_dma_status(void) {
    return mock_i2c_last_dma_status;
}

void i2c_dma_tx_isr(void) {}
void i2c_dma_rx_isr(void) {}

/* External Test Functions */
extern void run_ring_buffer_tests(void);
extern void run_protocol_tests(void);

int main(void) {
    printf("==================================================\n");
    printf("     RUNNING HOST UNIT TESTS FOR CORTEX-M3 BRIDGE  \n");
    printf("==================================================\n");

    /* 1. Run Ring Buffer Tests */
    run_ring_buffer_tests();

    /* 2. Run Protocol State Machine Tests */
    run_protocol_tests();

    printf("\n==================================================\n");
    printf("      ALL UNIT TESTS PASSED SUCCESSFULLY!          \n");
    printf("==================================================\n");
    return 0;
}
