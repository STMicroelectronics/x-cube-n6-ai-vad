/**
  ******************************************************************************
  * @file    misc_toolbox.h
  * @author  MCD Application Team
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023-2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef MISC_TOOLBOX_H
#define MISC_TOOLBOX_H

/* Includes ------------------------------------------------------------------*/
#include "stm32n6xx_hal.h"
#if NUCLEO_N6_CONFIG == 0
#include "stm32n6570_discovery.h"
#include "stm32n6570_discovery_bus.h"
#include "stm32n6570_discovery_xspi.h"
#else /* not NUCLEO_N6_CONFIG */
#include "stm32n6xx_nucleo.h"
#include "stm32n6xx_nucleo_bus.h"
#include "stm32n6xx_nucleo_xspi.h"
#endif /* NUCLEO_N6_CONFIG */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
extern UART_HandleTypeDef UartHandle;
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/**
 * @brief Keeps required peripheral and memory clocks enabled during sleep.
 */
void set_clk_sleep_mode(void);

/**
 * @brief Raises the VDDCore level for overdrive operation.
 */
void upscale_vddcore_level(void);

/**
 * @brief Configures USART1 and its GPIO pins for application logging.
 */
void UART_Config(void);

/**
 * @brief Configures NPU clocks, cache, and RIF access attributes.
 */
void NPU_Config(void);

/**
 * @brief Configures the used RISAF instances for unrestricted application access.
 */
void RISAF_Config(void);

/**
 * @brief Sets the application vector-table address and resets interrupt state.
 */
void set_vector_table_addr(void);

/**
 * @brief Performs board initialization after startup.
 *
 * Enables NPU RAMs and allows data and instruction caches to be activated.
 */
void system_init_post(void);

#ifdef HAL_BSEC_MODULE_ENABLED
/**
 * @brief Programs OTP settings for VDDIO2 and VDDIO3 low-voltage I/O mode.
 */
void fuse_vddio(void);
#endif /* HAL_BSEC_MODULE_ENABLED */

#endif /* MISC_TOOLBOX_H */