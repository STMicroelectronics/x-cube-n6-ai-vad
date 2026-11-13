/**
  ******************************************************************************
  * @file    stm32n6xx_it.h
  * @author  MCD Application Team
  * @version v1.0.0
  * @date    05-Nov-2026
  * @brief   This file contains the headers of the interrupt handlers.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32N6XX_IT_H
#define STM32N6XX_IT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/** @brief Handles the non-maskable interrupt. */
void NMI_Handler(void);
/** @brief Handles hard-fault exceptions. */
void HardFault_Handler(void);
/** @brief Handles memory-management faults. */
void MemManage_Handler(void);
/** @brief Handles bus faults. */
void BusFault_Handler(void);
/** @brief Handles usage faults. */
void UsageFault_Handler(void);
/** @brief Handles secure faults. */
void SecureFault_Handler(void);
/** @brief Handles supervisor calls. */
void SVC_Handler(void);
/** @brief Handles debug-monitor exceptions. */
void DebugMon_Handler(void);
/** @brief Handles pending supervisor calls. */
void PendSV_Handler(void);
/** @brief Handles system-tick interrupts. */
void SysTick_Handler(void);
/** @brief Services the audio-output DMA-complete interrupt. */
void GPDMA1_Channel2_IRQHandler(void);
/** @brief Services the digital-microphone DMA interrupt. */
void GPDMA1_Channel0_IRQHandler(void);
/** @brief Services the user-button external interrupt. */
void EXTI13_IRQHandler(void);
/** @brief Services the tamper-button external interrupt. */
void EXTI0_IRQHandler(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* STM32N6XX_IT_H */

