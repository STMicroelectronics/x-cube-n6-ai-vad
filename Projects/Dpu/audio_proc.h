/**
  ******************************************************************************
  * @file    audio_proc.h
  * @author  MCD Application Team
  * @version v1.0.0
  * @date    05-Nov-2026
  * @brief   Header for the audio preprocessing context module.
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
#ifndef AUDIO_PROC_H
#define AUDIO_PROC_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "ai_device_adaptor.h"
#include "dpu_config.h"
#include "feature_extraction_f16.h"
#include "mel_filterbank_f16.h"
#include "audio_prePost_process_f16.h"
#include "feature_extraction.h"
#include "mel_filterbank.h"
#include "audio_prePost_process.h"

/* Exported types ------------------------------------------------------------*/
typedef struct
{
  arm_rfft_fast_instance_f16   S_Rfft;
  SpectrogramTypeDef_f16       S_Spectr;
  MelSpectrogramTypeDef_f16    S_MelSpectr;
  LogMelSpectrogramTypeDef_f16 S_LogMelSpectr;
  MelFilterTypeDef_f16         S_MelFilter;
  float16_t                    pSpectrScratchBuffer1[AI_SPECT_NFFT];
  float16_t                    pSpectrScratchBuffer2[AI_SPECT_NFFT];
  int8_t                       output_Q_offset;
  float16_t                    output_Q_inv_scale;
  /* Pre-emphasis x[-1] carried across calls instead of being reset to
   * 0 every call, which otherwise injects a discontinuity at the start of
   * every patch. */
  float                        preemp_prev;
#ifdef APP_LOW_LAT
  int8_t                       spect[AI_SPECT_COL * (AI_SPECT_NMEL + 1)];
#endif /* APP_LOW_LAT */
}AudioProcCtx_t;

#endif /* AUDIO_PROC_H */
