/**
  ******************************************************************************
  * @file    audio_bm.c
  * @author  MCD Application Team
  * @version v1.0.0
  * @date    05-Nov-2026
  * @brief
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
/* Includes ------------------------------------------------------------------*/
#include "stm32n6xx_hal.h"
#include "stm32n6570_discovery.h"
#include "stm32n6570_discovery_xspi.h"
#include "stm32n6570_discovery_audio.h"
#include "app_config.h"
#include "system_clock_config.h"
#include "misc_toolbox.h"
#include "cpu_stats.h"
#include "AudioCapture_ring_buff.h"
#include "preproc_dpu.h"                           /* Preprocessing includes  */
#include "ai_dpu.h"                                /* AI includes             */
#include "audio_bm.h"

#ifdef APP_LCD
#include "stm32n6570_discovery_lcd.h"
#include "stm32_lcd.h"
#include "stm32_lcd_ex.h"
#include "stlogo.h"
#include "voiceicon.h"
#endif /* APP_LCD */

/* Private define ------------------------------------------------------------*/
/* Derived from the patch/acquisition layout ([overlap | new samples]),
 * not restated in terms of the spectrogram window/hop, so it cannot silently
 * drift out of sync with AI_SPECT_COL. */
#define AUDIO_ACQ_OFFSET      (PATCH_LEN - AUDIO_ACQ_LEN)
#ifdef APP_LCD
#define LCD_WIDTH            (480)
#define LCD_HEIGHT           (800)
#define LCD_FRAMEBUFFER_SIZE (LCD_WIDTH * LCD_HEIGHT * 2)
#endif /* APP_LCD */

_Static_assert(PATCH_LEN >= AUDIO_ACQ_LEN,
               "PATCH_LEN must be able to hold at least one new acquisition (AUDIO_ACQ_LEN)");
_Static_assert(AI_SPECT_NFFT >= AI_SPECT_WIN_LEN,
               "AI_SPECT_NFFT must be able to hold a full analysis window (AI_SPECT_WIN_LEN)");
_Static_assert((AUDIO_ACQ_LEN % CAPTURE_BUFFER_SIZE) == 0,
               "An acquisition (AUDIO_ACQ_LEN) must be a whole number of capture buffers (CAPTURE_BUFFER_SIZE)");

/* Private function prototypes -----------------------------------------------*/
static void IAC_Config(void);
static void MPU_Config(void);
static void Ext_Mem_Config(void);
static void Int_Mem_Config(void);
static void SleepClks_init(void);
static void Security_Config(void);
static void Record_Init(void);
static void NPU_SettingsLog(void);
static void printCpuStats(void);
static void displaySystemSetting(void);
static bool audio_process(AudioBM_acq_t *acq_ctx_ptr,\
                          AudioBM_proc_t *proc_ctx_ptr);
static void initAudioCapture(AudioBM_acq_t *ctx_ptr);
static void startAudioCapture(AudioBM_acq_t *ctx_ptr);
static void stopAudioCapture(void);
static void initAudioProc(AudioBM_proc_t *ctx_ptr);
static void AudioCapture_half_buf_cb(AudioCapture_ring_buff_t *pHdle,\
                                     int16_t *pData, uint8_t half_buf);
static void printHeader(void);
static void printAppHeader(void);
#ifdef APP_LCD
static void LCD_init(void);
static void displayLCD_VAD(void);
#endif /* APP_LCD */

/* Private variables ---------------------------------------------------------*/
static AudioBM_acq_t audio_acq_ctx;
static AudioBM_proc_t audio_proc_ctx;
static float last_refresh_cpu;
static float last_refresh_voice;

#ifdef APP_LCD
/* Voice status currently drawn on the LCD, used to redraw only on change. */
typedef enum
{
  LCD_VOICE_UNKNOWN = 0,
  LCD_VOICE_DETECTED,
  LCD_VOICE_LISTENING
} LcdVoiceState_t;
static LcdVoiceState_t lcd_voice_state = LCD_VOICE_UNKNOWN;
#endif /* APP_LCD */

#ifdef APP_LCD
/* lcd_buffer lives in .psram_bss (xSPI1 @0x90000000, see
 * STM32N657XX_LRUN.ld), which is only memory-mapped once Ext_Mem_Config() runs
 * BSP_XSPI_RAM_Init()/EnableMemoryMappedMode() below, itself gated on
 * USE_EXT_SRAM. Without this guard, APP_LCD + !USE_EXT_SRAM links successfully
 * and the LTDC silently scans out of an unmapped window at runtime. */
#if !defined(USE_EXT_SRAM)
#error "APP_LCD places lcd_buffer in .psram_bss (xSPI1 @0x90000000), which is only mapped when USE_EXT_SRAM enables BSP_XSPI_RAM_Init()"
#endif /* !USE_EXT_SRAM */
__attribute__ ((section (".psram_bss")))
__attribute__ ((aligned (32)))
static uint8_t lcd_buffer[LCD_FRAMEBUFFER_SIZE];
#endif /* APP_LCD */

/**
  * @brief  Initializes the bare-metal audio application.
  * @param  None
  * @retval None
  */
void init_bm(void)
{
  /* Power on ICACHE */
  MEMSYSCTL->MSCR |= MEMSYSCTL_MSCR_ICACTIVE_Msk;

  /* Set back system and CPU clock source to HSI */
  __HAL_RCC_CPUCLK_CONFIG(RCC_CPUCLKSOURCE_HSI);
  __HAL_RCC_SYSCLK_CONFIG(RCC_SYSCLKSOURCE_HSI);

  HAL_Init();

  SystemClock_Config_Full();

  /* Force fusing of the OTP when using a Nucleo/DK board only */
#if (defined(USE_STM32N6xx_NUCLEO) || defined(USE_STM32N6570_DK))
  fuse_vddio();
#endif /* USE_STM32N6xx_NUCLEO */

  MPU_Config();
  Int_Mem_Config();
  Ext_Mem_Config();
  Security_Config();
  NPU_Config();
  IAC_Config();
  SCB_EnableICache();
  SCB_EnableDCache();
  port_dwt_init_imp();
  SleepClks_init(); /* configures for sleep */

  /* BSP inits */
#ifdef APP_UART
  UART_Config();
  printAppHeader();
  /* configuration information on console */
  displaySystemSetting();
#endif /* APP_UART */

  BSP_PB_Init(BUTTON_USER1, BUTTON_MODE_EXTI);
  BSP_PB_Init(BUTTON_TAMP, BUTTON_MODE_EXTI);
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_RED);

#ifdef APP_LCD
  LCD_init();
#endif /* APP_LCD */
}

/**
  * @brief  Runs the bare-metal audio processing loop.
  * @param  None
  * @retval None
  */
void exec_bm(void)
{
  bool cont = true;
  time_stats_init();
  last_refresh_cpu   = time_stats_get_runtime();
  last_refresh_voice = time_stats_get_runtime();
  initAudioProc(&audio_proc_ctx);
  initAudioCapture(&audio_acq_ctx);
  startAudioCapture(&audio_acq_ctx);

#ifdef APP_UART
  printHeader();
#endif /* APP_UART */
#ifdef APP_LCD
  displayLCD_VAD();
#endif /* APP_LCD */

  while (cont)
  {
    __NOP(); /* to be resilient to gcc optim ... to investigate */
    if (audio_acq_ctx.ring_buff.availableSamples >= AUDIO_ACQ_LEN)
    {
      cont = audio_process(&audio_acq_ctx, &audio_proc_ctx);
      printCpuStats();
    }
  }
  stopAudioCapture();

  my_printf("\n\r# End Processing\n\r");
}

/**
  * @brief  Initializes the audio preprocessing and AI processing context.
  * @param  ctx_ptr Pointer to the audio processing context.
  * @retval None
  */
static void initAudioProc(AudioBM_proc_t *ctx_ptr)
{
  /* Layout guard: APP_LOW_LAT changes AudioProcCtx_t's layout, and nothing
   * stops a future header change from making some translation unit lose
   * sight of the macro again (as preproc_dpu.c/ai_dpu.c silently did). Fail
   * loudly here instead of reading past a buffer sized by the wrong view. */
  if (PreProc_DPU_GetCtxSize() != sizeof(AudioProcCtx_t) ||
      PreProc_DPU_GetPatchLen() != PATCH_LEN)
  {
    LogError("Layout guard: preproc_dpu.c's view of AudioProcCtx_t/PATCH_LEN "
             "disagrees with audio_bm.c's -- APP_LOW_LAT visibility regression\n\r");
    Error_Handler();
  }

  /* get the AI model */
  AiDPULoadModel(&ctx_ptr->aiCtx);
  ctx_ptr->ai_in_ptr = (int8_t *)ctx_ptr->aiCtx.p_stai_inputs[0];
  ctx_ptr->ai_out_ptr = (int8_t *)ctx_ptr->aiCtx.p_stai_outputs[0];

  /* clear input samples array ( get silence on first overlayed patch */
  memset(ctx_ptr->proc_buff, 0, PATCH_LEN * sizeof(int16_t));
  /* Transfer quantization parameters from the AI model to the Audio DPU. */
  ctx_ptr->audioPreCtx.output_Q_offset = ctx_ptr->aiCtx.input_Q_offset;
  ctx_ptr->audioPreCtx.output_Q_inv_scale =
      (float16_t)ctx_ptr->aiCtx.input_Q_inv_scale;
  /* Audio Preprocessing init */
  PreProc_DPUInit(&ctx_ptr->audioPreCtx);
}

/**
  * @brief  Processes one audio frame through preprocessing and AI inference.
  * @param  acq_ctx_ptr Pointer to the audio acquisition context.
  * @param  proc_ctx_ptr Pointer to the audio processing context.
  * @retval true Processing can continue.
  */
bool audio_process(AudioBM_acq_t *acq_ctx_ptr, AudioBM_proc_t *proc_ctx_ptr)
{
  uint8_t *proc_buf = (uint8_t *) proc_ctx_ptr->proc_buff;
  uint8_t *proc_buf_ovl = (uint8_t *) (&proc_ctx_ptr->proc_buff[AUDIO_ACQ_LEN]);
  uint8_t *acq_buf = (uint8_t *) (&proc_ctx_ptr->proc_buff[AUDIO_ACQ_OFFSET]);

  /* prepare overlapping samples from previous patch (source and destination
   * may overlap, so memmove is required) */
  memmove(proc_buf, proc_buf_ovl, AUDIO_ACQ_OFFSET * sizeof(int16_t));

  /* Audio samples acquisition */
  AudioCapture_ring_buff_consume(acq_buf, &acq_ctx_ptr->ring_buff, AUDIO_ACQ_LEN);

  /* Audio pre processing */
  PreProc_DPU(&proc_ctx_ptr->audioPreCtx, proc_buf, proc_ctx_ptr->ai_in_ptr);

  /* AI inference */
  AiDPUProcess(&proc_ctx_ptr->aiCtx);

  BSP_LED_Toggle(LED_RED);
  return true;
}

/**
  * @brief  Initializes the microphone audio capture.
  * @param  ctx_ptr Pointer to the audio acquisition context.
  * @retval None
  */
void initAudioCapture(AudioBM_acq_t *ctx_ptr)
{
  ctx_ptr->ring_buff.nbSamples = ((PATCH_LEN / CAPTURE_BUFFER_SIZE) + 1) \
                                 * CAPTURE_BUFFER_SIZE;
  ctx_ptr->ring_buff.nbBytesPerSample = 2;
  ctx_ptr->ring_buff.nbFrames = 2;
  AudioCapture_ring_buff_alloc(&ctx_ptr->ring_buff);
  /* Initialize record */
  Record_Init();
}

/**
  * @brief  Starts microphone audio capture.
  * @param  ctx_ptr Pointer to the audio acquisition context.
  * @retval None
  */
static void startAudioCapture(AudioBM_acq_t *ctx_ptr)
{
  /* Start record */
  if (BSP_ERROR_NONE != BSP_AUDIO_IN_Record(1, (uint8_t *) ctx_ptr->acq_buf,
                                             CAPTURE_BUFFER_SIZE * sizeof(int16_t)))
  {
    Error_Handler();
  }
}

/**
  * @brief  Stops microphone audio capture.
  * @param  None
  * @retval None
  */
static void stopAudioCapture(void)
{
  BSP_AUDIO_IN_Stop(1);
}

/**
  * @brief  Prints the processing statistics header on the console.
  * @param  None
  * @retval None
  */
static void printHeader(void)
{
  my_printf(SEPARATION_LINE_SHORT);
  my_printf("# Start Processing\033[?25l\n\r"); /* Hide cursor. */
  my_printf(SEPARATION_LINE_SHORT);
  my_printf("|  Cpu |  Pre |  AI  | Voice |");
  my_printf("\r\n");
}
#ifdef APP_LCD
/**
  * @brief  Initializes the LCD and its framebuffer layer.
  * @param  None
  * @retval None
  */
static void LCD_init(void)
{
  BSP_LCD_LayerConfig_t LayerConfig = {0};

  BSP_LCD_Init(0, LCD_ORIENTATION_LANDSCAPE);

  /* Preview layer Init */
  LayerConfig.X0          = 0;
  LayerConfig.Y0          = 0;
  LayerConfig.X1          = LCD_HEIGHT;
  LayerConfig.Y1          = LCD_WIDTH;
  LayerConfig.PixelFormat = LCD_PIXEL_FORMAT_RGB565;
  LayerConfig.Address     = (uint32_t) lcd_buffer;

  BSP_LCD_ConfigLayer(0, LTDC_LAYER_1, &LayerConfig);

  UTIL_LCD_SetFuncDriver(&LCD_Driver);
  UTIL_LCD_Clear(UTIL_LCD_COLOR_BLACK);
  UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLACK);
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
  UTIL_LCD_SetFont(&Font20);
}

/**
  * @brief  Displays the initial Voice Activity Detection screen.
  * @param  None
  * @retval None
  */
static void displayLCD_VAD(void)
{
  /* Display Voice Activity Detection on LCD */
  UTIL_LCD_Clear(UTIL_LCD_COLOR_WHITE);
  UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_ST_BLUE_DARK);
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
  UTIL_LCD_FillRGBRect(650, 0, stlogo, 150, 103);
  UTIL_LCD_FillRect(150, 40, 500, 150, UTIL_LCD_COLOR_ST_BLUE_DARK);
  UTIL_LCD_SetFont(&Font24);
  UTIL_LCDEx_PrintfAt(0, 60, CENTER_MODE, "Voice Activity Detection");
  UTIL_LCD_SetFont(&Font20);
  UTIL_LCDEx_PrintfAt(0, 95, CENTER_MODE, "ST TenVAD model");
  UTIL_LCDEx_PrintfAt(0, 160, CENTER_MODE, "Inf.:      ms | AI confidence:   %%");
}
#endif /* APP_LCD */

/**
  * @brief  Displays CPU load, inference results and voice activity status.
  * @param  None
  * @retval None
  */
static void printCpuStats(void)
{
  /* Display real-time statistics. */
  float pre_avg = time_stats_get_avg(TIME_STAT_PRE_PROC);
  float ai_avg = time_stats_get_avg(TIME_STAT_AI_PROC);
  float pre_load = 100 * pre_avg / AUDIO_ACQ_LEN_MS;
  float ai_load = 100 * ai_avg / AUDIO_ACQ_LEN_MS;
  float runtime = time_stats_get_runtime();
#ifdef APP_UART
  my_printf("|%6.2f|%6.2f|%6.2f|", pre_load + ai_load, pre_load, ai_load);
#endif
#ifdef APP_LCD
  UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_ST_BLUE_DARK);
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
  UTIL_LCD_SetFont(&Font20);
  UTIL_LCDEx_PrintfAt(232, 160, LEFT_MODE, "%.3f", pre_avg + ai_avg);
  UTIL_LCDEx_PrintfAt(585, 160, LEFT_MODE, "%2d", (int)audio_proc_ctx.aiCtx.out);
  UTIL_LCD_SetFont(&Font24);
  UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_WHITE);
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_ST_BLUE_DARK);
#endif /* APP_LCD */

  if (audio_proc_ctx.aiCtx.out > APP_VAD_THR)
  {
    last_refresh_voice = runtime;
	BSP_LED_On(LED_GREEN);
#ifdef APP_UART
	my_printf("\033[1;32m %6.2f\033[0m| Voice activity detected",
			  audio_proc_ctx.aiCtx.out);
#endif /* APP_UART */
#ifdef APP_LCD
    /* Redraw the icon when the state changes. */
    if (lcd_voice_state != LCD_VOICE_DETECTED)
    {
      UTIL_LCDEx_PrintfAt(0, 430, CENTER_MODE, "    Voice Activity detected!   ");
      UTIL_LCD_FillRGBRect(300, 210, voiceicon, 200, 199);
      lcd_voice_state = LCD_VOICE_DETECTED;
    }
#endif /* APP_LCD */
  }
  else
  {
#ifdef APP_UART
    my_printf("\033[0;31m %6.2f\033[0m|", audio_proc_ctx.aiCtx.out);
#endif /* APP_UART */
    BSP_LED_Off(LED_GREEN);
  }

  if (runtime - last_refresh_voice > APP_REFRESH_VOICE)
  {
#ifdef APP_UART
    my_printf("                        ");
#endif /* APP_UART */
#ifdef APP_LCD
    if (lcd_voice_state != LCD_VOICE_LISTENING)
    {
      UTIL_LCD_FillRect(300, 210, 200, 200, UTIL_LCD_COLOR_WHITE);
      UTIL_LCDEx_PrintfAt(0, 430, CENTER_MODE, "Listening, waiting for speech");
      lcd_voice_state = LCD_VOICE_LISTENING;
    }
#endif /* APP_LCD */
  }
#ifdef APP_UART
  my_printf("\r");

  fflush(stdout);
#endif /* APP_UART */

  if (runtime - last_refresh_cpu > APP_REFRESH_CPU)
  {
    time_stats_reset(TIME_STAT_AI_PROC);
    time_stats_reset(TIME_STAT_PRE_PROC);
    last_refresh_cpu = runtime;
  }
}

/**
  * @brief  Prints the build/version UART header.
  * @param  None
  * @retval None
  */
static void printAppHeader(void)
{
  my_printf(SEPARATION_LINE);
  my_printf("x-cube-n6-ai-vad %s (%s)\n\r", APP_VERSION_STRING, APP_GIT_SHA1_STRING);
  my_printf("Build date & time: %s %s\n\r", __DATE__, __TIME__);
#if defined(__GNUC__)
  my_printf("Compiler: GCC %d.%d.%d\n\r", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif defined(__ICCARM__)
  my_printf("Compiler: IAR EWARM %d.%d.%d\n\r", __VER__ / 1000000, (__VER__ / 1000) % 1000, __VER__ % 1000);
#else
  my_printf("Compiler: Unknown\n\r");
#endif
  my_printf("HAL: %lu.%lu.%lu\n\r", (uint32_t)__STM32N6xx_HAL_VERSION_MAIN,
            (uint32_t)__STM32N6xx_HAL_VERSION_SUB1, (uint32_t)__STM32N6xx_HAL_VERSION_SUB2);
  my_printf("STEdgeAI Tools: %d.%d.%d\n\r", STAI_TOOLS_VERSION_MAJOR, STAI_TOOLS_VERSION_MINOR, STAI_TOOLS_VERSION_MICRO);
  my_printf("NN model: %s\n\r", LL_ATON_NETWORK_ORIGIN_MODEL_NAME);
  my_printf(SEPARATION_LINE);
}

/**
  * @brief  Displays the system configuration on the console.
  * @param  None
  * @retval None
  */
static void displaySystemSetting(void)
{
  my_printf("\n\n\n\n\r");
  my_printf(SEPARATION_LINE);
  my_printf("        System configuration (%s)\n\r",APP_CONF_STR);
  my_printf(SEPARATION_LINE);
  my_printf("- model variant : %s (%s)\n\r", APP_MODEL_STR, LL_ATON_NETWORK_ORIGIN_MODEL_NAME);
  my_printf("- latency mode  : %s\n\r", APP_LAT_STR);
  my_printf("- acq cadence   : %lu ms\n\r", (uint32_t)AUDIO_ACQ_LEN_MS);
  my_printf(SEPARATION_LINE);
  printf("\n\rLog Level: %s\n\n\r", getLogLevelStr(LOG_LEVEL));
  systemSettingLog();
  NPU_SettingsLog();
}

static void AudioCapture_half_buf_cb(AudioCapture_ring_buff_t *pHdle,
                                     int16_t *pData, uint8_t half_buf)
{
  const int16_t *in_p = pData + half_buf * (CAPTURE_BUFFER_SIZE / 2);

  AudioCapture_ring_buff_feed(pHdle, (uint8_t *)in_p, CAPTURE_BUFFER_SIZE / 2);
}

/**
  * @brief  Handles the BSP audio transfer-complete event.
  * @param  Instance Audio input instance.
  * @retval None
  */
void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance)
{
  if (Instance == 1U)
  {
    AudioCapture_half_buf_cb(&audio_acq_ctx.ring_buff, audio_acq_ctx.acq_buf, 1U);
  }
}

/**
  * @brief  Handles the BSP audio half-transfer event.
  * @param  Instance Audio input instance.
  * @retval None
  */
void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance)
{
  if (Instance == 1U)
  {
    AudioCapture_half_buf_cb(&audio_acq_ctx.ring_buff, audio_acq_ctx.acq_buf, 0U);
  }
}

/**
  * @brief  Handles a BSP audio input error.
  * @param  Instance Audio input instance.
  * @retval None
  */
void BSP_AUDIO_IN_Error_CallBack(uint32_t Instance)
{
  Error_Handler();
}

/**
  * @brief  Handles a user button event.
  * @param  Button Identifier of the button that triggered the event.
  * @retval None
  */
void BSP_PB_Callback(Button_TypeDef Button)
{
  if (BUTTON_USER1 == Button)
  {
  }
}

/*==============================================================================
                    private  functions definition
 ============================================================================= */

/**
  * @brief  Configures the internal memory and enables memory clocks.
  * @param  None
  * @retval None
  */
static void Int_Mem_Config(void)
{
  RAMCFG_HandleTypeDef hramcfg = {0};

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_CRC_CLK_ENABLE();

  RCC->MEMENR |= RCC_MEMENR_AXISRAM3EN | RCC_MEMENR_AXISRAM4EN |
                 RCC_MEMENR_AXISRAM5EN | RCC_MEMENR_AXISRAM6EN;
  RCC->MEMENR |= RCC_MEMENR_CACHEAXIRAMEN;
  hramcfg.Instance = RAMCFG_SRAM2_AXI;
  HAL_RAMCFG_EnableAXISRAM(&hramcfg);
  hramcfg.Instance = RAMCFG_SRAM3_AXI;
  HAL_RAMCFG_EnableAXISRAM(&hramcfg);
  hramcfg.Instance = RAMCFG_SRAM4_AXI;
  HAL_RAMCFG_EnableAXISRAM(&hramcfg);
  hramcfg.Instance = RAMCFG_SRAM5_AXI;
  HAL_RAMCFG_EnableAXISRAM(&hramcfg);
  hramcfg.Instance = RAMCFG_SRAM6_AXI;
  HAL_RAMCFG_EnableAXISRAM(&hramcfg);

  __HAL_RCC_CACHEAXIRAM_MEM_CLK_ENABLE();
  __HAL_RCC_AXISRAM2_MEM_CLK_ENABLE();
  __HAL_RCC_AXISRAM3_MEM_CLK_ENABLE();
  __HAL_RCC_AXISRAM4_MEM_CLK_ENABLE();
  __HAL_RCC_AXISRAM5_MEM_CLK_ENABLE();
  __HAL_RCC_AXISRAM6_MEM_CLK_ENABLE();

  /* Allow caches to be activated. The current boot sets the default to 0. */
  MEMSYSCTL->MSCR |= MEMSYSCTL_MSCR_DCACTIVE_Msk | MEMSYSCTL_MSCR_ICACTIVE_Msk;
}

/**
  * @brief  Initializes and validates the audio input peripheral.
  * @param  None
  * @retval None
  */
static void Record_Init(void)
{
  BSP_AUDIO_Init_t AudioInit;
  uint32_t GetData;

  /* Check the initial state. */
  if (BSP_ERROR_NONE != BSP_AUDIO_IN_GetState(1, &GetData))
  {
    Error_Handler();
  }
  if (GetData != AUDIO_IN_STATE_RESET)
  {
    Error_Handler();
  }

  AudioInit.Device        = AUDIO_IN_DEVICE_DIGITAL_MIC;
  AudioInit.SampleRate    = AUDIO_FREQUENCY;
  AudioInit.BitsPerSample = AUDIO_RESOLUTION_16B;
  AudioInit.ChannelsNbr   = 1;
  AudioInit.Volume        = 80; /* Not used */

  if (BSP_ERROR_NONE != BSP_AUDIO_IN_Init(1, &AudioInit))
  {
    Error_Handler();
  }

  /* Check the stopped state. */
  if (BSP_ERROR_NONE != BSP_AUDIO_IN_GetState(1, &GetData))
  {
    Error_Handler();
  }
  if (GetData != AUDIO_IN_STATE_STOP)
  {
    Error_Handler();
  }

  /* Validate supported and unsupported audio settings. */
  if (BSP_ERROR_FEATURE_NOT_SUPPORTED != BSP_AUDIO_IN_SetVolume(1, 10))
  {
    Error_Handler();
  }
  if (BSP_ERROR_NONE != BSP_AUDIO_IN_SetDevice(1, AUDIO_IN_DEVICE_ANALOG_MIC))
  {
    Error_Handler();
  }
  if (BSP_ERROR_FEATURE_NOT_SUPPORTED != BSP_AUDIO_IN_SetBitsPerSample(1, AUDIO_RESOLUTION_8B))
  {
    Error_Handler();
  }
  if (BSP_ERROR_NONE != BSP_AUDIO_IN_SetBitsPerSample(1, AUDIO_RESOLUTION_16B))
  {
    Error_Handler();
  }
  if (BSP_ERROR_FEATURE_NOT_SUPPORTED != BSP_AUDIO_IN_SetBitsPerSample(1, AUDIO_RESOLUTION_24B))
  {
    Error_Handler();
  }
  if (BSP_ERROR_FEATURE_NOT_SUPPORTED != BSP_AUDIO_IN_SetBitsPerSample(1, AUDIO_RESOLUTION_32B))
  {
    Error_Handler();
  }
  if (BSP_ERROR_FEATURE_NOT_SUPPORTED != BSP_AUDIO_IN_SetChannelsNbr(1, 2))
  {
    Error_Handler();
  }
  if (BSP_ERROR_NONE != BSP_AUDIO_IN_SetChannelsNbr(1, 1))
  {
    Error_Handler();
  }

  if (BSP_ERROR_FEATURE_NOT_SUPPORTED != BSP_AUDIO_IN_GetVolume(1, &GetData))
  {
    Error_Handler();
  }
  if (BSP_ERROR_NONE != BSP_AUDIO_IN_GetSampleRate(1, &GetData))
  {
    Error_Handler();
  }
  if (BSP_ERROR_NONE != BSP_AUDIO_IN_GetDevice(1, &GetData))
  {
    Error_Handler();
  }
  if (GetData != AUDIO_IN_DEVICE_DIGITAL_MIC)
  {
    Error_Handler();
  }
  if (BSP_ERROR_NONE != BSP_AUDIO_IN_GetBitsPerSample(1, &GetData))
  {
    Error_Handler();
  }
  if (GetData != AUDIO_RESOLUTION_16B)
  {
    Error_Handler();
  }
  if (BSP_ERROR_NONE != BSP_AUDIO_IN_GetChannelsNbr(1, &GetData))
  {
    Error_Handler();
  }
  if (GetData != 1U)
  {
    Error_Handler();
  }

  /* Set the initial sample rate */
  if (BSP_ERROR_NONE != BSP_AUDIO_IN_SetSampleRate(1, AudioInit.SampleRate))
  {
    Error_Handler();
  }
}

/**
  * @brief  Logs the NPU and NIC clock configuration.
  * @param  None
  * @retval None
  */
static void NPU_SettingsLog(void)
{
  struct mcu_conf sys_conf;

  getSysConf(&sys_conf);
  my_printf("\n\rNPU Runtime configuration...\r\n");
  my_printf(" NPU clock    : %u MHz\r\n", (int)sys_conf.extra[1] / 1000000);
  my_printf(" NIC clock    : %u MHz\r\n", (int)sys_conf.extra[2] / 1000000);
}

/**
  * @brief  Configures the external NOR flash and RAM memories.
  * @param  None
  * @retval None
  */
static void Ext_Mem_Config(void)
{
  BSP_XSPI_NOR_Init_t flash;
  flash.InterfaceMode = MX66UW1G45G_OPI_MODE;
  flash.TransferRate = MX66UW1G45G_DTR_TRANSFER;

  if (BSP_XSPI_NOR_Init(0, &flash) != BSP_ERROR_NONE)
  {
    __BKPT(0);
  }
  BSP_XSPI_NOR_EnableMemoryMappedMode(0);
  MODIFY_REG(XSPI2->CR, XSPI_CR_NOPREF, HAL_XSPI_AUTOMATIC_PREFETCH_DISABLE); /* Hotfix for XSPI: disable prefetch. */

#ifdef USE_EXT_SRAM
  BSP_XSPI_RAM_Init(0);
  BSP_XSPI_RAM_EnableMemoryMappedMode(0);
  MODIFY_REG(XSPI1->CR, XSPI_CR_NOPREF, HAL_XSPI_AUTOMATIC_PREFETCH_DISABLE); /* Hotfix for xspi: no prefetch */
#endif /* USE_EXT_SRAM */
}

/**
  * @brief  Configures the illegal access controller.
  * @param  None
  * @retval None
  */
static void IAC_Config(void)
{
  /* Configure IAC to trap illegal access events. */
  __HAL_RCC_IAC_CLK_ENABLE();
  __HAL_RCC_IAC_FORCE_RESET();
  __HAL_RCC_IAC_RELEASE_RESET();
}

/**
  * @brief  Configures the MPU non-cacheable memory region.
  * @param  None
  * @retval None
  */
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef default_config = {0};
  MPU_Attributes_InitTypeDef attr_config = {0};
  uint32_t primask_bit = __get_PRIMASK();
  __disable_irq();
  /* Disable the MPU. */
  HAL_MPU_Disable();
  /* Create an attribute configuration for the MPU. */
  attr_config.Attributes = INNER_OUTER(MPU_NOT_CACHEABLE);
  attr_config.Number = MPU_ATTRIBUTES_NUMBER0;
  HAL_MPU_ConfigMemoryAttributes(&attr_config);
  /* Create a non-cacheable region. */
  /* Normal memory type, code execution allowed. */
  default_config.Enable = MPU_REGION_ENABLE;
  default_config.Number = MPU_REGION_NUMBER0;
  default_config.BaseAddress = __NON_CACHEABLE_SECTION_BEGIN;
  default_config.LimitAddress = __NON_CACHEABLE_SECTION_END;
  default_config.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  default_config.AccessPermission = MPU_REGION_ALL_RW;
  default_config.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  default_config.AttributesIndex = MPU_ATTRIBUTES_NUMBER0;
  HAL_MPU_ConfigRegion(&default_config);
  /* Enable the MPU. */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
  /* Exit the critical section and lock the MPU configuration. */
  __set_PRIMASK(primask_bit);
}

/**
  * @brief  Enables peripheral clocks during low-power sleep modes.
  * @param  None
  * @retval None
  */
static void SleepClks_init(void)
{
    /* Keep all IPs enabled during WFE so they can wake up the CPU. Fine-tune
      this configuration when maximum power saving is required. This is
      especially needed when LL_ATON_RT_MODE=LL_ATON_RT_ASYNC is used.
  */
  LL_BUS_EnableClockLowPower(~0);
  LL_MEM_EnableClockLowPower(~0);
  LL_AHB1_GRP1_EnableClockLowPower(~0);
  LL_AHB2_GRP1_EnableClockLowPower(~0);
  LL_AHB3_GRP1_EnableClockLowPower(~0);
  LL_AHB4_GRP1_EnableClockLowPower(~0);
  LL_AHB5_GRP1_EnableClockLowPower(~0);
  LL_APB1_GRP1_EnableClockLowPower(~0);
  LL_APB1_GRP2_EnableClockLowPower(~0);
  LL_APB2_GRP1_EnableClockLowPower(~0);
  LL_APB4_GRP1_EnableClockLowPower(~0);
  LL_APB4_GRP2_EnableClockLowPower(~0);
  LL_APB5_GRP1_EnableClockLowPower(~0);
  LL_MISC_EnableClockLowPower(~0);
}

/**
  * @brief  Configures security attributes for the required peripherals.
  * @param  None
  * @retval None
  */
static void Security_Config(void)
{
  __HAL_RCC_RIFSC_CLK_ENABLE();
  RIMC_MasterConfig_t RIMC_master = {0};
  RIMC_master.MasterCID = RIF_CID_1;
  RIMC_master.SecPriv = RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV;
  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_DMA2D, &RIMC_master);
  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_LTDC1 , &RIMC_master);
  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_LTDC2 , &RIMC_master);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_DMA2D , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_LTDC   , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_LTDCL1 , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_LTDCL2 , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
}
