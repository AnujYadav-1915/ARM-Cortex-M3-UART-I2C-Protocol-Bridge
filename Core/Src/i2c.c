/**
 ******************************************************************************
 * @file    i2c.c
 * @brief   Register-Level I2C Master Driver Implementation.
 *          Implements register control and DMA offloading for STM32F103 I2C1.
 ******************************************************************************
 */

#include "i2c.h"
#include "config.h"
#include "stm32f103xb.h"
#include "core_cm3.h"
#include <stddef.h>

/* DMA I2C Status Trackers */
static volatile bool i2c_dma_busy = false;
static volatile I2C_Status i2c_last_dma_status = I2C_STATUS_SUCCESS;

/* Hardware Timeout Helper (uses loop counts for register safety) */
#define I2C_TIMEOUT_COUNT    100000U

static I2C_Status wait_flag_status(volatile uint32_t *reg, uint32_t flag, uint32_t status) {
    uint32_t timeout = I2C_TIMEOUT_COUNT;
    while (((*reg & flag) ? 1U : 0U) != status) {
        if (--timeout == 0U) {
            return I2C_STATUS_TIMEOUT;
        }
        
        /* Check for physical error flags in SR1 */
        if (I2C1->SR1 & (I2C_SR1_AF | I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_TIMEOUT)) {
            /* Clear error flags (read SR1 then write 0 to flags) */
            I2C1->SR1 &= ~(I2C_SR1_AF | I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_TIMEOUT);
            return I2C_STATUS_NACK;
        }
        __DMB();
    }
    return I2C_STATUS_SUCCESS;
}

void i2c_init(void) {
    i2c_dma_busy = false;
    i2c_last_dma_status = I2C_STATUS_SUCCESS;

    /* 1. Enable Peripheral Clocks: I2C1, GPIOB, and DMA1 */
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC->AHBENR  |= RCC_AHBENR_DMA1EN;

    /* 2. Configure GPIO Pins (PB6 - SCL, PB7 - SDA)
     * Alternate Function Open-Drain, Speed 50MHz (CNF = 11, MODE = 11 -> 0xF)
     */
    uint32_t crl = GPIOB->CRL;
    crl &= ~((0xFU << 24U) | (0xFU << 28U)); /* Clear PB6 and PB7 */
    crl |= (0xFU << 24U) | (0xFU << 28U);    /* PB6 & PB7 to 0xF */
    GPIOB->CRL = crl;

    /* 3. Reset I2C1 to clean any stuck state */
    I2C1->CR1 |= I2C_CR1_SWRST;
    __DMB();
    I2C1->CR1 &= ~I2C_CR1_SWRST;
    __DMB();

    /* 4. Configure I2C1 Control Register 2
     * Set FREQ bits equal to APB1 clock in MHz (36 MHz -> 36)
     */
    I2C1->CR2 = 36U & I2C_CR2_FREQ;

    /* 5. Configure Clock Control Register (CCR) for Fast Mode 400kHz
     * Fast Mode: FS = 1, DUTY = 0 (1:2 ratio)
     * Thigh = CCR * Tck, Tlow = 2 * CCR * Tck -> Period = 3 * CCR * Tck
     * CCR = APB1_CLOCK / (3 * I2C1_SPEED) = 36000000 / (3 * 400000) = 30
     */
    I2C1->CCR = I2C_CCR_FS | (30U & I2C_CCR_CCR);

    /* 6. Configure TRISE (Maximum Rise Time in Fast Mode is 300ns)
     * TRISE = (300ns / T_APB1) + 1 = (300ns * 36MHz) + 1 = 10.8 + 1 = 11
     */
    I2C1->TRISE = 11U;

    /* 7. Enable Address Acknowledge (ACK) and Enable I2C Peripheral (PE) */
    I2C1->CR1 |= I2C_CR1_ACK | I2C_CR1_PE;

    /* 8. Setup Fixed DMA Channels Destination and Source Pointers */
    DMA1_Channel6->CPAR = (uint32_t)(&I2C1->DR); /* I2C1 TX DMA */
    DMA1_Channel7->CPAR = (uint32_t)(&I2C1->DR); /* I2C1 RX DMA */

    /* NVIC interrupts enabled by DMA/UART drivers */
}

I2C_Status i2c_write_polling(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t length) {
    I2C_Status status;
    volatile uint32_t temp;

    /* Wait for I2C bus to be free */
    status = wait_flag_status(&I2C1->SR2, I2C_SR2_BUSY, 0U);
    if (status != I2C_STATUS_SUCCESS) return status;

    /* 1. Generate START Condition */
    I2C1->CR1 |= I2C_CR1_START;
    status = wait_flag_status(&I2C1->SR1, I2C_SR1_SB, 1U);
    if (status != I2C_STATUS_SUCCESS) return status;

    /* 2. Send Device Address (Write mode: LSB = 0) */
    I2C1->DR = (uint8_t)(dev_addr << 1U);
    status = wait_flag_status(&I2C1->SR1, I2C_SR1_ADDR, 1U);
    if (status != I2C_STATUS_SUCCESS) {
        I2C1->CR1 |= I2C_CR1_STOP;
        return status;
    }
    
    /* Clear ADDR flag by reading SR2 */
    temp = I2C1->SR2;
    (void)temp;

    /* 3. Send Register/Memory Address */
    I2C1->DR = reg_addr;
    status = wait_flag_status(&I2C1->SR1, I2C_SR1_TXE, 1U);
    if (status != I2C_STATUS_SUCCESS) {
        I2C1->CR1 |= I2C_CR1_STOP;
        return status;
    }

    /* 4. Stream Data Bytes */
    if (data != NULL) {
        for (uint16_t i = 0; i < length; ++i) {
            I2C1->DR = data[i];
            status = wait_flag_status(&I2C1->SR1, I2C_SR1_TXE, 1U);
            if (status != I2C_STATUS_SUCCESS) {
                I2C1->CR1 |= I2C_CR1_STOP;
                return status;
            }
        }
    }

    /* Wait for BTF (Byte Transfer Finished) to complete last byte */
    status = wait_flag_status(&I2C1->SR1, I2C_SR1_BTF, 1U);
    if (status != I2C_STATUS_SUCCESS) {
        I2C1->CR1 |= I2C_CR1_STOP;
        return status;
    }

    /* 5. Generate STOP Condition */
    I2C1->CR1 |= I2C_CR1_STOP;

    return I2C_STATUS_SUCCESS;
}

I2C_Status i2c_read_polling(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t length) {
    I2C_Status status;
    volatile uint32_t temp;

    if (data == NULL || length == 0) {
        return I2C_STATUS_ERROR;
    }

    /* Wait for I2C bus to be free */
    status = wait_flag_status(&I2C1->SR2, I2C_SR2_BUSY, 0U);
    if (status != I2C_STATUS_SUCCESS) return status;

    /* 1. Generate START Condition to write Register address */
    I2C1->CR1 |= I2C_CR1_START;
    status = wait_flag_status(&I2C1->SR1, I2C_SR1_SB, 1U);
    if (status != I2C_STATUS_SUCCESS) return status;

    /* Send Address (Write) */
    I2C1->DR = (uint8_t)(dev_addr << 1U);
    status = wait_flag_status(&I2C1->SR1, I2C_SR1_ADDR, 1U);
    if (status != I2C_STATUS_SUCCESS) {
        I2C1->CR1 |= I2C_CR1_STOP;
        return status;
    }
    temp = I2C1->SR2;
    (void)temp;

    /* Send Register address */
    I2C1->DR = reg_addr;
    status = wait_flag_status(&I2C1->SR1, I2C_SR1_TXE, 1U);
    if (status != I2C_STATUS_SUCCESS) {
        I2C1->CR1 |= I2C_CR1_STOP;
        return status;
    }
    status = wait_flag_status(&I2C1->SR1, I2C_SR1_BTF, 1U);
    if (status != I2C_STATUS_SUCCESS) {
        I2C1->CR1 |= I2C_CR1_STOP;
        return status;
    }

    /* 2. Generate REPEATED START Condition for Reading */
    I2C1->CR1 |= I2C_CR1_START;
    status = wait_flag_status(&I2C1->SR1, I2C_SR1_SB, 1U);
    if (status != I2C_STATUS_SUCCESS) return status;

    /* Send Address (Read mode: LSB = 1) */
    I2C1->DR = (uint8_t)((dev_addr << 1U) | 0x01U);
    status = wait_flag_status(&I2C1->SR1, I2C_SR1_ADDR, 1U);
    if (status != I2C_STATUS_SUCCESS) {
        I2C1->CR1 |= I2C_CR1_STOP;
        return status;
    }

    /* 3. Stream Data Bytes */
    if (length == 1U) {
        /* Disable Acknowledge before clearing ADDR for single-byte read */
        I2C1->CR1 &= ~I2C_CR1_ACK;
        
        /* Clear ADDR flag */
        temp = I2C1->SR2;
        (void)temp;
        
        /* Generate STOP right after clearing ADDR */
        I2C1->CR1 |= I2C_CR1_STOP;
        
        /* Wait for RXNE */
        status = wait_flag_status(&I2C1->SR1, I2C_SR1_RxNE, 1U);
        if (status != I2C_STATUS_SUCCESS) return status;
        
        data[0] = (uint8_t)I2C1->DR;
    } else {
        /* Enable ACK */
        I2C1->CR1 |= I2C_CR1_ACK;
        
        /* Clear ADDR flag */
        temp = I2C1->SR2;
        (void)temp;

        for (uint16_t i = 0; i < length; ++i) {
            if (i == (length - 1U)) {
                /* NACK the very last byte */
                I2C1->CR1 &= ~I2C_CR1_ACK;
                I2C1->CR1 |= I2C_CR1_STOP;
            }

            /* Wait for RXNE */
            status = wait_flag_status(&I2C1->SR1, I2C_SR1_RxNE, 1U);
            if (status != I2C_STATUS_SUCCESS) return status;

            data[i] = (uint8_t)I2C1->DR;
        }
    }

    /* Re-enable ACK for future transactions */
    I2C1->CR1 |= I2C_CR1_ACK;

    return I2C_STATUS_SUCCESS;
}

I2C_Status i2c_write_dma(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t length) {
    I2C_Status status;
    volatile uint32_t temp;

    if (i2c_dma_busy) return I2C_STATUS_BUSY;

    /* Wait for bus to be free */
    status = wait_flag_status(&I2C1->SR2, I2C_SR2_BUSY, 0U);
    if (status != I2C_STATUS_SUCCESS) return status;

    i2c_dma_busy = true;
    i2c_last_dma_status = I2C_STATUS_SUCCESS;

    /* 1. Configure DMA1 Channel 6 for Writing */
    DMA1_Channel6->CCR &= ~DMA_CCR_EN;
    DMA1_Channel6->CMAR = (uint32_t)data;
    DMA1_Channel6->CNDTR = length;
    DMA1->IFCR = DMA_IFCR_CGIF6 | DMA_IFCR_CTCIF6;
    
    /* Config: DIR=1 (Mem2Periph), MINC=1, TCIE=1, PL=10 (High), EN=0 */
    DMA1_Channel6->CCR = DMA_CCR_DIR | DMA_CCR_MINC | DMA_CCR_TCIE | (0x2U << DMA_CCR_PL_Pos);

    /* 2. Enable DMA Request in I2C Control Register 2 */
    I2C1->CR2 |= I2C_CR2_DMAEN;

    /* 3. Send Start */
    I2C1->CR1 |= I2C_CR1_START;
    status = wait_flag_status(&I2C1->SR1, I2C_SR1_SB, 1U);
    if (status != I2C_STATUS_SUCCESS) {
        i2c_dma_busy = false;
        return status;
    }

    /* 4. Send Device Address (Write) */
    I2C1->DR = (uint8_t)(dev_addr << 1U);
    status = wait_flag_status(&I2C1->SR1, I2C_SR1_ADDR, 1U);
    if (status != I2C_STATUS_SUCCESS) {
        I2C1->CR1 |= I2C_CR1_STOP;
        i2c_dma_busy = false;
        return status;
    }
    temp = I2C1->SR2;
    (void)temp;

    /* 5. Send Register Address */
    I2C1->DR = reg_addr;
    status = wait_flag_status(&I2C1->SR1, I2C_SR1_TXE, 1U);
    if (status != I2C_STATUS_SUCCESS) {
        I2C1->CR1 |= I2C_CR1_STOP;
        i2c_dma_busy = false;
        return status;
    }

    /* Wait for BTF before enabling DMA to write data payload */
    status = wait_flag_status(&I2C1->SR1, I2C_SR1_BTF, 1U);
    if (status != I2C_STATUS_SUCCESS) {
        I2C1->CR1 |= I2C_CR1_STOP;
        i2c_dma_busy = false;
        return status;
    }

    /* 6. Enable DMA Channel to stream bulk payload */
    DMA1_Channel6->CCR |= DMA_CCR_EN;

    return I2C_STATUS_SUCCESS;
}

I2C_Status i2c_read_dma(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t length) {
    I2C_Status status;
    volatile uint32_t temp;

    if (i2c_dma_busy) return I2C_STATUS_BUSY;

    status = wait_flag_status(&I2C1->SR2, I2C_SR2_BUSY, 0U);
    if (status != I2C_STATUS_SUCCESS) return status;

    i2c_dma_busy = true;
    i2c_last_dma_status = I2C_STATUS_SUCCESS;

    /* 1. Configure DMA1 Channel 7 for Reading */
    DMA1_Channel7->CCR &= ~DMA_CCR_EN;
    DMA1_Channel7->CMAR = (uint32_t)data;
    DMA1_Channel7->CNDTR = length;
    DMA1->IFCR = DMA_IFCR_CGIF7 | DMA_IFCR_CTCIF7;
    
    /* Config: DIR=0 (Periph2Mem), MINC=1, TCIE=1, PL=10 (High), EN=0 */
    DMA1_Channel7->CCR = DMA_CCR_MINC | DMA_CCR_TCIE | (0x2U << DMA_CCR_PL_Pos);

    /* Enable DMA Request in I2C */
    I2C1->CR2 |= I2C_CR2_DMAEN;

    /* 2. Write Address & Register */
    I2C1->CR1 |= I2C_CR1_START;
    status = wait_flag_status(&I2C1->SR1, I2C_SR1_SB, 1U);
    if (status != I2C_STATUS_SUCCESS) {
        i2c_dma_busy = false;
        return status;
    }

    I2C1->DR = (uint8_t)(dev_addr << 1U);
    status = wait_flag_status(&I2C1->SR1, I2C_SR1_ADDR, 1U);
    if (status != I2C_STATUS_SUCCESS) {
        I2C1->CR1 |= I2C_CR1_STOP;
        i2c_dma_busy = false;
        return status;
    }
    temp = I2C1->SR2;
    (void)temp;

    I2C1->DR = reg_addr;
    status = wait_flag_status(&I2C1->SR1, I2C_SR1_TXE, 1U);
    if (status != I2C_STATUS_SUCCESS) {
        I2C1->CR1 |= I2C_CR1_STOP;
        i2c_dma_busy = false;
        return status;
    }
    status = wait_flag_status(&I2C1->SR1, I2C_SR1_BTF, 1U);
    if (status != I2C_STATUS_SUCCESS) {
        I2C1->CR1 |= I2C_CR1_STOP;
        i2c_dma_busy = false;
        return status;
    }

    /* 3. Send Repeated Start */
    I2C1->CR1 |= I2C_CR1_START;
    status = wait_flag_status(&I2C1->SR1, I2C_SR1_SB, 1U);
    if (status != I2C_STATUS_SUCCESS) {
        i2c_dma_busy = false;
        return status;
    }

    /* Send Address (Read) */
    I2C1->DR = (uint8_t)((dev_addr << 1U) | 0x01U);
    status = wait_flag_status(&I2C1->SR1, I2C_SR1_ADDR, 1U);
    if (status != I2C_STATUS_SUCCESS) {
        I2C1->CR1 |= I2C_CR1_STOP;
        i2c_dma_busy = false;
        return status;
    }

    /* Configure NACK on last DMA transfer in I2C Control Register 2 */
    I2C1->CR2 |= I2C_CR2_LAST;

    /* Clear ADDR flag */
    temp = I2C1->SR2;
    (void)temp;

    /* 4. Enable DMA Channel to start reading payload */
    DMA1_Channel7->CCR |= DMA_CCR_EN;

    return I2C_STATUS_SUCCESS;
}

bool i2c_is_dma_busy(void) {
    return i2c_dma_busy;
}

I2C_Status i2c_get_last_dma_status(void) {
    return i2c_last_dma_status;
}

void i2c_dma_tx_isr(void) {
    /* Check Transfer Complete interrupt status flag for DMA1 Channel 6 */
    if (DMA1->ISR & DMA_ISR_TCIF6) {
        /* Clear DMA flag */
        DMA1->IFCR = DMA_IFCR_CTCIF6;
        
        /* Disable DMA Channel */
        DMA1_Channel6->CCR &= ~DMA_CCR_EN;
        
        /* Wait for Byte Transfer Finished to ensure I2C data is completely shifted */
        (void)wait_flag_status(&I2C1->SR1, I2C_SR1_BTF, 1U);
        
        /* Generate Stop Condition */
        I2C1->CR1 |= I2C_CR1_STOP;

        /* Disable I2C DMA request and clear LAST bit */
        I2C1->CR2 &= ~I2C_CR2_DMAEN;
        
        /* Mark I2C DMA transaction complete */
        i2c_last_dma_status = I2C_STATUS_SUCCESS;
        i2c_dma_busy = false;
        __DMB();
    }
}

void i2c_dma_rx_isr(void) {
    /* Check Transfer Complete interrupt status flag for DMA1 Channel 7 */
    if (DMA1->ISR & DMA_ISR_TCIF7) {
        /* Clear DMA flag */
        DMA1->IFCR = DMA_IFCR_CTCIF7;
        
        /* Disable DMA Channel */
        DMA1_Channel7->CCR &= ~DMA_CCR_EN;
        
        /* Generate Stop Condition */
        I2C1->CR1 |= I2C_CR1_STOP;

        /* Disable I2C DMA request and clear LAST bit */
        I2C1->CR2 &= ~(I2C_CR2_DMAEN | I2C_CR2_LAST);
        
        /* Mark I2C DMA transaction complete */
        i2c_last_dma_status = I2C_STATUS_SUCCESS;
        i2c_dma_busy = false;
        __DMB();
    }
}
