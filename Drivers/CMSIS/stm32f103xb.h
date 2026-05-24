/**
 ******************************************************************************
 * @file    stm32f103xb.h
 * @brief   CMSIS STM32F103xB Device Peripheral Access Layer Header File.
 *          Contains register structures and memory map for STM32F103.
 ******************************************************************************
 */

#ifndef STM32F103XB_H
#define STM32F103XB_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Memory Map Base Addresses */
#define FLASH_BASE            ((uint32_t)0x08000000)
#define SRAM_BASE             ((uint32_t)0x20000000)
#define PERIPH_BASE           ((uint32_t)0x40000000)

#define APB1PERIPH_BASE       PERIPH_BASE
#define APB2PERIPH_BASE       (PERIPH_BASE + 0x00010000)
#define AHBPERIPH_BASE        (PERIPH_BASE + 0x00020000)

/* APB1 Peripherals */
#define I2C1_BASE             (APB1PERIPH_BASE + 0x00005400)

/* APB2 Peripherals */
#define GPIOA_BASE            (APB2PERIPH_BASE + 0x00000800)
#define GPIOB_BASE            (APB2PERIPH_BASE + 0x00000C00)
#define USART1_BASE           (APB2PERIPH_BASE + 0x00003800)

/* AHB Peripherals */
#define DMA1_BASE             (AHBPERIPH_BASE + 0x00000000)
#define RCC_BASE              (AHBPERIPH_BASE + 0x00001000)

/* =================================================================********* */
/*                         Peripheral Register Structures                     */
/* =================================================================********* */

/* Reset and Clock Control (RCC) */
typedef struct {
    volatile uint32_t CR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t APB2RSTR;
    volatile uint32_t APB1RSTR;
    volatile uint32_t AHBENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t APB1ENR;
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
} RCC_TypeDef;

/* General Purpose I/O (GPIO) */
typedef struct {
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} GPIO_TypeDef;

/* Universal Synchronous Asynchronous Receiver Transmitter (USART) */
typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} USART_TypeDef;

/* Inter-Integrated Circuit (I2C) */
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t OAR1;
    volatile uint32_t OAR2;
    volatile uint32_t DR;
    volatile uint32_t SR1;
    volatile uint32_t SR2;
    volatile uint32_t CCR;
    volatile uint32_t TRISE;
} I2C_TypeDef;

/* DMA Channel Structure */
typedef struct {
    volatile uint32_t CCR;
    volatile uint32_t CNDTR;
    volatile uint32_t CPAR;
    volatile uint32_t CMAR;
    uint32_t RESERVED;
} DMA_Channel_TypeDef;

/* DMA Controller (DMA) */
typedef struct {
    volatile uint32_t ISR;
    volatile uint32_t IFCR;
    DMA_Channel_TypeDef Ch1;
    DMA_Channel_TypeDef Ch2;
    DMA_Channel_TypeDef Ch3;
    DMA_Channel_TypeDef Ch4;
    DMA_Channel_TypeDef Ch5;
    DMA_Channel_TypeDef Ch6;
    DMA_Channel_TypeDef Ch7;
} DMA_TypeDef;

/* =================================================================********* */
/*                         Peripheral Declarations                            */
/* =================================================================********* */

#ifndef MOCK_STM32_TESTS
/* Real Hardware Mapping - used when compiling for Cortex-M3 target */
#define RCC                 ((RCC_TypeDef *) RCC_BASE)
#define GPIOA               ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB               ((GPIO_TypeDef *) GPIOB_BASE)
#define USART1              ((USART_TypeDef *) USART1_BASE)
#define I2C1                ((I2C_TypeDef *) I2C1_BASE)
#define DMA1                ((DMA_TypeDef *) DMA1_BASE)
#define DMA1_Channel4       ((DMA_Channel_TypeDef *) &DMA1->Ch4)
#define DMA1_Channel6       ((DMA_Channel_TypeDef *) &DMA1->Ch6)
#define DMA1_Channel7       ((DMA_Channel_TypeDef *) &DMA1->Ch7)
#endif /* MOCK_STM32_TESTS */

/* Peripheral Register Bit Definitions (CMSIS-style) */

/* RCC AHB Peripheral Clock Enable Register (RCC_AHBENR) */
#define RCC_AHBENR_DMA1EN_Pos     (0U)
#define RCC_AHBENR_DMA1EN_Msk     (0x1U << RCC_AHBENR_DMA1EN_Pos)
#define RCC_AHBENR_DMA1EN         RCC_AHBENR_DMA1EN_Msk

/* RCC APB2 Peripheral Clock Enable Register (RCC_APB2ENR) */
#define RCC_APB2ENR_IOPAEN_Pos    (2U)
#define RCC_APB2ENR_IOPAEN_Msk    (0x1U << RCC_APB2ENR_IOPAEN_Pos)
#define RCC_APB2ENR_IOPAEN        RCC_APB2ENR_IOPAEN_Msk
#define RCC_APB2ENR_IOPBEN_Pos    (3U)
#define RCC_APB2ENR_IOPBEN_Msk    (0x1U << RCC_APB2ENR_IOPBEN_Pos)
#define RCC_APB2ENR_IOPBEN        RCC_APB2ENR_IOPBEN_Msk
#define RCC_APB2ENR_USART1EN_Pos  (14U)
#define RCC_APB2ENR_USART1EN_Msk  (0x1U << RCC_APB2ENR_USART1EN_Pos)
#define RCC_APB2ENR_USART1EN      RCC_APB2ENR_USART1EN_Msk

/* RCC APB1 Peripheral Clock Enable Register (RCC_APB1ENR) */
#define RCC_APB1ENR_I2C1EN_Pos    (21U)
#define RCC_APB1ENR_I2C1EN_Msk    (0x1U << RCC_APB1ENR_I2C1EN_Pos)
#define RCC_APB1ENR_I2C1EN        RCC_APB1ENR_I2C1EN_Msk

/* USART Status Register (USART_SR) */
#define USART_SR_RXNE_Pos         (5U)
#define USART_SR_RXNE_Msk         (0x1U << USART_SR_RXNE_Pos)
#define USART_SR_RXNE             USART_SR_RXNE_Msk
#define USART_SR_TC_Pos           (6U)
#define USART_SR_TC_Msk           (0x1U << USART_SR_TC_Pos)
#define USART_SR_TC               USART_SR_TC_Msk
#define USART_SR_TXE_Pos          (7U)
#define USART_SR_TXE_Msk          (0x1U << USART_SR_TXE_Pos)
#define USART_SR_TXE              USART_SR_TXE_Msk

/* USART Control Register 1 (USART_CR1) */
#define USART_CR1_RE_Pos          (2U)
#define USART_CR1_RE_Msk          (0x1U << USART_CR1_RE_Pos)
#define USART_CR1_RE              USART_CR1_RE_Msk
#define USART_CR1_TE_Pos          (3U)
#define USART_CR1_TE_Msk          (0x1U << USART_CR1_TE_Pos)
#define USART_CR1_TE              USART_CR1_TE_Msk
#define USART_CR1_RXNEIE_Pos      (5U)
#define USART_CR1_RXNEIE_Msk      (0x1U << USART_CR1_RXNEIE_Pos)
#define USART_CR1_RXNEIE          USART_CR1_RXNEIE_Msk
#define USART_CR1_UE_Pos          (13U)
#define USART_CR1_UE_Msk          (0x1U << USART_CR1_UE_Pos)
#define USART_CR1_UE              USART_CR1_UE_Msk

/* USART Control Register 3 (USART_CR3) */
#define USART_CR3_DMAT_Pos        (7U)
#define USART_CR3_DMAT_Msk        (0x1U << USART_CR3_DMAT_Pos)
#define USART_CR3_DMAT            USART_CR3_DMAT_Msk

/* I2C Control Register 1 (I2C_CR1) */
#define I2C_CR1_PE_Pos            (0U)
#define I2C_CR1_PE_Msk            (0x1U << I2C_CR1_PE_Pos)
#define I2C_CR1_PE                I2C_CR1_PE_Msk
#define I2C_CR1_START_Pos         (8U)
#define I2C_CR1_START_Msk         (0x1U << I2C_CR1_START_Pos)
#define I2C_CR1_START             I2C_CR1_START_Msk
#define I2C_CR1_STOP_Pos          (9U)
#define I2C_CR1_STOP_Msk          (0x1U << I2C_CR1_STOP_Pos)
#define I2C_CR1_STOP              I2C_CR1_STOP_Msk
#define I2C_CR1_ACK_Pos           (10U)
#define I2C_CR1_ACK_Msk           (0x1U << I2C_CR1_ACK_Pos)
#define I2C_CR1_ACK               I2C_CR1_ACK_Msk
#define I2C_CR1_SWRST_Pos         (15U)
#define I2C_CR1_SWRST_Msk         (0x1U << I2C_CR1_SWRST_Pos)
#define I2C_CR1_SWRST             I2C_CR1_SWRST_Msk

/* I2C Control Register 2 (I2C_CR2) */
#define I2C_CR2_FREQ_Pos          (0U)
#define I2C_CR2_FREQ_Msk          (0x3FU << I2C_CR2_FREQ_Pos)
#define I2C_CR2_FREQ              I2C_CR2_FREQ_Msk
#define I2C_CR2_ITERREN_Pos       (8U)
#define I2C_CR2_ITERREN_Msk       (0x1U << I2C_CR2_ITERREN_Pos)
#define I2C_CR2_ITERREN           I2C_CR2_ITERREN_Msk
#define I2C_CR2_ITEVTEN_Pos       (9U)
#define I2C_CR2_ITEVTEN_Msk       (0x1U << I2C_CR2_ITEVTEN_Pos)
#define I2C_CR2_ITEVTEN           I2C_CR2_ITEVTEN_Msk
#define I2C_CR2_DMAEN_Pos         (11U)
#define I2C_CR2_DMAEN_Msk         (0x1U << I2C_CR2_DMAEN_Pos)
#define I2C_CR2_DMAEN             I2C_CR2_DMAEN_Msk
#define I2C_CR2_LAST_Pos          (12U)
#define I2C_CR2_LAST_Msk          (0x1U << I2C_CR2_LAST_Pos)
#define I2C_CR2_LAST              I2C_CR2_LAST_Msk

/* I2C Status Register 1 (I2C_SR1) */
#define I2C_SR1_SB_Pos            (0U)
#define I2C_SR1_SB_Msk            (0x1U << I2C_SR1_SB_Pos)
#define I2C_SR1_SB                I2C_SR1_SB_Msk
#define I2C_SR1_ADDR_Pos          (1U)
#define I2C_SR1_ADDR_Msk          (0x1U << I2C_SR1_ADDR_Pos)
#define I2C_SR1_ADDR              I2C_SR1_ADDR_Msk
#define I2C_SR1_BTF_Pos           (2U)
#define I2C_SR1_BTF_Msk           (0x1U << I2C_SR1_BTF_Pos)
#define I2C_SR1_BTF               I2C_SR1_BTF_Msk
#define I2C_SR1_ADD10_Pos         (3U)
#define I2C_SR1_ADD10_Msk         (0x1U << I2C_SR1_ADD10_Pos)
#define I2C_SR1_ADD10             I2C_SR1_ADD10_Msk
#define I2C_SR1_STOPF_Pos         (4U)
#define I2C_SR1_STOPF_Msk         (0x1U << I2C_SR1_STOPF_Pos)
#define I2C_SR1_STOPF             I2C_SR1_STOPF_Msk
#define I2C_SR1_RxNE_Pos          (6U)
#define I2C_SR1_RxNE_Msk          (0x1U << I2C_SR1_RxNE_Pos)
#define I2C_SR1_RxNE              I2C_SR1_RxNE_Msk
#define I2C_SR1_TxE_Pos           (7U)
#define I2C_SR1_TxE_Msk           (0x1U << I2C_SR1_TxE_Pos)
#define I2C_SR1_TxE               I2C_SR1_TxE_Msk
#define I2C_SR1_BERR_Pos          (8U)
#define I2C_SR1_BERR_Msk          (0x1U << I2C_SR1_BERR_Pos)
#define I2C_SR1_BERR              I2C_SR1_BERR_Msk
#define I2C_SR1_ARLO_Pos          (9U)
#define I2C_SR1_ARLO_Msk          (0x1U << I2C_SR1_ARLO_Pos)
#define I2C_SR1_ARLO              I2C_SR1_ARLO_Msk
#define I2C_SR1_AF_Pos            (10U)
#define I2C_SR1_AF_Msk            (0x1U << I2C_SR1_AF_Pos)
#define I2C_SR1_AF                I2C_SR1_AF_Msk
#define I2C_SR1_OVR_Pos           (11U)
#define I2C_SR1_OVR_Msk           (0x1U << I2C_SR1_OVR_Pos)
#define I2C_SR1_OVR               I2C_SR1_OVR_Msk
#define I2C_SR1_PECERR_Pos        (12U)
#define I2C_SR1_PECERR_Msk        (0x1U << I2C_SR1_PECERR_Pos)
#define I2C_SR1_PECERR            I2C_SR1_PECERR_Msk
#define I2C_SR1_TIMEOUT_Pos       (14U)
#define I2C_SR1_TIMEOUT_Msk       (0x1U << I2C_SR1_TIMEOUT_Pos)
#define I2C_SR1_TIMEOUT           I2C_SR1_TIMEOUT_Msk

/* I2C Status Register 2 (I2C_SR2) */
#define I2C_SR2_MSL_Pos           (0U)
#define I2C_SR2_MSL_Msk           (0x1U << I2C_SR2_MSL_Pos)
#define I2C_SR2_MSL               I2C_SR2_MSL_Msk
#define I2C_SR2_BUSY_Pos          (1U)
#define I2C_SR2_BUSY_Msk          (0x1U << I2C_SR2_BUSY_Pos)
#define I2C_SR2_BUSY              I2C_SR2_BUSY_Msk
#define I2C_SR2_TRA_Pos           (2U)
#define I2C_SR2_TRA_Msk           (0x1U << I2C_SR2_TRA_Pos)
#define I2C_SR2_TRA               I2C_SR2_TRA_Msk

/* I2C Clock Control Register (I2C_CCR) */
#define I2C_CCR_CCR_Pos           (0U)
#define I2C_CCR_CCR_Msk           (0xFFFU << I2C_CCR_CCR_Pos)
#define I2C_CCR_CCR               I2C_CCR_CCR_Msk
#define I2C_CCR_DUTY_Pos          (14U)
#define I2C_CCR_DUTY_Msk          (0x1U << I2C_CCR_DUTY_Pos)
#define I2C_CCR_DUTY              I2C_CCR_DUTY_Msk
#define I2C_CCR_FS_Pos            (15U)
#define I2C_CCR_FS_Msk            (0x1U << I2C_CCR_FS_Pos)
#define I2C_CCR_FS                I2C_CCR_FS_Msk

/* DMA Channel Control Register (DMA_CCR) */
#define DMA_CCR_EN_Pos            (0U)
#define DMA_CCR_EN_Msk            (0x1U << DMA_CCR_EN_Pos)
#define DMA_CCR_EN                DMA_CCR_EN_Msk
#define DMA_CCR_TCIE_Pos          (1U)
#define DMA_CCR_TCIE_Msk          (0x1U << DMA_CCR_TCIE_Pos)
#define DMA_CCR_TCIE              DMA_CCR_TCIE_Msk
#define DMA_CCR_HTIE_Pos          (2U)
#define DMA_CCR_HTIE_Msk          (0x1U << DMA_CCR_HTIE_Pos)
#define DMA_CCR_HTIE              DMA_CCR_HTIE_Msk
#define DMA_CCR_TEIE_Pos          (3U)
#define DMA_CCR_TEIE_Msk          (0x1U << DMA_CCR_TEIE_Pos)
#define DMA_CCR_TEIE              DMA_CCR_TEIE_Msk
#define DMA_CCR_DIR_Pos           (4U)
#define DMA_CCR_DIR_Msk           (0x1U << DMA_CCR_DIR_Pos)
#define DMA_CCR_DIR               DMA_CCR_DIR_Msk
#define DMA_CCR_CIRC_Pos          (5U)
#define DMA_CCR_CIRC_Msk          (0x1U << DMA_CCR_CIRC_Pos)
#define DMA_CCR_CIRC              DMA_CCR_CIRC_Msk
#define DMA_CCR_PINC_Pos          (6U)
#define DMA_CCR_PINC_Msk          (0x1U << DMA_CCR_PINC_Pos)
#define DMA_CCR_PINC              DMA_CCR_PINC_Msk
#define DMA_CCR_MINC_Pos          (7U)
#define DMA_CCR_MINC_Msk          (0x1U << DMA_CCR_MINC_Pos)
#define DMA_CCR_MINC              DMA_CCR_MINC_Msk
#define DMA_CCR_PSIZE_Pos         (8U)
#define DMA_CCR_PSIZE_Msk         (0x3U << DMA_CCR_PSIZE_Pos)
#define DMA_CCR_PSIZE             DMA_CCR_PSIZE_Msk
#define DMA_CCR_MSIZE_Pos         (10U)
#define DMA_CCR_MSIZE_Msk         (0x3U << DMA_CCR_MSIZE_Pos)
#define DMA_CCR_MSIZE             DMA_CCR_MSIZE_Msk
#define DMA_CCR_PL_Pos            (12U)
#define DMA_CCR_PL_Msk            (0x3U << DMA_CCR_PL_Pos)
#define DMA_CCR_PL                DMA_CCR_PL_Msk
#define DMA_CCR_MEM2MEM_Pos       (14U)
#define DMA_CCR_MEM2MEM_Msk       (0x1U << DMA_CCR_MEM2MEM_Pos)
#define DMA_CCR_MEM2MEM           DMA_CCR_MEM2MEM_Msk

/* DMA Interrupt Flag Clear Register (DMA_IFCR) */
#define DMA_IFCR_CGIF4_Pos        (12U)
#define DMA_IFCR_CGIF4_Msk        (0x1U << DMA_IFCR_CGIF4_Pos)
#define DMA_IFCR_CGIF4            DMA_IFCR_CGIF4_Msk
#define DMA_IFCR_CTCIF4_Pos       (13U)
#define DMA_IFCR_CTCIF4_Msk       (0x1U << DMA_IFCR_CTCIF4_Pos)
#define DMA_IFCR_CTCIF4           DMA_IFCR_CTCIF4_Msk
#define DMA_IFCR_CGIF6_Pos        (20U)
#define DMA_IFCR_CGIF6_Msk        (0x1U << DMA_IFCR_CGIF6_Pos)
#define DMA_IFCR_CGIF6            DMA_IFCR_CGIF6_Msk
#define DMA_IFCR_CTCIF6_Pos       (21U)
#define DMA_IFCR_CTCIF6_Msk       (0x1U << DMA_IFCR_CTCIF6_Pos)
#define DMA_IFCR_CTCIF6           DMA_IFCR_CTCIF6_Msk
#define DMA_IFCR_CGIF7_Pos        (24U)
#define DMA_IFCR_CGIF7_Msk        (0x1U << DMA_IFCR_CGIF7_Pos)
#define DMA_IFCR_CGIF7            DMA_IFCR_CGIF7_Msk
#define DMA_IFCR_CTCIF7_Pos       (25U)
#define DMA_IFCR_CTCIF7_Msk       (0x1U << DMA_IFCR_CTCIF7_Pos)
#define DMA_IFCR_CTCIF7           DMA_IFCR_CTCIF7_Msk

/* DMA Interrupt Status Register (DMA_ISR) */
#define DMA_ISR_TCIF4_Pos         (13U)
#define DMA_ISR_TCIF4_Msk         (0x1U << DMA_ISR_TCIF4_Pos)
#define DMA_ISR_TCIF4             DMA_ISR_TCIF4_Msk
#define DMA_ISR_TCIF6_Pos         (21U)
#define DMA_ISR_TCIF6_Msk         (0x1U << DMA_ISR_TCIF6_Pos)
#define DMA_ISR_TCIF6             DMA_ISR_TCIF6_Msk
#define DMA_ISR_TCIF7_Pos         (25U)
#define DMA_ISR_TCIF7_Msk         (0x1U << DMA_ISR_TCIF7_Pos)
#define DMA_ISR_TCIF7             DMA_ISR_TCIF7_Msk

#ifdef __cplusplus
}
#endif

#endif /* STM32F103XB_H */
