/**
  ******************************************************************************
  * @file    cpu_stats.h
  * @version v1.0.0
  * @date    05-Nov-2026
  * @brief
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef CPU_STATS_H
#define CPU_STATS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Exported macro ------------------------------------------------------------*/
/* Define the maximum number of log entries */
#define TIME_STAT_PRE_PROC   (0)
#define TIME_STAT_AI_PROC    (1)
#define TIME_STAT_POST_PROC  (2)
#define TIME_STAT_TOT        (3)
#define TIME_STAT_NB_ENTRIES (TIME_STAT_TOT + 1)

/**
 * @brief Initializes all timing-statistics entries.
 *
 * Clears the statistics buffer and starts the runtime counter.
 */
void time_stats_init(void);

/**
 * @brief Resets one timing-statistics entry.
 * @param idx Index of the entry to reset.
 */
void time_stats_reset(unsigned int idx);

/**
 * @brief Stores one execution time in a timing-statistics entry.
 * @param idx Index of the entry to update.
 * @param time_in_ms Execution time in milliseconds.
 */
void time_stats_store(unsigned int idx, float time_in_ms);

/**
 * @brief Gets the elapsed time since timing statistics were initialized.
 * @return Elapsed time in milliseconds.
 */
float time_stats_get_runtime(void);

/**
 * @brief Gets the accumulated execution time for an entry.
 * @param idx Index of the entry to read.
 * @return Accumulated execution time in milliseconds.
 */
float time_stats_get_sum(unsigned int idx);

/**
 * @brief Gets the average execution time for an entry.
 * @param idx Index of the entry to read.
 * @return Average execution time in milliseconds, or 0.0F when no value exists.
 */
float time_stats_get_avg(unsigned int idx);

/**
 * @brief Gets the minimum execution time for an entry.
 * @param idx Index of the entry to read.
 * @return Minimum execution time in milliseconds, or 0.0F when no value exists.
 */
float time_stats_get_min(unsigned int idx);

/**
 * @brief Gets the maximum execution time for an entry.
 * @param idx Index of the entry to read.
 * @return Maximum execution time in milliseconds.
 */
float time_stats_get_max(unsigned int idx);

/**
 * @brief Gets the latest execution time stored for an entry.
 * @param idx Index of the entry to read.
 * @return Latest execution time in milliseconds.
 */
float time_stats_get_latest(unsigned int idx);

/**
 * @brief Gets the number of execution times stored for an entry.
 * @param idx Index of the entry to read.
 * @return Number of stored execution times.
 */
unsigned int time_stats_get_cnt(unsigned int idx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CPU_STATS_H */
