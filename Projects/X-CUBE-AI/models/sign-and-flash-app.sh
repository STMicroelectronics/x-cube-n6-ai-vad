#!/bin/bash
# sign-and-flash-app.sh <full|partial> [hl]
#
# Signs the Projects/GS application binary built for the given AI model
# variant (see Projects/GS/Makefile, AI_MODEL=<full|partial>), combines it
# with the FSBL and the matching NPU weights into
# app_vad_<variant>[_hl].hex, and flashes the result.
#
# The board must be in development boot mode (BOOT1 switch to the right
# position). Switch it back to the left position and power cycle the board
# once flashed.
#
# Run from anywhere, e.g.:
#   Projects/X-CUBE-AI/models/sign-and-flash-app.sh full
#
# [hl] selects the high-latency build (Projects/GS/Makefile's
# EXTRA_DEFS=-DAPP_LOW_LAT_OFF, Build/<variant>_APP_LOW_LAT_OFF); omit it for
# the default low-latency build.

set -euo pipefail

# All the paths below are relative to this directory (Projects/X-CUBE-AI/models/)
cd "$(dirname "${BASH_SOURCE[0]}")"

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

# STM32CubeProgrammer + SigningTool must be on PATH (see README.md). The
# external loader is located in the ExternalLoader/ subdirectory next to
# STM32_Programmer_CLI; override with EXTERNAL_LOADER if your install layout
# differs. When the CLI is reached through a WSL drive mount (/mnt/<drive>/...),
# the --extload value is rewritten to the Windows-native <DRIVE>:/... form, since
# the (Windows) CLI resolves that path itself, not through WSL.
prog="$(command -v STM32_Programmer_CLI)" || {
  echo "error: STM32_Programmer_CLI not found on PATH" >&2
  exit 1
}
sign="$(command -v STM32_SigningTool_CLI)" || {
  echo "error: STM32_SigningTool_CLI not found on PATH" >&2
  exit 1
}
el="$(dirname "$prog")/ExternalLoader/MX66UW1G45G_STM32N6570-DK.stldr"
if [[ "$el" =~ ^/mnt/([A-Za-z])/(.*)$ ]]; then
  el="${BASH_REMATCH[1]}:/${BASH_REMATCH[2]}"
fi
el="${EXTERNAL_LOADER:-$el}"

# Where the application was built (Makefile BUILD_DIR layout)
if [ "$lat" = "hl" ]; then
  build_dir="../../GS/Build/${variant}_APP_LOW_LAT_OFF"
  suffix="${variant}_hl"
else
  build_dir="../../GS/Build/${variant}"
  suffix="$variant"
fi

app_bin="$build_dir/x-cube-n6-ai-vad.bin"
wei_hex="$variant/network_data.hex"
app_bin_signed="x-cube-n6-ai-vad_signed.bin"
app_hex="x-cube-n6-ai-vad.hex"
fsbl_hex="../../../FSBL/ai_fsbl.hex"
combined_hex="app_vad_${suffix}.hex"

for f in "$app_bin" "$wei_hex" "$fsbl_hex"; do
  if [ ! -f "$f" ]; then
    echo "error: $f not found" >&2
    exit 1
  fi
done

# Sign the application as a secure-boot SSBL image (no key, header version 2.3),
# the format the FSBL expects
"$sign" -s -bin "$app_bin" -nk -t ssbl -hv 2.3 --align -o "$app_bin_signed"
# Convert it to Intel HEX at the external flash address the FSBL loads it from
arm-none-eabi-objcopy -I binary "$app_bin_signed" --change-addresses 0x70100000 -O ihex "$app_hex"

rm -f "$combined_hex"
# Generate combined HEX file from FSBL + signed app + matching weights.
# Intel HEX merge: strip EOF record (:00000001FF) from all but the last file.
grep -v "^:00000001FF" "$fsbl_hex" > "$combined_hex"
grep -v "^:00000001FF" "$app_hex" >> "$combined_hex"
cat "$wei_hex" >> "$combined_hex"

# Flash the combined image over SWD (hot-plug, access port 1) through the
# external flash loader
"$prog" -c port=swd mode=HOTPLUG ap=1 --extload "$el" -w "$combined_hex"
