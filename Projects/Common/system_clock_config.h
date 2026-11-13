/**
  ******************************************************************************
  * @file    system_clock_config.h
  * @author  GPM/AIS Application Team
  * @version v1.0.0
  * @date    05-Nov-2026
  * @brief   system clock config
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SYSTEM_CLOCK_CONFIG_H
#define SYSTEM_CLOCK_CONFIG_H

/* Exported macros -----------------------------------------------------------*/
#define SystemClock_Config_Full SystemClock_Config_HSI_no_overdrive
#define SystemClock_Config_Low  SystemClock_Config_MSI_4MHz_4MHz_exp

/* Exported functions --------------------------------------------------------*/
/**
 * @brief Restores system clocks and oscillators to their reset configuration.
 */
void SystemClock_Config_ResetClocks(void);

/**
 * @brief Configures MSI as the 4 MHz CPU and system clock source.
 */
void SystemClock_Config_MSI_4MHz_4MHz_exp(void);

/**
 * @brief Configures the HSI clock tree for overdrive operation.
 */
void SystemClock_Config_HSI_overdrive(void);

/**
 * @brief Configures the HSI clock tree without overdrive.
 */
void SystemClock_Config_HSI_no_overdrive(void);

#endif /* SYSTEM_CLOCK_CONFIG_H */
