/**
 ******************************************************************************
 * @file    protocol.h
 * @brief   Bridge Protocol & Command Parser State Machine.
 *          Defines UART packet formats, command codes, status responses, 
 *          and provides the packet parsing state machine.
 ******************************************************************************
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Sync Bytes */
#define PROTOCOL_SYNC_RX      0xAAU     /* Host to Bridge Sync Byte */
#define PROTOCOL_SYNC_TX      0x55U     /* Bridge to Host Sync Byte */

/* Command Types (Host to Bridge) */
#define CMD_I2C_READ          0x01U     /* Read registers from I2C device */
#define CMD_I2C_WRITE         0x02U     /* Write registers to I2C device */
#define CMD_STATUS_QUERY      0x03U     /* Query bridge internal status */

/* Response Status Codes (Bridge to Host) */
#define STATUS_SUCCESS        0x00U     /* Transaction completed successfully */
#define STATUS_I2C_NACK       0x01U     /* I2C target NACKed address or data */
#define STATUS_I2C_TIMEOUT    0x02U     /* I2C peripheral transaction timeout */
#define STATUS_BUF_OVERFLOW   0x03U     /* Lock-free circular buffer overflow */
#define STATUS_CHECKSUM_ERR   0x04U     /* Packet checksum mismatch */
#define STATUS_INVALID_CMD    0x05U     /* Unsupported command code */
#define STATUS_GENERIC_ERR    0x06U     /* Miscellaneous operational error */

/* Protocol Limits */
#define PROTOCOL_MAX_PAYLOAD  512U      /* Maximum size of I2C write/read block */

/* Parser State Machine States */
typedef enum {
    STATE_SYNC = 0,
    STATE_CMD_TYPE,
    STATE_DEV_ADDR,
    STATE_REG_ADDR,
    STATE_LEN_MSB,
    STATE_LEN_LSB,
    STATE_PAYLOAD,
    STATE_CHECKSUM
} ParserState;

/**
 * @brief  Initializes the protocol parser state machine and response buffers.
 */
void protocol_init(void);

/**
 * @brief  Drives the protocol parser state machine.
 *         Reads bytes from the lock-free UART receive buffer, processes packet 
 *         integrity online, and executes corresponding I2C/status transactions.
 *         Should be called regularly in the main-thread background loop.
 */
void protocol_process(void);

/**
 * @brief  Constructs and dispatches a response packet to the host.
 *         Formats the response with sync byte, status byte, payload length, 
 *         payload data, and checksum, and fires UART DMA transmission.
 * @param  status: Status code to report.
 * @param  payload: Pointer to response data.
 * @param  length: Length of response data.
 */
void protocol_tx_response(uint8_t status, const uint8_t *payload, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* PROTOCOL_H */
