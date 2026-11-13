/**
  ******************************************************************************
  * @file    app_config.h
  * @author  GPM/AIS Application Team
  * @version v1.0.0
  * @date    05-Nov-2026
  * @brief   APP configuration
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

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "logging.h"

#define LOG_LEVEL LOG_INFO

#define my_printf LogInfo

/* UART usage/configuration */
#define USE_UART_BAUDRATE (921600)

#define APP_CONF_STR "Bare Metal"

/* Overridable via Makefile's EXTRA_DEFS, e.g. from a CI build's git describe. */
#ifndef APP_GIT_SHA1_STRING
#define APP_GIT_SHA1_STRING "dev"
#endif /* !APP_GIT_SHA1_STRING */
#ifndef APP_VERSION_STRING
#define APP_VERSION_STRING "unversioned"
#endif /* !APP_VERSION_STRING */

/* Low-latency: one new mel column per 16 ms hop instead of three per 48 ms.
 * Opt out with -DAPP_LOW_LAT_OFF (default on). */
#define APP_LOW_LAT
#ifdef APP_LOW_LAT_OFF
#undef APP_LOW_LAT
#endif /* APP_LOW_LAT_OFF */

#define APP_LCD
#define APP_UART
#define APP_REFRESH_CPU   (1000.0F)
#define APP_REFRESH_VOICE (1000.0F)

#define APP_VAD_THR (80)

#ifdef AI_MODEL_FULL
#define APP_MODEL_STR "full"
#else
#define APP_MODEL_STR "partial"
#endif /* AI_MODEL_FULL */

#ifdef APP_LOW_LAT
#define APP_LAT_STR "low-latency"
#else
#define APP_LAT_STR "high-latency"
#endif /* APP_LOW_LAT */

#define SEPARATION_LINE "---------------------------------------------------------------\n\r"
#define SEPARATION_LINE_SHORT "------------------------------\n\r"

#ifndef USE_NPU_CACHE
  #define USE_NPU_CACHE 1
#endif /* !USE_NPU_CACHE */

#endif /* APP_CONFIG_H */
