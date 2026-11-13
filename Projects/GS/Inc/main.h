/**
  ******************************************************************************
  * @file    main.h
  * @author  MCD Application Team
  * @version v1.0.0
  * @date    05-Nov-2026
  * @brief   Header for main.c module
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
#ifndef MAIN_H
#define MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32n6xx_hal.h"

/**
 * @brief Stops application execution after an unrecoverable error.
 */
void Error_Handler(void);

#endif /* MAIN_H */

