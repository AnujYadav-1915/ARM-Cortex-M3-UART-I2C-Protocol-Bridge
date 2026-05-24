/**
 ******************************************************************************
 * @file    config.h
 * @brief   System Configuration Settings.
 *          Defines clock speeds, peripheral configuration settings, 
 *          timeouts, and physical pins map.
 ******************************************************************************
 */

#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* System Clock Settings */
#define SYSTEM_CORE_CLOCK     72000000U /* 72 MHz main system clock */
#define APB2_CLOCK            72000000U /* APB2 clock (72 MHz max) */
#define APB1_CLOCK            36000000U /* APB1 clock (36 MHz max) */

/* USART1 Configurations */
#define USART1_BAUDRATE       115200U
#define UART_RX_BUF_SIZE      256U      /* SPSC Ring Buffer capacity */

/* I2C1 Configurations */
#define I2C1_SPEED            400000U   /* I2C Fast-Mode clock speed (400 kHz) */
#define I2C_DUTY_CYCLE_2      0U        /* Standard I2C Tlow/Thigh duty cycle (1:2) */
#define I2C1_OWN_ADDRESS      0x0A      /* Bridge's own 7-bit I2C address */
#define I2C_TIMEOUT_MS        500U      /* Maximum wait time for bus flags (ms) */

/* Pin Definitions for Reference (STM32F103xB) */
/* Port A Pin Definitions */
#define USART1_TX_PIN         9U        /* PA9  - Alternate Function Push-Pull */
#define USART1_RX_PIN         10U       /* PA10 - Input Floating / Input Pull-Up */

/* Port B Pin Definitions */
#define I2C1_SCL_PIN          6U        /* PB6  - Alternate Function Open-Drain */
#define I2C1_SDA_PIN          7U        /* PB7  - Alternate Function Open-Drain */

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_H */
