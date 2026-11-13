# Release Notes for x-cube-n6-ai-vad Application

## Purpose

This project provides an STM32 Microcontroller embedded real time environment
to execute [X-CUBE-AI](https://www.st.com/en/embedded-software/x-cube-ai.html)
generated model targetting audio applications. The purpose of this package is
to stream physical data acquired by sensors into a processing chain including a
preprocessing step that typically would perform a first level of feature
extraction, the machine learning inference itself, and a post processing step
before exposing the results to the user in real time.

## Key Features

- Support of Voice Activity Detection (VAD) using TEN-VAD
- Bare Metal (BM) Implementation

## Software components

| Name                                | Version                                | Release notes
|-----                                | -------                                | -------------
| STEdge AI runtime                   |  v4.1.0                                |
| STM32 AI AudioPreprocessing Library |  v1.6                                  | [release notes](Middlewares/ST/stm32-audio-sensing-processing/Release_Notes.html)
| LCD Utility                         |  v2.2.0                                | [release notes](STM32Cube_FW_N6/Utilities/lcd/Release_Notes.html)
| CMSIS                               |  v6.2.0 (Core 6.1.1, DSP 1.16.2)       | [release notes](STM32Cube_FW_N6/Drivers/CMSIS/st_readme.txt)
| STM32N6xx CMSIS Device              |  v1.4.0                                | [release notes](STM32Cube_FW_N6/Drivers/CMSIS/Device/ST/STM32N6xx/Release_Notes.html)
| STM32N6xx HAL/LL Drivers            |  v1.4.0                                | [release notes](STM32Cube_FW_N6/Drivers/STM32N6xx_HAL_Driver/Release_Notes.html)
| STM32N6570-DK BSP Drivers           |  v1.4.1                                | [release notes](STM32Cube_FW_N6/Drivers/BSP/STM32N6570-DK/Release_Notes.html)
| BSP Component aps256xx              |  v1.0.6                                | [release notes](STM32Cube_FW_N6/Drivers/BSP/Components/aps256xx/Release_Notes.html)
| BSP Component cs42l51               |  v2.0.7                                | [release notes](STM32Cube_FW_N6/Drivers/BSP/Components/cs42l51/Release_Notes.html)
| BSP Component mx66uw1g45g           |  v1.1.0                                | [release notes](STM32Cube_FW_N6/Drivers/BSP/Components/mx66uw1g45g/Release_Notes.html)
| BSP Component rk050hr18             |  v1.0.1                                | [release notes](STM32Cube_FW_N6/Drivers/BSP/Components/rk050hr18/Release_Notes.html)
| BSP Component wm8904                |  v1.1.1                                | [release notes](STM32Cube_FW_N6/Drivers/BSP/Components/wm8904/Release_Notes.html)
| BSP Component Common                |  v7.3.0                                | [release notes](STM32Cube_FW_N6/Drivers/BSP/Components/Common/Release_Notes.html)
| STM32N6 FSBL                        |  v1.4.0                                | [release notes](FSBL/Release_Notes.md)

## Update history

### V1.0.0 / October 2026

- Initial Version

## AI disclosure

*Parts of this software were developed with the assistance of generative AI tools.*
