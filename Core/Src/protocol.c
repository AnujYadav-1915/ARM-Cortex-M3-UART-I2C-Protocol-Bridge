/**
 ******************************************************************************
 * @file    protocol.c
 * @brief   Bridge Protocol & Command Parser State Machine Implementation.
 *          Decodes incoming UART frame flows and executes I2C transactions.
 ******************************************************************************
 */

#include "protocol.h"
#include "uart.h"
#include "i2c.h"
#include "core_cm3.h"
#include <string.h>

/* Parser variables */
static ParserState current_state = STATE_SYNC;
static uint8_t rx_cmd_type;
static uint8_t rx_dev_addr;
static uint8_t rx_reg_addr;
static uint16_t rx_payload_len;
static uint16_t rx_payload_idx;
static uint8_t rx_payload[PROTOCOL_MAX_PAYLOAD];
static uint8_t calculated_checksum;

/* Telemetry counters */
static uint8_t stat_processed_packets = 0;
static uint8_t stat_buffer_overruns = 0;
static uint8_t stat_last_err = STATUS_SUCCESS;

/* Transmit double buffer for UART DMA responses */
static uint8_t tx_buffer[PROTOCOL_MAX_PAYLOAD + 8U];

void protocol_init(void) {
    current_state = STATE_SYNC;
    rx_cmd_type = 0;
    rx_dev_addr = 0;
    rx_reg_addr = 0;
    rx_payload_len = 0;
    rx_payload_idx = 0;
    calculated_checksum = 0;
    
    stat_processed_packets = 0;
    stat_buffer_overruns = 0;
    stat_last_err = STATUS_SUCCESS;
    
    memset(rx_payload, 0, sizeof(rx_payload));
    memset(tx_buffer, 0, sizeof(tx_buffer));
}

void protocol_tx_response(uint8_t status, const uint8_t *payload, uint16_t length) {
    if (length > PROTOCOL_MAX_PAYLOAD) {
        length = PROTOCOL_MAX_PAYLOAD;
    }

    tx_buffer[0] = PROTOCOL_SYNC_TX;
    tx_buffer[1] = status;
    tx_buffer[2] = (uint8_t)((length >> 8U) & 0xFFU);
    tx_buffer[3] = (uint8_t)(length & 0xFFU);

    uint8_t tx_checksum = status + tx_buffer[2] + tx_buffer[3];

    if (payload != NULL && length > 0U) {
        memcpy(&tx_buffer[4], payload, length);
        for (uint16_t i = 0U; i < length; ++i) {
            tx_checksum += payload[i];
        }
    }

    tx_buffer[4U + length] = tx_checksum;

    /* Offload packet transmission entirely to UART DMA Channel 4 */
    uart_transmit_dma(tx_buffer, length + 5U);
}

static void execute_command(void) {
    I2C_Status i2c_status = I2C_STATUS_SUCCESS;
    uint8_t local_read_buf[PROTOCOL_MAX_PAYLOAD];

    switch (rx_cmd_type) {
        case CMD_STATUS_QUERY: {
            /* Return diagnostic telemetry */
            uint8_t telemetry[4];
            telemetry[0] = (uint8_t)ring_buffer_get_count(uart_get_rx_buffer());
            telemetry[1] = stat_processed_packets;
            telemetry[2] = stat_buffer_overruns;
            telemetry[3] = stat_last_err;
            
            protocol_tx_response(STATUS_SUCCESS, telemetry, 4U);
            stat_processed_packets++;
            break;
        }

        case CMD_I2C_WRITE: {
            /* Decide based on payload size: Bulk transfers (>= 4 bytes) use DMA */
            if (rx_payload_len >= 4U) {
                i2c_status = i2c_write_dma(rx_dev_addr, rx_reg_addr, rx_payload, rx_payload_len);
                if (i2c_status == I2C_STATUS_SUCCESS) {
                    /* Wait for DMA to complete asynchronously */
                    while (i2c_is_dma_busy()) {
                        #ifndef MOCK_STM32_TESTS
                        __WFI(); /* Put CPU into low-power sleep waiting for interrupt */
                        #else
                        __DMB();
                        #endif
                    }
                    i2c_status = i2c_get_last_dma_status();
                }
            } else {
                i2c_status = i2c_write_polling(rx_dev_addr, rx_reg_addr, rx_payload, rx_payload_len);
            }

            /* Translate I2C outcome to Protocol Response */
            if (i2c_status == I2C_STATUS_SUCCESS) {
                protocol_tx_response(STATUS_SUCCESS, NULL, 0U);
                stat_last_err = STATUS_SUCCESS;
            } else if (i2c_status == I2C_STATUS_NACK) {
                protocol_tx_response(STATUS_I2C_NACK, NULL, 0U);
                stat_last_err = STATUS_I2C_NACK;
            } else {
                protocol_tx_response(STATUS_I2C_TIMEOUT, NULL, 0U);
                stat_last_err = STATUS_I2C_TIMEOUT;
            }
            stat_processed_packets++;
            break;
        }

        case CMD_I2C_READ: {
            if (rx_payload_len > PROTOCOL_MAX_PAYLOAD) {
                rx_payload_len = PROTOCOL_MAX_PAYLOAD;
            }

            memset(local_read_buf, 0, rx_payload_len);

            /* Bulk reads (>= 4 bytes) offloaded to DMA */
            if (rx_payload_len >= 4U) {
                i2c_status = i2c_read_dma(rx_dev_addr, rx_reg_addr, local_read_buf, rx_payload_len);
                if (i2c_status == I2C_STATUS_SUCCESS) {
                    while (i2c_is_dma_busy()) {
                        #ifndef MOCK_STM32_TESTS
                        __WFI();
                        #else
                        __DMB();
                        #endif
                    }
                    i2c_status = i2c_get_last_dma_status();
                }
            } else {
                i2c_status = i2c_read_polling(rx_dev_addr, rx_reg_addr, local_read_buf, rx_payload_len);
            }

            if (i2c_status == I2C_STATUS_SUCCESS) {
                protocol_tx_response(STATUS_SUCCESS, local_read_buf, rx_payload_len);
                stat_last_err = STATUS_SUCCESS;
            } else if (i2c_status == I2C_STATUS_NACK) {
                protocol_tx_response(STATUS_I2C_NACK, NULL, 0U);
                stat_last_err = STATUS_I2C_NACK;
            } else {
                protocol_tx_response(STATUS_I2C_TIMEOUT, NULL, 0U);
                stat_last_err = STATUS_I2C_TIMEOUT;
            }
            stat_processed_packets++;
            break;
        }

        default: {
            protocol_tx_response(STATUS_INVALID_CMD, NULL, 0U);
            stat_last_err = STATUS_INVALID_CMD;
            break;
        }
    }
}

void protocol_process(void) {
    uint8_t byte;
    RingBuffer *rx_rb = uart_get_rx_buffer();

    /* Process all bytes currently in the receive circular buffer */
    while (ring_buffer_pop(rx_rb, &byte)) {
        switch (current_state) {
            case STATE_SYNC:
                if (byte == PROTOCOL_SYNC_RX) {
                    current_state = STATE_CMD_TYPE;
                    calculated_checksum = 0U;
                }
                break;

            case STATE_CMD_TYPE:
                rx_cmd_type = byte;
                calculated_checksum += byte;
                
                if (rx_cmd_type == CMD_STATUS_QUERY) {
                    /* Status query has no dev/reg/payload fields, goes to checksum */
                    current_state = STATE_CHECKSUM;
                } else if (rx_cmd_type == CMD_I2C_READ || rx_cmd_type == CMD_I2C_WRITE) {
                    current_state = STATE_DEV_ADDR;
                } else {
                    /* Invalid command, report error and return to sync */
                    protocol_tx_response(STATUS_INVALID_CMD, NULL, 0U);
                    stat_last_err = STATUS_INVALID_CMD;
                    current_state = STATE_SYNC;
                }
                break;

            case STATE_DEV_ADDR:
                rx_dev_addr = byte;
                calculated_checksum += byte;
                current_state = STATE_REG_ADDR;
                break;

            case STATE_REG_ADDR:
                rx_reg_addr = byte;
                calculated_checksum += byte;
                current_state = STATE_LEN_MSB;
                break;

            case STATE_LEN_MSB:
                rx_payload_len = (uint16_t)(((uint16_t)byte) << 8U);
                calculated_checksum += byte;
                current_state = STATE_LEN_LSB;
                break;

            case STATE_LEN_LSB:
                rx_payload_len |= byte;
                calculated_checksum += byte;
                rx_payload_idx = 0U;

                if (rx_payload_len > PROTOCOL_MAX_PAYLOAD) {
                    protocol_tx_response(STATUS_GENERIC_ERR, NULL, 0U);
                    stat_last_err = STATUS_GENERIC_ERR;
                    current_state = STATE_SYNC;
                } else if (rx_cmd_type == CMD_I2C_WRITE && rx_payload_len > 0U) {
                    current_state = STATE_PAYLOAD;
                } else {
                    /* Read command doesn't transmit payload, goes directly to checksum */
                    current_state = STATE_CHECKSUM;
                }
                break;

            case STATE_PAYLOAD:
                rx_payload[rx_payload_idx++] = byte;
                calculated_checksum += byte;
                if (rx_payload_idx >= rx_payload_len) {
                    current_state = STATE_CHECKSUM;
                }
                break;

            case STATE_CHECKSUM:
                if (byte == calculated_checksum) {
                    /* Packet valid, execute transaction */
                    execute_command();
                } else {
                    /* Mismatch, send error code */
                    protocol_tx_response(STATUS_CHECKSUM_ERR, NULL, 0U);
                    stat_last_err = STATUS_CHECKSUM_ERR;
                }
                current_state = STATE_SYNC;
                break;

            default:
                current_state = STATE_SYNC;
                break;
        }
    }
}
