/**
  ******************************************************************************
  * @file    logging.h
  * @version v1.0.0
  * @date    05-Nov-2026
  * @brief   This file contains basic implememtation for a logger
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
#ifndef LOGGING_H
#define LOGGING_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define LOG_NONE     0
#define LOG_ERROR    1
#define LOG_WARN     2
#define LOG_INFO     3
#define LOG_DEBUG    4

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>

/**
 * @brief Prints a formatted log message when its level is enabled.
 * @param LogLevel Severity level of the message.
 * @param format Printf-style format string.
 * @param ... Format arguments.
 */
void LogPrintf(int LogLevel, char *format, ...);

/* Exported macro ------------------------------------------------------------*/
#define LogError(...)  do { LogPrintf(LOG_ERROR, __VA_ARGS__); } while (0)
#define LogWarn(...)   do { LogPrintf(LOG_WARN, __VA_ARGS__); } while (0)
#define LogInfo(...)   do { LogPrintf(LOG_INFO, __VA_ARGS__); } while (0)
#define LogDebug(...)  do { LogPrintf(LOG_DEBUG, __VA_ARGS__); } while (0)

/**
 * @brief Converts a log level to its printable name.
 * @param LogLevel Severity level to convert.
 * @return Constant string describing the log level, or "Invalid" when unknown.
 */
extern const char *getLogLevelStr(int LogLevel);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LOGGING_H */
