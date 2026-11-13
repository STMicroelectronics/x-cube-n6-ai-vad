/**
  ******************************************************************************
  * @file    dpu_config.h
  * @author  STMicroelectronics - AIS - MCD Team
  * @version v1.0.0
  * @date    05-Nov-2026
  * @brief   Configure the getting started functionality
  *
  * Each logic module of the application should define a DEBUG control byte
  * used to turn on/off the log for the module.
  *
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
#ifndef DPU_CONFIG_H_
#define DPU_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "ai_model_config.h"

/* Exported constants --------------------------------------------------------*/
#ifndef AI_SENSOR_ODR
#define AI_SENSOR_ODR          (16000.0F)
#endif /* !AI_SENSOR_ODR */
#ifndef AI_SPECT_NMEL
#define AI_SPECT_NMEL          (40U)
#endif /* !AI_SPECT_NMEL */
#ifndef AI_SPECT_HOP_LEN
#define AI_SPECT_HOP_LEN       (256U)
#endif /* !AI_SPECT_HOP_LEN */
#ifndef AI_SPECT_NFFT
#define AI_SPECT_NFFT          (1024U)
#endif /* !AI_SPECT_NFFT */
#ifndef AI_SPECT_WIN_LEN
#define AI_SPECT_WIN_LEN       (768U)
#endif /* !AI_SPECT_WIN_LEN */
#ifndef AI_SPECT_NORMALIZE
#define AI_SPECT_NORMALIZE     (1U)
#endif /* !AI_SPECT_NORMALIZE */
#ifndef AI_SPECT_FORMULA
#define AI_SPECT_FORMULA       (MEL_HTK)
#endif /* !AI_SPECT_FORMULA */
#ifndef AI_SPECT_FMIN
#define AI_SPECT_FMIN          (125U)
#endif /* !AI_SPECT_FMIN */
#ifndef AI_SPECT_FMAX
#define AI_SPECT_FMAX          (7500U)
#endif /* !AI_SPECT_FMAX */
#ifndef AI_SPECT_TYPE
#define AI_SPECT_TYPE          (SPECTRUM_TYPE_POWER)
#endif /* !AI_SPECT_TYPE */
#ifndef AI_SPECT_LOG_FORMULA
#define AI_SPECT_LOG_FORMULA   (LOGMELSPECTROGRAM_SCALE_LOG) /* LOGMELSPECTROGRAM_SCALE_DB */
#endif /* !AI_SPECT_LOG_FORMULA */
#ifndef AI_SPECT_WIN
#define AI_SPECT_WIN           (user_win)
#endif /* !AI_SPECT_WIN */
#ifndef AI_SPECT_MEL_LUT
#define AI_SPECT_MEL_LUT       (user_melFiltersLut)
#endif /* !AI_SPECT_MEL_LUT */
#ifndef AI_SPECT_MEL_START_IDX
#define AI_SPECT_MEL_START_IDX (user_melFiltersStartIndices)
#endif /* !AI_SPECT_MEL_START_IDX */
#ifndef AI_SPECT_MEL_STOP_IDX
#define AI_SPECT_MEL_STOP_IDX  (user_melFiltersStopIndices)
#endif /* !AI_SPECT_MEL_STOP_IDX */

#ifdef APP_LOW_LAT
#define AUDIO_ACQ_LEN (AI_SPECT_HOP_LEN)
#else /* not APP_LOW_LAT */
#define AUDIO_ACQ_LEN (AI_SPECT_COL * AI_SPECT_HOP_LEN)
#endif /* APP_LOW_LAT */

#define AUDIO_ACQ_LEN_MS ((uint32_t)(1000.0F * (float)AUDIO_ACQ_LEN / (float)AI_SENSOR_ODR))

typedef enum
{
  DPU_OK = 0x00,
  DPU_ERROR = 0x01,
} DPU_StatusTypeDef;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DPU_CONFIG_H_ */
