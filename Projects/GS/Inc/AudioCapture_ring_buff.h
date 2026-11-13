/**
  ******************************************************************************
  * @file    AudioCapture_ring_buff.h
  * @author  MCD Application Team
  * @version v1.0.0
  * @date    05-Nov-2026
  * @brief   Header for AudioCapture_ring_buff.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifndef AUDIO_CAPTURE_RING_BUFF_H
#define AUDIO_CAPTURE_RING_BUFF_H

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/**
 * @brief Lock-free circular buffer used between DMA audio capture and processing.
 */
typedef struct
{
  uint8_t *pData;                 /* Pointer to the start of the circular buffer. */
  uint8_t nbBytesPerSample;       /* Number of bytes per sample. */
  uint32_t nbFrames;              /* Number of frames in the circular buffer. */
  uint32_t nbSamples;             /* Number of samples per frame. */
  uint32_t readSampleIndex;       /* Index of the next sample to read. */
  uint32_t writeSampleIndex;      /* Index of the next sample to write. */
  uint32_t availableSamples;      /* Number of samples available for consumption. */
} AudioCapture_ring_buff_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */



/** @brief Resets a ring-buffer descriptor without releasing its storage. */
void AudioCapture_ring_buff_reset(AudioCapture_ring_buff_t *pHdle);

/** @brief Allocates storage for a ring buffer from its configured geometry. */
void AudioCapture_ring_buff_alloc(AudioCapture_ring_buff_t *pHdle);

/** @brief Releases the storage allocated for a ring buffer. */
void AudioCapture_ring_buff_free(AudioCapture_ring_buff_t *pHdle);

/**
 * @brief Writes samples to the ring buffer.
 * @param pHdle Ring-buffer descriptor to update.
 * @param pData Source sample buffer.
 * @param nbSamples Number of samples to write.
 */
void AudioCapture_ring_buff_feed(AudioCapture_ring_buff_t *pHdle, uint8_t *pData, int16_t nbSamples);

/**
 * @brief Copies samples from the ring buffer to a destination buffer.
 * @param pData Destination sample buffer.
 * @param pHdle Ring-buffer descriptor to consume.
 * @param nbSamples Number of samples to consume.
 * @return Pointer to @p pData.
 */
uint8_t *AudioCapture_ring_buff_consume(uint8_t *pData,
                                        AudioCapture_ring_buff_t *pHdle,
                                        uint32_t nbSamples);

/**
 * @brief Consumes contiguous samples without copying them.
 * @param pHdle Ring-buffer descriptor to consume.
 * @param nbSamples Number of samples to consume.
 * @return Pointer to the consumed samples in the ring buffer.
 */
uint8_t *AudioCapture_ring_buff_consume_no_cpy(AudioCapture_ring_buff_t *pHdle,
                                                uint32_t nbSamples);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AUDIO_CAPTURE_RING_BUFF_H */

