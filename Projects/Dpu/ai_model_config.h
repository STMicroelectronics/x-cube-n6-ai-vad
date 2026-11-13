/**
  ******************************************************************************
  * @file    ai_model_config.h
  * @author  STMicroelectronics - AIS - MCD Team
  * @version v1.0.0
  * @date    05-Nov-2026
  * @brief   Defines the audio preprocessing model configuration.
  *
  *  Each logic module of the application should define a DEBUG control byte
  * used to turn on/off the log for the module.
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022-2025 STMicroelectronics
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file in
  * the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */

/* Generated code -----------------------------------------------------------*/
#ifndef AI_MODEL_CONFIG_H
#define AI_MODEL_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Exported constants --------------------------------------------------------*/
#define AI_SENSOR_ODR          (16000.0F)
#define AI_SPECT_NMEL          (40U)
#define AI_SPECT_COL           (3U)
#define AI_SPECT_HOP_LEN       (256U)
#define AI_SPECT_NFFT          (1024U)
#define AI_SPECT_WIN_LEN       (768U)
#define AI_SPECT_NORMALIZE     (0U) /* (1U) */
#define AI_SPECT_FORMULA       (MEL_HTK) /* MEL_SLANEY */
#define AI_SPECT_FMIN          (125U)
#define AI_SPECT_FMAX          (7500U)
#define AI_SPECT_TYPE          (SPECTRUM_TYPE_POWER)
#define AI_SPECT_LOG_FORMULA   (LOGMELSPECTROGRAM_SCALE_LOG) /* LOGMELSPECTROGRAM_SCALE_DB */
#define AI_SPECT_WIN           (user_win)
#define AI_SPECT_MEL_LUT       (user_melFiltersLut)
#define AI_SPECT_MEL_START_IDX (user_melFiltersStartIndices)
#define AI_SPECT_MEL_STOP_IDX  (user_melFiltersStopIndices)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_MODEL_CONFIG_H */
