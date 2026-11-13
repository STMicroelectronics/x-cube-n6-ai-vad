#!/bin/bash
# flash-weights.sh <full|partial>
#
# Flashes the NPU weights for the given TEN-VAD model variant to external
# xSPI flash. No separate address argument is needed: it is embedded in
# network_data.hex's own address records (0x70180000).
#
# Each variant's weights live in their own directory,
# Projects/X-CUBE-AI/models/<variant>/network_data.hex (see
# generate-n6-model.sh). They must match the quantization variant of the
# application you run.
#
# Programmer used:
#   - flash_N6, when it is on PATH (WSL2 -> Windows STM32CubeProgrammer wrapper)
#   - otherwise STM32_Programmer_CLI, which must be on PATH. Its external
#     loader is looked up in the ExternalLoader/ folder next to it; set
#     EXTERNAL_LOADER to override.
#
# The board must be in development boot mode (BOOT1 switch to the right
# position).
#
# Run from anywhere, e.g.:
#   Projects/X-CUBE-AI/models/flash-weights.sh full

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

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
hex="$script_dir/$variant/network_data.hex"

if [ ! -f "$hex" ]; then
  echo "error: $hex not found - run Projects/X-CUBE-AI/models/generate-n6-model.sh first" >&2
  exit 1
fi

echo "Flashing $variant weights: $hex"
if command -v flash_N6 >/dev/null; then
  flash_N6 "$hex"
else
  prog="$(command -v STM32_Programmer_CLI)" || {
    echo "error: neither flash_N6 nor STM32_Programmer_CLI found on PATH" >&2
    exit 1
  }
  el="${EXTERNAL_LOADER:-$(dirname "$prog")/ExternalLoader/MX66UW1G45G_STM32N6570-DK.stldr}"
  "$prog" -c port=swd mode=HOTPLUG ap=1 --extload "$el" -w "$hex"
fi
