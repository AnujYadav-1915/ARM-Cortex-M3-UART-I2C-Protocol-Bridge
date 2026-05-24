/**
 ******************************************************************************
 * @file    i2c.h
 * @brief   Register-Level I2C Master Driver Interface.
 *          Provides low-level register control for I2C master operations,
 *          supporting synchronous polling and high-throughput DMA offloading.
 ******************************************************************************
 */

#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* I2C Transfer Status Codes */
typedef enum {
    I2C_STATUS_SUCCESS = 0x00,
    I2C_STATUS_NACK    = 0x01,
    I2C_STATUS_TIMEOUT = 0x02,
    I2C_STATUS_ERROR   = 0x03,
    I2C_STATUS_BUSY    = 0x04
} I2C_Status;

/**
 * @brief  Initializes I2C1 peripheral as master.
 *         Configures:
 *         - GPIO pins (PB6 SCL and PB7 SDA as AF Open-Drain)
 *         - I2C speed and rise-time registers (Fast-Mode 400kHz)
 *         - Enables peripheral and configures DMA triggers
 */
void i2c_init(void);

/**
 * @brief  Synchronously writes registers/data using polling.
 *         Used primarily for tiny payloads or configuration transactions.
 * @param  dev_addr: 7-bit slave device address.
 * @param  reg_addr: 8-bit memory/register address inside the slave.
 * @param  data: Pointer to data bytes to write.
 * @param  length: Number of bytes to write.
 * @return I2C_Status code.
 */
I2C_Status i2c_write_polling(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t length);

/**
 * @brief  Synchronously reads registers/data using polling.
 * @param  dev_addr: 7-bit slave device address.
 * @param  reg_addr: 8-bit memory/register address inside the slave.
 * @param  data: Pointer to buffer where read bytes will be stored.
 * @param  length: Number of bytes to read.
 * @return I2C_Status code.
 */
I2C_Status i2c_read_polling(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t length);

/**
 * @brief  Asynchronously writes bulk data offloaded to DMA1 Channel 6.
 *         Initiates I2C start, writes device & register addresses, 
 *         then triggers the DMA engine to complete the data payload transfer.
 * @param  dev_addr: 7-bit slave device address.
 * @param  reg_addr: 8-bit memory/register address inside the slave.
 * @param  data: Pointer to source buffer in RAM.
 * @param  length: Number of bytes to write.
 * @return I2C_Status code indicating if the transfer was successfully scheduled.
 */
I2C_Status i2c_write_dma(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t length);

/**
 * @brief  Asynchronously reads bulk data offloaded to DMA1 Channel 7.
 *         Initiates I2C start, writes device & register addresses, sends restart,
 *         then triggers the DMA engine to clock in the data payload.
 * @param  dev_addr: 7-bit slave device address.
 * @param  reg_addr: 8-bit memory/register address inside the slave.
 * @param  data: Pointer to destination buffer in RAM.
 * @param  length: Number of bytes to read.
 * @return I2C_Status code indicating if the transfer was successfully scheduled.
 */
I2C_Status i2c_read_dma(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t length);

/**
 * @brief  Checks if there is an active DMA I2C transaction in progress.
 * @return true if busy, false if idle/completed.
 */
bool i2c_is_dma_busy(void);

/**
 * @brief  Queries the status result of the last DMA transaction.
 * @return I2C_Status of the completed transaction.
 */
I2C_Status i2c_get_last_dma_status(void);

/**
 * @brief  DMA1 Channel 6 (I2C1 TX DMA) Interrupt Service Routine.
 *         Should be called by DMA1_Channel6_IRQHandler.
 */
void i2c_dma_tx_isr(void);

/**
 * @brief  DMA1 Channel 7 (I2C1 RX DMA) Interrupt Service Routine.
 *         Should be called by DMA1_Channel7_IRQHandler.
 */
void i2c_dma_rx_isr(void);

#ifdef __cplusplus
}
#endif

#endif /* I2C_H */
