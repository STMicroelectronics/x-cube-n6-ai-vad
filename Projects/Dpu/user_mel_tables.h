/**
  ******************************************************************************
  * @file    user_mel_tables.h
  * @author  MCD Application Team
  * @brief   Header for the user Mel lookup tables module.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MEL_USER_TABLES_H
#define MEL_USER_TABLES_H

/* Includes ------------------------------------------------------------------*/
#include "arm_math.h"
#include "preproc_dpu.h"

/* Exported constants --------------------------------------------------------*/
extern const float16_t user_win[768];
extern const float16_t user_melFiltersLut[949];
extern const float16_t user_melFiltersmeans[40];
extern const float16_t user_melFiltersstds[40];
extern const uint32_t user_melFiltersStartIndices[40];
extern const uint32_t user_melFiltersStopIndices[40];

#endif /* MEL_USER_TABLES_H */
