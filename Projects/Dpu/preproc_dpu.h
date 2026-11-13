/**
  ******************************************************************************
  * @file    preproc_dpu.h
  * @author  MCD Application Team
  * @version v1.0.0
  * @date    05-Nov-2026
  * @brief   Header for preproc_dpu.c module
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
#ifndef PREPROC_DPU_H
#define PREPROC_DPU_H

/* Includes ------------------------------------------------------------------*/
/* APP_LOW_LAT must be visible here before audio_proc.h (-> dpu_config.h)
 * resolves AUDIO_ACQ_LEN, or this translation unit silently disagrees with any
 * TU that does see it -- mirrors H7's Dpu/Inc/dpu.h and preproc_dpu.h. */
#include "app_config.h"
#include "audio_proc.h"

/* Exported constants --------------------------------------------------------*/
#ifdef APP_LOW_LAT
#define PATCH_LEN (AI_SPECT_WIN_LEN)
#else /* not APP_LOW_LAT */
#define PATCH_LEN (AI_SPECT_WIN_LEN + (AI_SPECT_COL - 1) * AI_SPECT_HOP_LEN)
#endif /* APP_LOW_LAT */

/* Exported functions --------------------------------------------------------*/
int PreProc_DPUInit(AudioProcCtx_t *pxCtx);
int PreProc_DPU(AudioProcCtx_t *pxCtx, uint8_t *pDataIn, int8_t *p_spectro);

/* Layout guard: report this translation unit's own view of the
 * APP_LOW_LAT-dependent layout, so a caller (audio_bm.c) can assert it
 * against its own view instead of trusting that every TU sees the same
 * macro. */
size_t PreProc_DPU_GetCtxSize(void);
uint32_t PreProc_DPU_GetPatchLen(void);

#endif /* PREPROC_DPU_H */
