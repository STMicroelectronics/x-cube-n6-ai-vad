/**
  ******************************************************************************
  * @file    audio_bm.h
  * @author  GPM/AIS Application Team
  * @version v1.0.0
  * @date    05-Nov-2026
  * @brief   Public interface for the bare-metal audio processing application.
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

#ifndef APP_AUDIO_BM_H
#define APP_AUDIO_BM_H
#include "app_config.h"
#include "AudioCapture_ring_buff.h"
#include "stm32n6570_discovery.h"
#include "stm32n6570_discovery_audio.h"
#include "preproc_dpu.h"
#include "ai_dpu.h"

/**
 * @brief Holds audio acquisition buffers and the capture ring buffer.
 */
typedef struct _AudioBM_acq_t
{
  int16_t acq_buf[CAPTURE_BUFFER_SIZE];
  AudioCapture_ring_buff_t ring_buff;
} AudioBM_acq_t;

/**
 * @brief Holds audio preprocessing and AI inference buffers and contexts.
 */
typedef struct _AudioBM_proc_t
{
  int16_t proc_buff[PATCH_LEN];
  AudioProcCtx_t audioPreCtx;
  AIProcCtx_t aiCtx;
  int8_t *ai_in_ptr;
  int8_t *ai_out_ptr;
} AudioBM_proc_t;

/**
 * @brief Initializes the bare-metal audio application and its peripherals.
 */
void init_bm(void);

/**
 * @brief Runs the bare-metal audio processing loop.
 *
 * This function does not return during normal operation.
 */
void exec_bm(void);

#endif /* APP_AUDIO_BM_H */
