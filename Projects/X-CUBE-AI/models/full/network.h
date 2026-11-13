/**
  ******************************************************************************
  * @file    network.h
  * @author  STEdgeAI
  * @date    2026-10-23 20:15:40
  * @brief   Minimal description of the generated c-implemention of the network
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025-2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */
#ifndef LL_ATON_NETWORK_H
#define LL_ATON_NETWORK_H

/******************************************************************************/
#define LL_ATON_NETWORK_C_MODEL_NAME        "network"
#define LL_ATON_NETWORK_ORIGIN_MODEL_NAME   "tenvadfixed_dec_full_qdq_w8a8"

/************************** USER ALLOCATED IOs ********************************/
// No user allocated inputs
// No user allocated outputs

/************************** INPUTS ********************************************/
#define LL_ATON_NETWORK_IN_NUM        (5)    // Total number of input buffers
// Input buffer 1 -- Input_34_out_0
#define LL_ATON_NETWORK_IN_1_ALIGNMENT   (32)
#define LL_ATON_NETWORK_IN_1_SIZE_BYTES  (123)
// Input buffer 2 -- Input_27_out_0
#define LL_ATON_NETWORK_IN_2_ALIGNMENT   (32)
#define LL_ATON_NETWORK_IN_2_SIZE_BYTES  (64)
// Input buffer 3 -- Input_23_out_0
#define LL_ATON_NETWORK_IN_3_ALIGNMENT   (32)
#define LL_ATON_NETWORK_IN_3_SIZE_BYTES  (64)
// Input buffer 4 -- Input_16_out_0
#define LL_ATON_NETWORK_IN_4_ALIGNMENT   (32)
#define LL_ATON_NETWORK_IN_4_SIZE_BYTES  (64)
// Input buffer 5 -- Input_12_out_0
#define LL_ATON_NETWORK_IN_5_ALIGNMENT   (32)
#define LL_ATON_NETWORK_IN_5_SIZE_BYTES  (64)

/************************** OUTPUTS *******************************************/
#define LL_ATON_NETWORK_OUT_NUM        (5)    // Total number of output buffers
// Output buffer 1 -- Quantize_214_out_0
#define LL_ATON_NETWORK_OUT_1_ALIGNMENT   (32)
#define LL_ATON_NETWORK_OUT_1_SIZE_BYTES  (1)
// Output buffer 2 -- Quantize_121_out_0
#define LL_ATON_NETWORK_OUT_2_ALIGNMENT   (32)
#define LL_ATON_NETWORK_OUT_2_SIZE_BYTES  (64)
// Output buffer 3 -- Quantize_115_out_0
#define LL_ATON_NETWORK_OUT_3_ALIGNMENT   (32)
#define LL_ATON_NETWORK_OUT_3_SIZE_BYTES  (64)
// Output buffer 4 -- Quantize_175_out_0
#define LL_ATON_NETWORK_OUT_4_ALIGNMENT   (32)
#define LL_ATON_NETWORK_OUT_4_SIZE_BYTES  (64)
// Output buffer 5 -- Quantize_169_out_0
#define LL_ATON_NETWORK_OUT_5_ALIGNMENT   (32)
#define LL_ATON_NETWORK_OUT_5_SIZE_BYTES  (64)

#endif /* LL_ATON_NETWORK_H */
