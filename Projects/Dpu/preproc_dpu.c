/**
  ******************************************************************************
  * @file    preproc_dpu.c
  * @author  MCD Application Team
  * @version v1.0.0
  * @date    05-Nov-2026
  * @brief   This file is implementing pre-processing functions that are making
  *          use of Audio pre-processing libraries
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

/* Includes ------------------------------------------------------------------*/

#include "preproc_dpu.h"
#include "user_mel_tables.h"
#include "logging.h"
#include "mcu_cache.h"
#include "cpu_stats.h"

/**
 * @brief Reports this translation unit's view of AudioProcCtx_t's size.
 * @retval sizeof(AudioProcCtx_t) as compiled here (layout guard).
 */
size_t PreProc_DPU_GetCtxSize(void)
{
  return sizeof(AudioProcCtx_t);
}

/**
 * @brief Reports this translation unit's view of PATCH_LEN.
 * @retval PATCH_LEN as compiled here (layout guard).
 */
uint32_t PreProc_DPU_GetPatchLen(void)
{
  return PATCH_LEN;
}

/**
 * @brief Initializes the audio preprocessing context.
 * @param pCtx Pointer to the preprocessing context to configure.
 *
 * Configures the FFT, spectrogram, Mel filter, and log-Mel spectrogram
 * instances using the application audio parameters. The persistent
 * spectrogram buffer is initialized with the output quantization offset.
 *
 * @retval 0 Initialization completed.
 */
int PreProc_DPUInit(AudioProcCtx_t *pCtx)
{
#ifdef APP_UART
  printf("\rPreprocessing\n\r");
  printf(SEPARATION_LINE_SHORT);
#endif
  assert_param(AI_SPECT_NFFT >= AI_SPECT_WIN_LEN);
  assert_param(AI_SPECT_NFFT >= AI_SPECT_NMEL);

  arm_rfft_fast_init_f16(&pCtx->S_Rfft, AI_SPECT_NFFT);
  pCtx->S_Spectr.pRfft                    = &pCtx->S_Rfft;
  pCtx->S_Spectr.Type                     = AI_SPECT_TYPE;
  pCtx->S_Spectr.pWindow                  = (float16_t *)AI_SPECT_WIN;
  pCtx->S_Spectr.SampRate                 = AI_SENSOR_ODR;
  pCtx->S_Spectr.FrameLen                 = AI_SPECT_WIN_LEN;
  pCtx->S_Spectr.FFTLen                   = AI_SPECT_NFFT;
  pCtx->S_Spectr.pScratch1                = pCtx->pSpectrScratchBuffer1;
  pCtx->S_Spectr.pScratch2                = pCtx->pSpectrScratchBuffer2;
  pCtx->S_Spectr.pad_left                 = 0;
  /* pad_left + FrameLen + pad_right must equal FFTLen (the FFT reads
   * exactly one full frame). Was AI_SPECT_WIN_LEN (768), overflowing the
   * 1024-sample pScratch1/pScratch2 buffers by 512 samples; H7 uses this
   * same NFFT-WIN_LEN expression. */
  pCtx->S_Spectr.pad_right                = AI_SPECT_NFFT - AI_SPECT_WIN_LEN;
  pCtx->S_MelFilter.pStartIndices         = (uint32_t *)AI_SPECT_MEL_START_IDX;
  pCtx->S_MelFilter.pStopIndices          = (uint32_t *)AI_SPECT_MEL_STOP_IDX;
  pCtx->S_MelFilter.pCoefficients         = (float16_t *)AI_SPECT_MEL_LUT;
  pCtx->S_MelFilter.NumMels               = AI_SPECT_NMEL;
  pCtx->S_MelFilter.FFTLen                = AI_SPECT_NFFT;
  pCtx->S_MelFilter.SampRate              = AI_SENSOR_ODR;
  pCtx->S_MelFilter.FMin                  = AI_SPECT_FMIN;
  pCtx->S_MelFilter.FMax                  = AI_SPECT_FMAX;
  pCtx->S_MelFilter.Formula               = AI_SPECT_FORMULA;
  pCtx->S_MelFilter.Normalize             = AI_SPECT_NORMALIZE;
  pCtx->S_MelFilter.Mel2F                 = 1U;
  pCtx->S_MelSpectr.SpectrogramConf       = &pCtx->S_Spectr;
  pCtx->S_MelSpectr.MelFilter             = &pCtx->S_MelFilter;
  pCtx->S_LogMelSpectr.MelSpectrogramConf = &pCtx->S_MelSpectr;
  pCtx->S_LogMelSpectr.LogFormula         = AI_SPECT_LOG_FORMULA;
  pCtx->S_LogMelSpectr.Ref                = 1.0f;
  pCtx->S_LogMelSpectr.TopdB              = HUGE_VALF;

  printf("MEL spectrogram %d mel x %d col\n\r", AI_SPECT_NMEL, AI_SPECT_COL);
  printf("- sampling freq : %lu Hz\n\r", (uint32_t)AI_SENSOR_ODR);
  printf("- acq period    : %lu ms\n\r", (uint32_t)AUDIO_ACQ_LEN_MS);
  printf("- window length : %u samples\n\r", AI_SPECT_WIN_LEN);
  printf("- hop length    : %u samples\n\r", AI_SPECT_HOP_LEN);

#ifdef APP_LOW_LAT
  for (int i = 0; i < AI_SPECT_COL * (AI_SPECT_NMEL + 1); i++)
  {
    pCtx->spect[i] = pCtx->output_Q_offset;
  }
#endif /* APP_LOW_LAT */

  /* Start pre-emphasis from silence; carried across calls from
   * here on (see PreProc_DPU()). */
  pCtx->preemp_prev = 0.0F;

  return 0;
}

/**
 * @brief Converts an audio patch into a quantized log-Mel spectrogram.
 * @param pxCtx Pointer to the initialized preprocessing context.
 * @param pDataIn Pointer to the input PCM audio patch.
 * @param p_spectro Pointer to the destination quantized spectrogram buffer.
 *
 * Applies pre-emphasis, FFT, Mel filtering, logarithmic scaling, normalization,
 * and quantization. When low-latency mode is enabled, the function retains
 * overlapping spectrogram columns from the previous audio patch.
 *
 * @retval 1 Processing completed.
 */
int PreProc_DPU(AudioProcCtx_t *pxCtx, uint8_t *pDataIn, int8_t *p_spectro)
{
  assert_param(pxCtx != NULL);
  assert_param(pDataIn != NULL);
  assert_param(p_spectro != NULL);

  port_dwt_reset();

  float16_t out[AI_SPECT_NMEL];
  int16_t p_out_preemp[PATCH_LEN];
  int16_t *p_in;

  FFTTypeDef_f16_t sFFT = {
    .pRfft = pxCtx->S_LogMelSpectr.MelSpectrogramConf->SpectrogramConf->pRfft,
    .pWindow = pxCtx->S_LogMelSpectr.MelSpectrogramConf->SpectrogramConf->pWindow,
    .pScratch = pxCtx->S_LogMelSpectr.MelSpectrogramConf->SpectrogramConf->pScratch1,
    .FFTLen = pxCtx->S_LogMelSpectr.MelSpectrogramConf->SpectrogramConf->FFTLen,
    .sDin.FrameLen = pxCtx->S_LogMelSpectr.MelSpectrogramConf->SpectrogramConf->FrameLen,
    .sDin.pad_left = pxCtx->S_LogMelSpectr.MelSpectrogramConf->SpectrogramConf->pad_left,
    .sDin.pad_right = pxCtx->S_LogMelSpectr.MelSpectrogramConf->SpectrogramConf->pad_right
  };

  FFTTypeDef_f16_t *pFFT = &sFFT;
  pxCtx->S_Spectr.spectro_sum = 0;

  /* Create a quantized Mel-scaled spectrogram column. */

  /* Pre-emphasis filter: y[n] = x[n] - 0.97 * x[n-1]. x[-1] is carried across
   * calls in pxCtx->preemp_prev instead of being reset to 0 every
   * call, which otherwise injects a discontinuity at the start of every
   * patch. */
  int16_t *p_in_preemp = (int16_t *)pDataIn;
  float xn_1 = pxCtx->preemp_prev;
  for (uint32_t i = 0; i < PATCH_LEN; i++)
  {
    float xn = (float)p_in_preemp[i];

    p_out_preemp[i] = (int16_t)(xn - 0.97F * xn_1);
    xn_1 = xn;
  }
  /* The next call's window is this patch shifted left by AUDIO_ACQ_LEN
   * samples (see audio_bm.c's audio_process(): proc_buff[0..AUDIO_ACQ_OFFSET-1]
   * <- proc_buff[AUDIO_ACQ_LEN..PATCH_LEN-1], new samples appended after), so
   * its true predecessor sample is this patch's p_in_preemp[AUDIO_ACQ_LEN - 1],
   * not the last sample of this patch (p_in_preemp[PATCH_LEN - 1]) - those
   * only coincide when AUDIO_ACQ_LEN == PATCH_LEN (APP_LOW_LAT_OFF, no
   * overlap). */
  pxCtx->preemp_prev = (float)p_in_preemp[AUDIO_ACQ_LEN - 1];

#ifdef APP_LOW_LAT
#define AI_SPECT_OVL_LEN ((AI_SPECT_COL - 1) * (AI_SPECT_NMEL + 1))
#define AI_SPECT_OFFSET  (AI_SPECT_NMEL + 1)

  /* Prepare overlapping samples from the previous patch (source and
   * destination overlap, so memmove is required). */
  memmove(pxCtx->spect, &pxCtx->spect[AI_SPECT_OFFSET], AI_SPECT_OVL_LEN);
  p_in = p_out_preemp;

  audio_is16of16_fft(p_in, pFFT,
                      pxCtx->S_LogMelSpectr.MelSpectrogramConf->SpectrogramConf->pScratch2);

  LogMelSpectrogramColumn_f16(&pxCtx->S_LogMelSpectr,
                              pxCtx->S_LogMelSpectr.MelSpectrogramConf->SpectrogramConf->pScratch2,
                              out);
  /* Normalize, quantize, and transpose. */

  float16_t inv_scale = pxCtx->output_Q_inv_scale;
  int8_t offset = pxCtx->output_Q_offset;
  for (uint32_t j = 0; j < pxCtx->S_MelFilter.NumMels; j++)
  {
    out[j] = (out[j] - user_melFiltersmeans[j]) / user_melFiltersstds[j];
    pxCtx->spect[AI_SPECT_OVL_LEN + j] =
      (int8_t)__SSAT((int16_t)roundf(out[j] * inv_scale + offset), 8);
  }
  /* Fill the pitch entry (last element of the newest column) with zero. */
  pxCtx->spect[AI_SPECT_OVL_LEN + AI_SPECT_NMEL] = offset;

  /* Transfer the result to the output. */
  memcpy(p_spectro, pxCtx->spect, AI_SPECT_OVL_LEN + AI_SPECT_OFFSET);

#else /* not APP_LOW_LAT */
  for (uint32_t i = 0; i < AI_SPECT_COL; i++)
  {
    p_in = p_out_preemp + AI_SPECT_HOP_LEN * i;

    audio_is16of16_fft(p_in, pFFT,
                        pxCtx->S_LogMelSpectr.MelSpectrogramConf->SpectrogramConf->pScratch2);

    LogMelSpectrogramColumn_f16(&pxCtx->S_LogMelSpectr,
                                pxCtx->S_LogMelSpectr.MelSpectrogramConf->SpectrogramConf->pScratch2,
                                out);
    /* Normalize, quantize, and transpose. */
    float16_t inv_scale = pxCtx->output_Q_inv_scale;
    int8_t offset = pxCtx->output_Q_offset;
    for (uint32_t j = 0; j < pxCtx->S_MelFilter.NumMels; j++)
    {
      out[j] = (out[j] - user_melFiltersmeans[j]) / user_melFiltersstds[j];
      p_spectro[i * (AI_SPECT_NMEL + 1) + j] =
        (int8_t)__SSAT((int16_t)roundf(out[j] * inv_scale + offset), 8);
    }
    /* Fill the pitch column with zero. */
    p_spectro[(i + 1) * (AI_SPECT_NMEL + 1)-1] = offset;
  }
#endif /* APP_LOW_LAT */

  time_stats_store(TIME_STAT_PRE_PROC,
                   port_dwt_get_cycles() * 1000.0F / port_hal_get_cpu_freq());

  return 1;
}
