/**
 ******************************************************************************
 * @file    mock_stm32.h
 * @brief   Mock Memory Mapped Structural Registers for Host Unit Testing.
 *          Maps peripheral symbols (USART1, I2C1, DMA1) to global variables
 *          that tests can inspect and manipulate.
 ******************************************************************************
 */

#ifndef MOCK_STM32_H
#define MOCK_STM32_H

#ifndef MOCK_STM32_TESTS
#define MOCK_STM32_TESTS
#endif

#include "stm32f103xb.h"

/* Global instances of mock register maps */
extern RCC_TypeDef   mock_RCC;
extern GPIO_TypeDef  mock_GPIOA;
extern GPIO_TypeDef  mock_GPIOB;
extern USART_TypeDef mock_USART1;
extern I2C_TypeDef   mock_I2C1;
extern DMA_TypeDef   mock_DMA1;

/* Redefine registers to point to mocks for Host Execution */
#define RCC                 (&mock_RCC)
#define GPIOA               (&mock_GPIOA)
#define GPIOB               (&mock_GPIOB)
#define USART1              (&mock_USART1)
#define I2C1                (&mock_I2C1)
#define DMA1                (&mock_DMA1)
#define DMA1_Channel4       (&mock_DMA1.Ch4)
#define DMA1_Channel6       (&mock_DMA1.Ch6)
#define DMA1_Channel7       (&mock_DMA1.Ch7)

/**
 * @brief Resets all mock register contents to zero or default reset values.
 */
void mock_reset_peripherals(void);

#endif /* MOCK_STM32_H */
