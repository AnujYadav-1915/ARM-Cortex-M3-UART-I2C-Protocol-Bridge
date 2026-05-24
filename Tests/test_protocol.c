/**
 ******************************************************************************
 * @file    test_protocol.c
 * @brief   Unit Tests for Protocol Parser State Machine.
 ******************************************************************************
 */

#include "protocol.h"
#include "uart.h"
#include "i2c.h"
#include "mock_stm32.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

/* Track mock tx output in tests */
extern uint8_t last_dma_tx_buf[PROTOCOL_MAX_PAYLOAD + 8];
extern uint16_t last_dma_tx_len;

static void push_packet_bytes(const uint8_t *data, uint16_t length) {
    RingBuffer *rx_rb = uart_get_rx_buffer();
    for (uint16_t i = 0; i < length; ++i) {
        (void)ring_buffer_push(rx_rb, data[i]);
    }
}

void test_protocol_status_query(void) {
    printf("[TEST] test_protocol_status_query: Running...\n");

    /* Reset states */
    mock_reset_peripherals();
    protocol_init();
    last_dma_tx_len = 0;
    memset(last_dma_tx_buf, 0, sizeof(last_dma_tx_buf));

    /* Sync (0xAA), CMD_STATUS_QUERY (0x03), Checksum (0x03) */
    uint8_t query_packet[] = { PROTOCOL_SYNC_RX, CMD_STATUS_QUERY, 0x03 };

    push_packet_bytes(query_packet, sizeof(query_packet));

    /* Execute parser */
    protocol_process();

    /* Verify response was sent via DMA Tx
     * Expect: Sync (0x55), Success (0x00), Length MSB (0x00), Length LSB (0x04),
     *         Telemetry[4], Checksum (0x04)
     */
    assert(last_dma_tx_len == 9U);
    assert(last_dma_tx_buf[0] == PROTOCOL_SYNC_TX);
    assert(last_dma_tx_buf[1] == STATUS_SUCCESS);
    assert(last_dma_tx_buf[2] == 0x00);
    assert(last_dma_tx_buf[3] == 0x04); /* Telemetry length */

    /* Verify checksum matches: status + len_msb + len_lsb + tel[0] + tel[1] + tel[2] + tel[3] */
    uint8_t expected_chk = last_dma_tx_buf[1] + last_dma_tx_buf[2] + last_dma_tx_buf[3] +
                           last_dma_tx_buf[4] + last_dma_tx_buf[5] + last_dma_tx_buf[6] + last_dma_tx_buf[7];
    assert(last_dma_tx_buf[8] == expected_chk);

    printf("[TEST] test_protocol_status_query: PASSED\n");
}

void test_protocol_checksum_error(void) {
    printf("[TEST] test_protocol_checksum_error: Running...\n");

    mock_reset_peripherals();
    protocol_init();
    last_dma_tx_len = 0;

    /* Sync (0xAA), CMD_STATUS_QUERY (0x03), Mismatched Checksum (0xFF) */
    uint8_t corrupt_packet[] = { PROTOCOL_SYNC_RX, CMD_STATUS_QUERY, 0xFF };

    push_packet_bytes(corrupt_packet, sizeof(corrupt_packet));
    protocol_process();

    /* Expect rejection response:
     * Sync (0x55), STATUS_CHECKSUM_ERR (0x04), Len MSB (0x00), Len LSB (0x00), Checksum (0x04)
     */
    assert(last_dma_tx_len == 5U);
    assert(last_dma_tx_buf[0] == PROTOCOL_SYNC_TX);
    assert(last_dma_tx_buf[1] == STATUS_CHECKSUM_ERR);
    assert(last_dma_tx_buf[4] == STATUS_CHECKSUM_ERR);

    printf("[TEST] test_protocol_checksum_error: PASSED\n");
}

void test_protocol_invalid_command(void) {
    printf("[TEST] test_protocol_invalid_command: Running...\n");

    mock_reset_peripherals();
    protocol_init();
    last_dma_tx_len = 0;

    /* Sync (0xAA), Unsupported Command Code (0x99) */
    uint8_t invalid_cmd_packet[] = { PROTOCOL_SYNC_RX, 0x99 };

    push_packet_bytes(invalid_cmd_packet, sizeof(invalid_cmd_packet));
    protocol_process();

    /* Expect immediate invalid command response:
     * Sync (0x55), STATUS_INVALID_CMD (0x05), Len MSB (0x00), Len LSB (0x00), Checksum (0x05)
     */
    assert(last_dma_tx_len == 5U);
    assert(last_dma_tx_buf[0] == PROTOCOL_SYNC_TX);
    assert(last_dma_tx_buf[1] == STATUS_INVALID_CMD);

    printf("[TEST] test_protocol_invalid_command: PASSED\n");
}

void run_protocol_tests(void) {
    test_protocol_status_query();
    test_protocol_checksum_error();
    test_protocol_invalid_command();
}
