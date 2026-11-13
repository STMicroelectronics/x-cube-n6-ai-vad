#!/bin/bash
#
# Flash a prebuilt x-cube-n6-ai-vad binary onto an STM32N6570-DK board.
#
# Usage (from any directory):
#   ./flash-bin.sh <full|partial>
#
#   <full|partial>  quantization variant to flash; selects
#                   STM32N6570-DK/x-cube-n6-ai-vad_<variant>.hex
#
# Prerequisite: STM32_Programmer_CLI (STM32CubeProgrammer) must be in the PATH.
# The hex file contains the FSBL and the network weights, so a single flash
# is enough.

set -euo pipefail

variant="${1:-}"

# Only the two shipped quantization variants exist
case "$variant" in
  full|partial) ;;
  *)
    echo "Usage: $0 <full|partial>" >&2
    exit 1
    ;;
esac

# Hex file to flash, located relative to this script
hex="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/STM32N6570-DK/x-cube-n6-ai-vad_${variant}.hex"
if [ ! -f "$hex" ]; then
  echo "error: $hex not found" >&2
  exit 1
fi

prog="$(command -v STM32_Programmer_CLI)" || {
  echo "error: STM32_Programmer_CLI not found on PATH" >&2
  exit 1
}
# The external loader (needed to program the external flash) is expected in the
# ExternalLoader folder next to the STM32_Programmer_CLI executable
external_loader="$(dirname "$prog")/ExternalLoader/MX66UW1G45G_STM32N6570-DK.stldr"

# BOOT1 to the right position puts the board in dev boot mode (needed to program)
echo "please connect the board and switch BOOT1 to Right position"
echo "when done, press a key to continue ..."
# Wait for a single key press, without echoing it
read -n 1 -s
echo "flashing the application $hex"

# Print the programmer command line as it is executed
set -x

# Connect over SWD in hot-plug mode on access port 1, load the external flash
# loader, then write the hex file
"$prog" -c port=swd mode=HOTPLUG ap=1 --extload "$external_loader" -w "$hex"

set +x

# BOOT1 to the left position makes the board boot from flash on next power up
echo "please switch BOOT1 to Left position and then power cycle the board"
echo "when done, press a key to continue ..."
read -n 1 -s
echo "Flashing done"
