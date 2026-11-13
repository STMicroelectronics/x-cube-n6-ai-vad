##########################################################################
# Makefile for x-cube-n6-ai-vad
# Mirrors the STM32CubeIDE managed-build configuration (BM/BM_PQ) exactly:
# flags/sources below were derived from a real cold headless CubeIDE build's
# verbose compile log and .d dependency files, not by hand-parsing .cproject.
# Toolchain: Cortex-M55 / STM32N657xx (STM32N6570-DK).
#
# Usage:
#   make                                  -> AI_MODEL=full (default)
#   make AI_MODEL=partial                 -> partial-quantization variant
#   make EXTRA_DEFS=-DAPP_LOW_LAT_OFF      -> opt out of the low-latency default
#   make clean
##########################################################################

TARGET   := x-cube-n6-ai-vad
AI_MODEL ?= full

VALID_AI_MODELS := partial full
ifneq ($(words $(AI_MODEL)),1)
  $(error AI_MODEL must be exactly one of: $(VALID_AI_MODELS))
endif
ifeq ($(filter $(AI_MODEL),$(VALID_AI_MODELS)),)
  $(error Unsupported AI_MODEL '$(AI_MODEL)'; expected one of: $(VALID_AI_MODELS))
endif

ifeq ($(AI_MODEL),full)
  AI_MODEL_DEFS := -DAI_MODEL_FULL
else
  AI_MODEL_DEFS :=
endif

# EXTRA_DEFS: additional -D flags appended by the caller, e.g.
#   make EXTRA_DEFS=-DAPP_LOW_LAT_OFF
EXTRA_DEFS ?=

# Optional UART header strings (see Src/audio_bm.c's printAppHeader()), e.g.:
#   make APP_GIT_SHA1_STRING=$(git rev-parse --short HEAD) APP_VERSION_STRING=1.2.3
ifdef APP_GIT_SHA1_STRING
EXTRA_DEFS += -DAPP_GIT_SHA1_STRING=\"$(APP_GIT_SHA1_STRING)\"
endif
ifdef APP_VERSION_STRING
EXTRA_DEFS += -DAPP_VERSION_STRING=\"$(APP_VERSION_STRING)\"
endif

# Output directory must not collide across AI_MODEL/EXTRA_DEFS combinations.
empty :=
space := $(empty) $(empty)
EXTRA_SUFFIX := $(if $(strip $(EXTRA_DEFS)),_$(subst $(space),_,$(strip $(subst =,_,$(subst -D,,$(EXTRA_DEFS))))),)
BUILD_DIR := Build/$(AI_MODEL)$(EXTRA_SUFFIX)

######################################
# Toolchain
######################################
PREFIX := arm-none-eabi-
CC     := $(PREFIX)gcc
AS     := $(PREFIX)gcc
CP     := $(PREFIX)objcopy
SZ     := $(PREFIX)size
HEX    := $(CP) -O ihex
BIN    := $(CP) -O binary -S

######################################
# CPU / FPU flags (Cortex-M55, hard FP). -mcmse is compile-only (secure
# gateway attribute); it is NOT passed to the final link step.
######################################
CPU_FLAGS  := -mcpu=cortex-m55 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard
CMSE_FLAG  := -mcmse

######################################
# C defines (order mirrors .cproject's definedsymbols list)
######################################
C_DEFS_COMMON := \
  -DSTM32N657xx \
  $(AI_MODEL_DEFS) \
  -DARM_MATH_CM55 \
  -DUSE_EXT_SRAM \
  -DUSE_FULL_ASSERT \
  -DUSE_FULL_LL_DRIVER \
  -DVECT_TAB_SRAM \
  -DLL_ATON_PLATFORM=LL_ATON_PLAT_STM32N6 \
  -DLL_ATON_OSAL=LL_ATON_OSAL_BARE_METAL \
  -DLL_ATON_RT_MODE=LL_ATON_RT_ASYNC \
  -DLL_ATON_SW_FALLBACK \
  -DLL_ATON_EB_DBG_INFO \
  -DLL_ATON_DUMP_DEBUG_API \
  -DLL_ATON_DBG_BUFFER_INFO_EXCLUDED=1 \
  $(EXTRA_DEFS)

# Assembler-only defines observed on startup_stm32n657xx.s in the reference
# build (kept for exact fidelity with the IDE build).
AS_DEFS := -DDEBUG

######################################
# Include paths (paths are relative to this Makefile, i.e. Projects/GS/)
######################################
C_INCLUDES := \
  -IInc \
  -I../Dpu \
  -I../Common \
  -I../X-CUBE-AI \
  -I../X-CUBE-AI/models/$(AI_MODEL) \
  -I../../STM32Cube_FW_N6/Drivers/CMSIS/Core/Include \
  -I../../STM32Cube_FW_N6/Drivers/CMSIS/DSP/Include \
  -I../../STM32Cube_FW_N6/Drivers/CMSIS/DSP/PrivateInclude \
  -I../../STM32Cube_FW_N6/Drivers/CMSIS/Device/ST/STM32N6xx/Include \
  -I../../STM32Cube_FW_N6/Drivers/CMSIS/Device/ST/STM32N6xx/Include/Templates \
  -I../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Inc \
  -I../../Middlewares/ST/stedgeai-lib/Inc \
  -I../../Middlewares/ST/stedgeai-lib/Misc/Inc \
  -I../../Middlewares/ST/stedgeai-lib/Npu/Devices/STM32N6xx \
  -I../../Middlewares/ST/stedgeai-lib/Npu/ll_aton \
  -I../../Middlewares/ST/stm32-audio-sensing-processing/Inc \
  -I../../STM32Cube_FW_N6/Drivers/BSP/Components/mx66uw1g45g \
  -I../../STM32Cube_FW_N6/Drivers/BSP/Components/rk050hr18 \
  -I../../STM32Cube_FW_N6/Drivers/BSP/Components/Common \
  -I../../STM32Cube_FW_N6/Drivers/BSP/STM32N6570-DK \
  -I../../STM32Cube_FW_N6/Utilities/lcd

######################################
# C compiler flags (matches the reference build's -O3/-g3/-Wall/etc. set)
######################################
CFLAGS := $(CPU_FLAGS) $(CMSE_FLAG) -std=gnu11 -g3 -O3 $(C_DEFS_COMMON) $(C_INCLUDES) \
  -ffunction-sections -fdata-sections -Wall -flax-vector-conversions \
  -fstack-usage -fcyclomatic-complexity \
  --specs=nano.specs \
  -MMD -MP

######################################
# Assembler flags
######################################
ASFLAGS := $(CPU_FLAGS) -g3 $(AS_DEFS) $(C_INCLUDES) \
  -x assembler-with-cpp \
  --specs=nano.specs \
  -MMD -MP

######################################
# Linker
######################################
LDSCRIPT := STM32CubeIDE/STM32N657XX_LRUN.ld

# NOTE: the reference build passes BOTH --specs=nosys.specs and
# --specs=nano.specs on the link line (confirmed via verbose build log);
# kept as-is for exact fidelity even though this looks redundant.
# --no-warn-rwx-segments is a Makefile-only addition (not in the reference
# build): silences ld's "LOAD segment with RWX permissions" warning.
LDFLAGS := $(CPU_FLAGS) \
  -T$(LDSCRIPT) \
  --specs=nosys.specs --specs=nano.specs \
  -Wl,-Map=$(BUILD_DIR)/$(TARGET).map \
  -Wl,--gc-sections \
  -Wl,--no-warn-rwx-segments \
  -static \
  -Wl,--cmse-implib -Wl,--out-implib=$(BUILD_DIR)/secure_nsclib.o \
  -L../../Middlewares/ST/stedgeai-lib/Lib/GCC/ARMCortexM55 \
  -l:NetworkRuntime1210_CM55_GCC.a \
  -u _printf_float \
  -Wl,--start-group -lc -lm -Wl,--end-group

######################################
# Signing / flashing (STM32CubeProgrammer + SigningTool must be on PATH;
# same tools/external loader as Projects/X-CUBE-AI/models/sign-and-flash-app.sh
# and README.md's manual flashing steps)
######################################
FLASHER   := STM32_Programmer_CLI
SIGNER    := STM32_SigningTool_CLI
SIGN_ADDR := 0x70100000
EL        := $(shell dirname "$(shell which $(FLASHER))")/ExternalLoader/MX66UW1G45G_STM32N6570-DK.stldr

######################################
# Application – Core (Projects/GS/Src)
######################################
C_SOURCES := \
  Src/audio_bm.c \
  Src/AudioCapture_ring_buff.c \
  Src/main.c \
  Src/stm32_lcd_ex.c \
  Src/stm32n6xx_it.c \
  Src/syscalls.c \
  Src/sysmem.c

######################################
# Application – Common
######################################
C_SOURCES += \
  ../Common/cpu_stats.c \
  ../Common/logging.c \
  ../Common/misc_toolbox.c \
  ../Common/system_clock_config.c \
  ../Common/Patch/stm32n6570_discovery_audio.c \
  ../Common/Patch/stm32n6570_discovery_audio.patch.hsi_600_400.c

######################################
# Application – DPU
######################################
C_SOURCES += \
  ../Dpu/ai_dpu.c \
  ../Dpu/preproc_dpu.c \
  ../Dpu/user_mel_tables.c

######################################
# Application – X-CUBE-AI glue + AI model (path parameterized by AI_MODEL)
######################################
C_SOURCES += \
  ../X-CUBE-AI/aiTestHelper_ST_AI.c \
  ../X-CUBE-AI/models/$(AI_MODEL)/network.c \
  ../X-CUBE-AI/models/$(AI_MODEL)/stai_network.c

######################################
# Middlewares – AI Misc
######################################
C_SOURCES += \
  ../../Middlewares/ST/stedgeai-lib/Misc/Src/lc_print.c \
  ../../Middlewares/ST/stedgeai-lib/Misc/Src/ai_device_adaptor.c \
  ../../Middlewares/ST/stedgeai-lib/Misc/Src/aiTestUtility.c

######################################
# Middlewares – AI Npu Devices
######################################
C_SOURCES += \
  ../../Middlewares/ST/stedgeai-lib/Npu/Devices/STM32N6xx/mcu_cache.c \
  ../../Middlewares/ST/stedgeai-lib/Npu/Devices/STM32N6xx/npu_cache.c

######################################
# Middlewares – AI Npu ll_aton (NPU/ATON runtime)
######################################
C_SOURCES += \
  ../../Middlewares/ST/stedgeai-lib/Npu/ll_aton/ecloader.c \
  ../../Middlewares/ST/stedgeai-lib/Npu/ll_aton/ll_aton_cipher.c \
  ../../Middlewares/ST/stedgeai-lib/Npu/ll_aton/ll_aton_dbgtrc.c \
  ../../Middlewares/ST/stedgeai-lib/Npu/ll_aton/ll_aton_debug.c \
  ../../Middlewares/ST/stedgeai-lib/Npu/ll_aton/ll_aton_lib.c \
  ../../Middlewares/ST/stedgeai-lib/Npu/ll_aton/ll_aton_lib_sw_operators.c \
  ../../Middlewares/ST/stedgeai-lib/Npu/ll_aton/ll_aton.c \
  ../../Middlewares/ST/stedgeai-lib/Npu/ll_aton/ll_aton_osal_zephyr.c \
  ../../Middlewares/ST/stedgeai-lib/Npu/ll_aton/ll_aton_profiler.c \
  ../../Middlewares/ST/stedgeai-lib/Npu/ll_aton/ll_aton_reloc_callbacks.c \
  ../../Middlewares/ST/stedgeai-lib/Npu/ll_aton/ll_aton_reloc_network.c \
  ../../Middlewares/ST/stedgeai-lib/Npu/ll_aton/ll_aton_rt_main_irq_sched.c \
  ../../Middlewares/ST/stedgeai-lib/Npu/ll_aton/ll_aton_rt_main.c \
  ../../Middlewares/ST/stedgeai-lib/Npu/ll_aton/ll_aton_runtime.c \
  ../../Middlewares/ST/stedgeai-lib/Npu/ll_aton/ll_aton_stai_internal.c \
  ../../Middlewares/ST/stedgeai-lib/Npu/ll_aton/ll_aton_util.c \
  ../../Middlewares/ST/stedgeai-lib/Npu/ll_aton/ll_sw_float.c \
  ../../Middlewares/ST/stedgeai-lib/Npu/ll_aton/ll_sw_integer.c

######################################
# Middlewares – Audio Preprocessing Library
######################################
C_SOURCES += \
  ../../Middlewares/ST/stm32-audio-sensing-processing/Src/audio_din_f16.c \
  ../../Middlewares/ST/stm32-audio-sensing-processing/Src/audio_din.c \
  ../../Middlewares/ST/stm32-audio-sensing-processing/Src/audio_prePost_process_f16.c \
  ../../Middlewares/ST/stm32-audio-sensing-processing/Src/audio_prePost_process.c \
  ../../Middlewares/ST/stm32-audio-sensing-processing/Src/common_tables_f16.c \
  ../../Middlewares/ST/stm32-audio-sensing-processing/Src/common_tables.c \
  ../../Middlewares/ST/stm32-audio-sensing-processing/Src/dct_f16.c \
  ../../Middlewares/ST/stm32-audio-sensing-processing/Src/dct.c \
  ../../Middlewares/ST/stm32-audio-sensing-processing/Src/feature_extraction_f16.c \
  ../../Middlewares/ST/stm32-audio-sensing-processing/Src/feature_extraction.c \
  ../../Middlewares/ST/stm32-audio-sensing-processing/Src/mel_filterbank_f16.c \
  ../../Middlewares/ST/stm32-audio-sensing-processing/Src/mel_filterbank.c \
  ../../Middlewares/ST/stm32-audio-sensing-processing/Src/window_f16.c \
  ../../Middlewares/ST/stm32-audio-sensing-processing/Src/window.c

######################################
# STM32N6570-DK BSP components (audio codec, PSRAM/xSPI, LCD panel)
######################################
C_SOURCES += \
  ../../STM32Cube_FW_N6/Drivers/BSP/Components/aps256xx/aps256xx.c \
  ../../STM32Cube_FW_N6/Drivers/BSP/Components/cs42l51/cs42l51.c \
  ../../STM32Cube_FW_N6/Drivers/BSP/Components/cs42l51/cs42l51_reg.c \
  ../../STM32Cube_FW_N6/Drivers/BSP/Components/mx66uw1g45g/mx66uw1g45g.c \
  ../../STM32Cube_FW_N6/Drivers/BSP/Components/wm8904/wm8904.c \
  ../../STM32Cube_FW_N6/Drivers/BSP/Components/wm8904/wm8904_reg.c

######################################
# STM32N6570-DK BSP board layer
######################################
C_SOURCES += \
  ../../STM32Cube_FW_N6/Drivers/BSP/STM32N6570-DK/stm32n6570_discovery_bus.c \
  ../../STM32Cube_FW_N6/Drivers/BSP/STM32N6570-DK/stm32n6570_discovery_lcd.c \
  ../../STM32Cube_FW_N6/Drivers/BSP/STM32N6570-DK/stm32n6570_discovery.c \
  ../../STM32Cube_FW_N6/Drivers/BSP/STM32N6570-DK/stm32n6570_discovery_xspi.c \
  ../../STM32Cube_FW_N6/Utilities/lcd/stm32_lcd.c

######################################
# CMSIS DSP — each group's aggregator .c #includes its individual arm_*.c
# implementation files internally; CubeIDE compiles ONLY the aggregator per
# group (confirmed against the reference build's object list). Do NOT also
# list the individual arm_*.c files here, or their symbols are defined twice.
######################################
C_SOURCES += \
  ../../STM32Cube_FW_N6/Drivers/CMSIS/DSP/Source/BasicMathFunctions/BasicMathFunctions.c \
  ../../STM32Cube_FW_N6/Drivers/CMSIS/DSP/Source/BasicMathFunctions/BasicMathFunctionsF16.c \
  ../../STM32Cube_FW_N6/Drivers/CMSIS/DSP/Source/CommonTables/CommonTables.c \
  ../../STM32Cube_FW_N6/Drivers/CMSIS/DSP/Source/CommonTables/CommonTablesF16.c \
  ../../STM32Cube_FW_N6/Drivers/CMSIS/DSP/Source/ComplexMathFunctions/ComplexMathFunctions.c \
  ../../STM32Cube_FW_N6/Drivers/CMSIS/DSP/Source/ComplexMathFunctions/ComplexMathFunctionsF16.c \
  ../../STM32Cube_FW_N6/Drivers/CMSIS/DSP/Source/StatisticsFunctions/StatisticsFunctions.c \
  ../../STM32Cube_FW_N6/Drivers/CMSIS/DSP/Source/StatisticsFunctions/StatisticsFunctionsF16.c \
  ../../STM32Cube_FW_N6/Drivers/CMSIS/DSP/Source/SupportFunctions/SupportFunctions.c \
  ../../STM32Cube_FW_N6/Drivers/CMSIS/DSP/Source/SupportFunctions/SupportFunctionsF16.c \
  ../../STM32Cube_FW_N6/Drivers/CMSIS/DSP/Source/TransformFunctions/TransformFunctions.c \
  ../../STM32Cube_FW_N6/Drivers/CMSIS/DSP/Source/TransformFunctions/TransformFunctionsF16.c

######################################
# CMSIS Device (system init; startup is assembly, listed separately below)
######################################
C_SOURCES += \
  ../../STM32Cube_FW_N6/Drivers/CMSIS/Device/ST/STM32N6xx/Source/Templates/system_stm32n6xx_fsbl.c

######################################
# STM32N6xx HAL Driver
######################################
C_SOURCES += \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_bsec.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_cacheaxi.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_cortex.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_dma2d.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_dma_ex.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_dma.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_exti.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_gpio.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_i2c_ex.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_i2c.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_icache.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_ltdc_ex.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_ltdc.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_mdf.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_mdios.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_pwr_ex.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_pwr.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_ramcfg.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_rcc_ex.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_rcc.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_rif.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_sai.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_sdio.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_uart.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_xspi.c \
  ../../STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_util_i3c.c

######################################
# Assembly sources
######################################
ASM_SOURCES := \
  ../../STM32Cube_FW_N6/Drivers/CMSIS/Device/ST/STM32N6xx/Source/Templates/gcc/startup_stm32n657xx.s

######################################
# Build rules
######################################
# Several C_SOURCES/ASM_SOURCES entries climb out of Projects/GS/ with "../"
# or "../../" (Common, Dpu, X-CUBE-AI, STM32Cube_FW_N6, Middlewares). Naively
# prefixing $(BUILD_DIR) onto a path containing ".." lets the ".." cancel part
# of BUILD_DIR itself, so the resulting object can land OUTSIDE $(BUILD_DIR)
# entirely (e.g. spilling untracked STM32Cube_FW_N6/Middlewares
# straight into Projects/GS/) instead of being contained per-AI_MODEL - which
# is exactly the directory-collision risk this Makefile must avoid. Fix: turn
# every source into its canonical path relative to the repo root (no ".."
# components at all) via $(abspath) before joining with $(BUILD_DIR); the
# build rules below then reconstruct the real on-disk absolute source path
# from that same canonical suffix.
REPO_ROOT := $(abspath $(CURDIR)/../..)

canon = $(patsubst $(REPO_ROOT)/%,%,$(abspath $(1)))

C_SOURCES_CANON   := $(call canon,$(C_SOURCES))
ASM_SOURCES_CANON := $(call canon,$(ASM_SOURCES))

C_OBJECTS   := $(addprefix $(BUILD_DIR)/,$(C_SOURCES_CANON:.c=.o))
ASM_OBJECTS := $(addprefix $(BUILD_DIR)/,$(ASM_SOURCES_CANON:.s=.o))
ALL_OBJECTS := $(C_OBJECTS) $(ASM_OBJECTS)
DEPS        := $(ALL_OBJECTS:.o=.d)

.PHONY: all clean size hex bin print-build-dir sign flash flash_fsbl flash_weights

all: $(BUILD_DIR)/$(TARGET).elf size

# Used by the target tier of Utils/validation/validate.sh: print BUILD_DIR
# without re-deriving EXTRA_SUFFIX's string transform elsewhere, which is
# exactly the kind of duplicated logic that silently drifts out of sync (a
# hand-rolled sed reimplementation of EXTRA_SUFFIX once produced a different
# result than this Makefile's own for multi-flag EXTRA_DEFS).
print-build-dir:
	@echo $(BUILD_DIR)

# Link
$(BUILD_DIR)/$(TARGET).elf: $(ALL_OBJECTS) $(LDSCRIPT)
	$(CC) $(ALL_OBJECTS) $(LDFLAGS) -o $@

# C source -> object (prerequisite is the real absolute source path)
$(BUILD_DIR)/%.o: $(REPO_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MF"$(@:.o=.d)" -MT"$@" -c $< -o $@

# Assembly source -> object (prerequisite is the real absolute source path)
$(BUILD_DIR)/%.o: $(REPO_ROOT)/%.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -MF"$(@:.o=.d)" -MT"$@" -c $< -o $@

size: $(BUILD_DIR)/$(TARGET).elf
	$(SZ) $<

$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	$(HEX) $< $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(BIN) $< $@

hex: $(BUILD_DIR)/$(TARGET).hex

bin: $(BUILD_DIR)/$(TARGET).bin

# Signed app binary/hex (ssbl, loaded at $(SIGN_ADDR)); network weights are
# flashed separately (flash_weights) since they are not linked into the ELF.
$(BUILD_DIR)/$(TARGET)_signed.bin: $(BUILD_DIR)/$(TARGET).bin
	$(SIGNER) -s -bin $< -nk -t ssbl -hv 2.3 --align -o $@

$(BUILD_DIR)/$(TARGET)_signed.hex: $(BUILD_DIR)/$(TARGET)_signed.bin
	$(CP) -I binary $< --change-addresses $(SIGN_ADDR) -O ihex $@

sign: $(BUILD_DIR)/$(TARGET)_signed.hex

flash: $(BUILD_DIR)/$(TARGET)_signed.hex
	$(FLASHER) -c port=SWD mode=HOTPLUG -el $(EL) -hardRst -w $<

flash_fsbl:
	$(FLASHER) -c port=SWD mode=HOTPLUG -el $(EL) -hardRst -w ../../FSBL/ai_fsbl.hex

flash_weights:
	$(FLASHER) -c port=SWD mode=HOTPLUG -el $(EL) -hardRst -w ../X-CUBE-AI/models/$(AI_MODEL)/network_data.hex

clean:
	rm -rf Build

-include $(DEPS)
