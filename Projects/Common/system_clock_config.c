/**
  ******************************************************************************
  * @file    system_clock_config.c
  * @author  GPM/AIS Application Team
  * @version v1.0.0
  * @date    05-Nov-2026
  * @brief   system clock config
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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "main.h"
#include "app_config.h"

/* Exported variables --------------------------------------------------------*/
uint32_t npur_freq_mhz = 64U; /* NIC frequency. */
uint32_t npu_freq_mhz = 64U;  /* NPU frequency. */

/**
  * @brief  Restores the system clocks and oscillators to their reset values.
  *
  * Disables clocks that may have been left enabled by the bootloader and
  * restores HSI as the CPU and system clock source.
  *
  * @param  None
  * @retval None
  */
void SystemClock_Config_ResetClocks(void)
{
  RCC_ClkInitTypeDef clk_init = {0};
  RCC_OscInitTypeDef osc_init = {0};

  /* Stop clocks that may be enabled after reset or bootloader execution. */
  __HAL_RCC_XSPI1_CLK_DISABLE();
  __HAL_RCC_XSPI2_CLK_DISABLE();
  __HAL_RCC_NPU_CLK_DISABLE();

  /* Keep default memory clocks enabled and disable the remaining memories. */

  RCC->MEMENSR = 0x000013F0;
  RCC->MEMENCR = ~0x000013F0;

  /* Reset CPUCLK, SYSCLK and prescalers to their default values. */
  clk_init.ClockType = (RCC_CLOCKTYPE_CPUCLK | RCC_CLOCKTYPE_SYSCLK |
                        RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 |
                        RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK4 |
                        RCC_CLOCKTYPE_PCLK5);
  clk_init.CPUCLKSource = RCC_CPUCLKSOURCE_HSI;
  clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  clk_init.AHBCLKDivider = RCC_HCLK_DIV2;
  clk_init.APB1CLKDivider = RCC_APB1_DIV1;
  clk_init.APB2CLKDivider = RCC_APB2_DIV1;
  clk_init.APB4CLKDivider = RCC_APB4_DIV1;
  clk_init.APB5CLKDivider = RCC_APB5_DIV1;
  if (HAL_RCC_ClockConfig(&clk_init) != HAL_OK)
  {
    while (1)
    {
    }
  }

  /* Stop all PLLs and keep HSI enabled. */
  osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSE;
  osc_init.HSIState = RCC_HSI_ON;
  osc_init.HSEState = RCC_HSE_OFF;
  osc_init.PLL1.PLLState = RCC_PLL_OFF;
  osc_init.PLL2.PLLState = RCC_PLL_OFF;
  osc_init.PLL3.PLLState = RCC_PLL_OFF;
  osc_init.PLL4.PLLState = RCC_PLL_OFF;
  if (HAL_RCC_OscConfig(&osc_init) != HAL_OK)
  {
    while (1)
    {
    }
  }
}

/**
  * @brief  Configures the HSI overdrive clock tree.
  *
  * Configures PLL1, PLL2 and PLL3 to provide the CPU, system, NPU,
  * NPU RAM and XSPI clocks at their overdrive frequencies.
  * - PLL1 @ 800MHz / PLL2 @ 1000MHz / PLL3 @ 900 MHz
  * - CPUCLK (clock of the CPU)        800 MHz
  * - SYSCLK (clock of the CPU NIC)    400MHz
  * - IC6    (clock of the NPU)        1000MHz
  * - IC11   (clock of the NPU RAMs)   900MHz
  * - IC3/4  (clocks of the xSPIs)     200MHz*
  * @param  None
  * @retval None
  */
void SystemClock_Config_HSI_overdrive(void)
{
  RCC_ClkInitTypeDef clk_init = {0};
  RCC_OscInitTypeDef osc_init = {0};
  RCC_PeriphCLKInitTypeDef periph_clk_init = {0};

  /* HSI selected as source.
     PLL1 output = ((HSI / PLLM) * PLLN) / PLLP1 / PLLP2
                 = ((64000000 / 2) * 25) / 1 / 1
                 = 800000000 (800 MHz)
     PLL2 output = 1 GHz for the NPU.
     PLL3 output = 900 MHz for AXISRAM3..6.
  */

  osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  osc_init.HSIState = RCC_HSI_ON;
  osc_init.PLL1.PLLState = RCC_PLL_ON;
  osc_init.PLL1.PLLSource = RCC_PLLSOURCE_HSI;
  osc_init.PLL1.PLLM = 2;
  osc_init.PLL1.PLLN = 25;
  osc_init.PLL1.PLLP1 = 1;
  osc_init.PLL1.PLLP2 = 1;
  osc_init.PLL1.PLLFractional = 0;
  /* PLL2: 64 x 125 / 8 = 1GHz */
  osc_init.PLL2.PLLState = RCC_PLL_ON;
  osc_init.PLL2.PLLSource = RCC_PLLSOURCE_HSI;
  osc_init.PLL2.PLLM = 8;
  osc_init.PLL2.PLLFractional = 0;
  osc_init.PLL2.PLLN = 125;
  osc_init.PLL2.PLLP1 = 1;
  osc_init.PLL2.PLLP2 = 1;
  /* PLL3: 64 x 225 / 16 = 900MHz */
  osc_init.PLL3.PLLState = RCC_PLL_ON;
  osc_init.PLL3.PLLSource = RCC_PLLSOURCE_HSI;
  osc_init.PLL3.PLLM = 16;
  osc_init.PLL3.PLLFractional = 0;
  osc_init.PLL3.PLLN = 225;
  osc_init.PLL3.PLLP1 = 1;
  osc_init.PLL3.PLLP2 = 1;
  /* PLL4: OFF */
  osc_init.PLL4.PLLState = RCC_PLL_OFF;

  if (HAL_RCC_OscConfig(&osc_init) != HAL_OK)
  {
    while (1)
    {
    }
  }

  /* Select PLL1 outputs as CPU and system bus clock sources.
     CPUCLK = ic1_ck = PLL1 output/ic1_divider = 800 MHz
     SYSCLK = ic2_ck = PLL1 output/ic2_divider = 400 MHz
     Configure the HCLK, PCLK1, PCLK2, PCLK4 and PCLK5 clocks dividers
     HCLK = SYSCLK/HCLK divider = 200 MHz
     PCLKx = HCLK / PCLKx divider = 100 MHz
  */

  clk_init.ClockType = (RCC_CLOCKTYPE_CPUCLK | RCC_CLOCKTYPE_SYSCLK |
                        RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 |
                        RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK4 |
                        RCC_CLOCKTYPE_PCLK5);
  /* IC1 is used for CPUCLK @ 800MHz (=800/1) */
  clk_init.IC1Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
  clk_init.IC1Selection.ClockDivider = 1;
  /* IC2 is used for SYSCLK @ 400MHz (=800/2) */
  clk_init.IC2Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
  clk_init.IC2Selection.ClockDivider = 2;
  /* IC6 (used for NPU) @ 1GHz */
  clk_init.IC6Selection.ClockSelection = RCC_ICCLKSOURCE_PLL2;
  clk_init.IC6Selection.ClockDivider = 1;
  npu_freq_mhz = 1000U;
  /* IC11 (for AXISRAM3->6) @ 900MHz */

  clk_init.IC11Selection.ClockSelection = RCC_ICCLKSOURCE_PLL3;
  clk_init.IC11Selection.ClockDivider = 1;
  npur_freq_mhz = 900U;

  clk_init.CPUCLKSource = RCC_CPUCLKSOURCE_IC1;
  clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_IC2_IC6_IC11;

  /* Do not use PCLK dividers other than 1; this causes unstable behavior. */
  clk_init.AHBCLKDivider = RCC_HCLK_DIV4;
  clk_init.APB1CLKDivider = RCC_APB1_DIV1;
  clk_init.APB2CLKDivider = RCC_APB2_DIV1;
  clk_init.APB4CLKDivider = RCC_APB4_DIV1;
  clk_init.APB5CLKDivider = RCC_APB5_DIV1;

  if (HAL_RCC_ClockConfig(&clk_init) != HAL_OK)
  {
    while (1)
    {
    }
  }

  /* Select IC4 from PLL1, divided by 4, as the 200 MHz XSPI1 source. */
  periph_clk_init.PeriphClockSelection = RCC_PERIPHCLK_XSPI1;
  periph_clk_init.Xspi1ClockSelection = RCC_XSPI1CLKSOURCE_IC4;
  periph_clk_init.ICSelection[RCC_IC4].ClockSelection = RCC_ICCLKSOURCE_PLL1;
  periph_clk_init.ICSelection[RCC_IC4].ClockDivider = 4;
  /* Select IC3 from PLL1, divided by 4, as the 200 MHz XSPI2 source. */
  periph_clk_init.PeriphClockSelection |= RCC_PERIPHCLK_XSPI2;
  periph_clk_init.Xspi2ClockSelection = RCC_XSPI2CLKSOURCE_IC3;
  periph_clk_init.ICSelection[RCC_IC3].ClockSelection = RCC_ICCLKSOURCE_PLL1;
  periph_clk_init.ICSelection[RCC_IC3].ClockDivider = 4;
  if (HAL_RCCEx_PeriphCLKConfig(&periph_clk_init) != HAL_OK)
  {
    __BKPT(0);
  }
}

/**
  * @brief  Configures the HSI clock tree without overdrive.
  *
  * Configures PLL1 and PLL2 to provide the CPU, system, NPU, NPU RAM
  * and XSPI clocks at the non-overdrive frequencies.
  * - PLL1 @ 800MHz / PLL2 @ 600MHz
  * - CPUCLK (clock of the CPU)        600 MHz
  * - SYSCLK (clock of the CPU NIC)    400MHz
  * - IC6    (clock of the NPU)        800MHz
  * - IC11   (clock of the NPU RAMs)   800MHz
  * - HCLK   (clock of the xSPIs)      200MHz
  * @param  None
  * @retval None
  */
void SystemClock_Config_HSI_no_overdrive(void)
{
  RCC_ClkInitTypeDef clk_init = {0};
  RCC_OscInitTypeDef osc_init = {0};
  RCC_PeriphCLKInitTypeDef periph_clk_init = {0};

  /* HSI selected as source.
     PLL1 output = ((HSI / PLLM) * PLLN) / PLLP1 / PLLP2
                 = ((64000000 / 2) * 25) / 1 / 1
                 = 800000000 (800 MHz)
     PLL2 output = 600 MHz for the CPU.
  */
  osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  osc_init.HSIState = RCC_HSI_ON;
  osc_init.PLL1.PLLState = RCC_PLL_ON;
  osc_init.PLL1.PLLSource = RCC_PLLSOURCE_HSI;
  osc_init.PLL1.PLLM = 2;
  osc_init.PLL1.PLLN = 25;
  osc_init.PLL1.PLLP1 = 1;
  osc_init.PLL1.PLLP2 = 1;
  osc_init.PLL1.PLLFractional = 0;
  /* PLL2: 64 x 75 / 8 = 600MHz */
  osc_init.PLL2.PLLState = RCC_PLL_ON;
  osc_init.PLL2.PLLSource = RCC_PLLSOURCE_HSI;
  osc_init.PLL2.PLLM = 8;
  osc_init.PLL2.PLLFractional = 0;
  osc_init.PLL2.PLLN = 75;
  osc_init.PLL2.PLLP1 = 1;
  osc_init.PLL2.PLLP2 = 1;
  /* PLL3: OFF */
  osc_init.PLL3.PLLState = RCC_PLL_OFF;
#ifdef APP_LCD
  /* PLL4 = (64 x 225 / 8) / (6 * 6) = 50 MHz */
  osc_init.PLL4.PLLState = RCC_PLL_ON;
  osc_init.PLL4.PLLSource = RCC_PLLSOURCE_HSI;
  osc_init.PLL4.PLLM = 8;
  osc_init.PLL4.PLLFractional = 0;
  osc_init.PLL4.PLLN = 225;
  osc_init.PLL4.PLLP1 = 6;
  osc_init.PLL4.PLLP2 = 6;
#else /* not APP_LCD */
  /* PLL4: off. */
  osc_init.PLL4.PLLState = RCC_PLL_OFF;
#endif /* APP_LCD */
  if (HAL_RCC_OscConfig(&osc_init) != HAL_OK)
  {
    while (1)
    {
    }
  }

  /* Select PLL1 outputs as CPU and system bus clock sources.
      CPUCLK = ic1_ck = PLL2 output/ic1_divider = 600 MHz
      SYSCLK = ic2_ck = PLL1 output/ic2_divider = 400 MHz
      Configure the HCLK, PCLK1, PCLK2, PCLK4 and PCLK5 clocks dividers
      HCLK = SYSCLK/HCLK divider = 200 MHz
      PCLKx = HCLK / PCLKx divider = 200 MHz
  */
  clk_init.ClockType = (RCC_CLOCKTYPE_CPUCLK | RCC_CLOCKTYPE_SYSCLK |
                        RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 |
                        RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK4 |
                        RCC_CLOCKTYPE_PCLK5);

  /* IC1 is used for CPUCLK @ 600MHz (=600/1) */
  clk_init.IC1Selection.ClockSelection = RCC_ICCLKSOURCE_PLL2;
  clk_init.IC1Selection.ClockDivider = 1;
  /* IC2 is used for SYSCLK @ 400MHz (=800/2) */
  clk_init.IC2Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
  clk_init.IC2Selection.ClockDivider = 2;
  /* IC6 (used for NPU) @ 800 MHz. */
  clk_init.IC6Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
  clk_init.IC6Selection.ClockDivider = 1;
  npu_freq_mhz = 800U;
  /* IC11 (for AXISRAM3->6) @ 800MHz */
  clk_init.IC11Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
  clk_init.IC11Selection.ClockDivider = 1;
  npur_freq_mhz = 800U;

  clk_init.CPUCLKSource = RCC_CPUCLKSOURCE_IC1;
  clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_IC2_IC6_IC11;

  /* Do not use PCLK dividers other than 1; this causes unstable behavior. */
  clk_init.AHBCLKDivider = RCC_HCLK_DIV2;
  clk_init.APB1CLKDivider = RCC_APB1_DIV1;
  clk_init.APB2CLKDivider = RCC_APB2_DIV1;
  clk_init.APB4CLKDivider = RCC_APB4_DIV1;
  clk_init.APB5CLKDivider = RCC_APB5_DIV1;

  if (HAL_RCC_ClockConfig(&clk_init) != HAL_OK)
  {
    while (1)
    {
    }
  }
  /* Keep both xSPIs on their reset-default source, the 200 MHz HCLK: the boot
     ROM reads the boot flash through XSPI1, and after an NRST reset a
     non-default XSPI1 clock switch outputs no clock (ES0620 errata 2.2.22),
     so the boot ROM cannot load the FSBL and only a power cycle restarts the
     board. */
  periph_clk_init.PeriphClockSelection = RCC_PERIPHCLK_XSPI1;
  periph_clk_init.Xspi1ClockSelection = RCC_XSPI1CLKSOURCE_HCLK;

  periph_clk_init.PeriphClockSelection |= RCC_PERIPHCLK_XSPI2;
  periph_clk_init.Xspi2ClockSelection = RCC_XSPI2CLKSOURCE_HCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&periph_clk_init) != HAL_OK)
  {
    __BKPT(0);
  }
}

/**
  * @brief  Configures MSI at 4 MHz for the CPU and system clocks.
  *
  * Disables all PLLs and HSI after switching the CPU and system clock
  * sources to MSI.
  *
  * @param  None
  * @retval None
  */
void SystemClock_Config_MSI_4MHz_4MHz_exp(void)
{
  RCC_ClkInitTypeDef clk_init = {0};
  RCC_OscInitTypeDef osc_init = {0};

  /* Start MSI at 4 MHz. */
  osc_init.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  osc_init.MSIState = RCC_MSI_ON;
  osc_init.MSIFrequency = RCC_MSI_FREQ_4MHZ;
  if (HAL_RCC_OscConfig(&osc_init) != HAL_OK)
  {
    while (1)
    {
    }
  }

  /* Select MSI as the CPU and system bus clock source. */
  clk_init.ClockType = (RCC_CLOCKTYPE_CPUCLK | RCC_CLOCKTYPE_SYSCLK |
                        RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 |
                        RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK4 |
                        RCC_CLOCKTYPE_PCLK5);

  clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  clk_init.CPUCLKSource = RCC_CPUCLKSOURCE_MSI;

  /* Do not use PCLK dividers other than 1; this causes unstable behavior. */
  clk_init.AHBCLKDivider = RCC_HCLK_DIV4;
  clk_init.APB1CLKDivider = RCC_APB1_DIV1;
  clk_init.APB2CLKDivider = RCC_APB2_DIV1;
  clk_init.APB4CLKDivider = RCC_APB4_DIV1;
  clk_init.APB5CLKDivider = RCC_APB5_DIV1;

  if (HAL_RCC_ClockConfig(&clk_init) != HAL_OK)
  {
    while (1)
    {
    }
  }

  /* Stop all PLLs. */
  osc_init.PLL1.PLLState = RCC_PLL_OFF;
  osc_init.PLL2.PLLState = RCC_PLL_OFF;
  osc_init.PLL3.PLLState = RCC_PLL_OFF;
  osc_init.PLL4.PLLState = RCC_PLL_OFF;

  if (HAL_RCC_OscConfig(&osc_init) != HAL_OK)
  {
    while (1)
    {
    }
  }

  /* Stop HSI. */
  osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  osc_init.HSIState = RCC_HSI_OFF;
  if (HAL_RCC_OscConfig(&osc_init) != HAL_OK)
  {
    while (1)
    {
    }
  }
}
