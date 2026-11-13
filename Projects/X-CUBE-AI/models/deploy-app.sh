#!/bin/bash
# deploy-app.sh <full|partial> [hl]
#
# End-to-end deployment of the TEN-VAD application for a model variant, from
# its quantized ONNX model to the application running on the STM32N6570-DK
# board:
#   1. generate-n6-model.sh      ONNX model -> network sources + NPU weights
#   2. make (Projects/GS)        build the application with the new sources
#   3. sign-and-flash-app.sh     sign it, combine it with the FSBL and the NPU
#                                weights, and flash the result
#
# [hl] deploys the high-latency build (EXTRA_DEFS=-DAPP_LOW_LAT_OFF) instead
# of the default low-latency one.
#
# See the individual scripts for their prerequisites (stedgeai,
# arm-none-eabi-gcc/objcopy, make, STM32_Programmer_CLI and
# STM32_SigningTool_CLI on PATH).
#
# Run from anywhere, e.g.:
#   Projects/X-CUBE-AI/models/deploy-app.sh full

set -euo pipefail

variant="${1:-}"
lat="${2:-}"

case "$variant" in
  full|partial) ;;
  *)
    echo "Usage: $0 <full|partial> [hl]" >&2
    exit 1
    ;;
esac

case "$lat" in
  ""|hl) ;;
  *)
    echo "Usage: $0 <full|partial> [hl]" >&2
    exit 1
    ;;
esac

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "==== Generating model for N6 ===="
"$script_dir/generate-n6-model.sh" "$variant"

# Make arguments: the model variant, plus the opt-out of the low-latency
# default for the high-latency build
make_args=(AI_MODEL="$variant")
if [ "$lat" = "hl" ]; then
  make_args+=(EXTRA_DEFS=-DAPP_LOW_LAT_OFF)
fi

# The binary lands in Projects/GS/Build/<variant>[_APP_LOW_LAT_OFF]/, where
# sign-and-flash-app.sh picks it up
echo "==== Building FW ===="
make -C "$script_dir/../../GS" -j8 "${make_args[@]}" bin

# Only the flashing step needs the board, so ask for it now rather than up front
echo "please connect the board and switch BOOT1 to Right position"
echo "when done, press a key to continue ..."
read -n 1 -s

echo "==== Flashing FW ===="
"$script_dir/sign-and-flash-app.sh" "$variant" "$lat"

echo "please power cycle the board after switching BOOT1 to Left position"