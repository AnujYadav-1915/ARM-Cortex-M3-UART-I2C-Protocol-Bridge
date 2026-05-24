/**
 ******************************************************************************
 * @file    core_cm3.h
 * @brief   CMSIS Cortex-M3 Core Peripheral Access Layer Header File.
 *          Provides instruction intrinsics and barriers.
 ******************************************************************************
 */

#ifndef CORE_CM3_H
#define CORE_CM3_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Interrupt Control and State Register */
#define SCB_ICSR_PENDSVSET_Pos     28
#define SCB_ICSR_PENDSVSET_Msk     (1UL << SCB_ICSR_PENDSVSET_Pos)

/* Compiler Intrinsics and Memory Barriers */
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
  /* Keil Compiler / ARM Compiler */
  #define __ASM            __asm
  #define __INLINE         __inline
  #define __STATIC_INLINE  static __inline
  
  #define __DMB()          __dmb(0xF)
  #define __WFI()          __wfi()

#elif defined(__GNUC__)
  /* GNU Compiler (GCC) */
  #define __ASM            __asm
  #define __INLINE         inline
  #define __STATIC_INLINE  static inline

  #ifndef MOCK_STM32_TESTS
    /* Real ARM GCC Target */
    __STATIC_INLINE void __DMB(void) {
        __asm volatile ("dmb 0xF" : : : "memory");
    }
    __STATIC_INLINE void __WFI(void) {
        __asm volatile ("wfi" : : : "memory");
    }
  #else
    /* Host GCC Simulation Fallbacks (Mocks) */
    __STATIC_INLINE void __DMB(void) {
        /* Compiler barrier only on host to prevent reordering */
        __asm volatile ("" : : : "memory");
    }
    __STATIC_INLINE void __WFI(void) {
        /* No-op on host */
    }
  #endif /* MOCK_STM32_TESTS */

#else
  /* Other compiler fallback */
  #define __STATIC_INLINE  static inline
  #define __DMB()          do {} while(0)
  #define __WFI()          do {} while(0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* CORE_CM3_H */
