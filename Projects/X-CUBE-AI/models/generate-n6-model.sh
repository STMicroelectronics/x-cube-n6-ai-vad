#!/bin/bash
# generate-n6-model.sh [full|partial]
#
# Generates, with ST Edge AI, the STM32N6 network sources and the NPU weights
# of the TEN-VAD model variants from their quantized ONNX models:
#   full    <- ten-vad-fixed_dec_full_qdq_w8a8.onnx
#   partial <- ten-vad-fixed_dec_partial_qdq_w8a8.onnx
# Omit the argument to generate both variants.
#
# For each variant, the files below end up in Projects/X-CUBE-AI/models/<variant>/:
#   network.c/.h, stai_network.c/.h  network sources compiled in the application
#   network_generate_report.txt      ST Edge AI generation report
#   network_data.hex                 NPU weights, addressed at 0x70180000 in the
#                                    external flash (see flash-weights.sh)
#
# Prerequisites: stedgeai and arm-none-eabi-objcopy on PATH. Set STEDGEAI to use
# an ST Edge AI executable that lives elsewhere.
#
# Run from anywhere, e.g.:
#   Projects/X-CUBE-AI/models/generate-n6-model.sh full

set -euo pipefail

STEDGEAI="${STEDGEAI:-stedgeai}"

# Everything below uses paths relative to this directory
cd "$(dirname "${BASH_SOURCE[0]}")"

generate_variant()
{
  model="$1"
  variant="$2"

  # --st-neural-art selects the "default" profile of user_neural_art.json (memory pool + compiler options)
  "$STEDGEAI" generate -m "$model" --target stm32n6 --st-neural-art default@user_neural_art.json

  mkdir -p "$variant"
  cp ./st_ai_output/network.c "$variant/network.c"
  cp ./st_ai_output/network.h "$variant/network.h"
  cp ./st_ai_output/stai_network.c "$variant/stai_network.c"
  cp ./st_ai_output/stai_network.h "$variant/stai_network.h"
  cp ./st_ai_output/network_generate_report.txt "$variant/network_generate_report.txt"
  # The weights are placed in external flash (xSPI2): convert the raw binary to Intel HEX
  # at the address they are flashed to, so that flashing needs no address argument
  cp ./st_ai_output/network_atonbuf.xSPI2.raw network_data.bin
  arm-none-eabi-objcopy -I binary network_data.bin --change-addresses 0x70180000 -O ihex "$variant/network_data.hex"
  rm network_data.bin
}

variant="${1:-}"

case "$variant" in
  full)    generate_variant ten-vad-fixed_dec_full_qdq_w8a8.onnx full ;;
  partial) generate_variant ten-vad-fixed_dec_partial_qdq_w8a8.onnx partial ;;
  "")
    generate_variant ten-vad-fixed_dec_full_qdq_w8a8.onnx full
    generate_variant ten-vad-fixed_dec_partial_qdq_w8a8.onnx partial
    ;;
  *)
    echo "Usage: $0 [full|partial]" >&2
    exit 1
    ;;
esac