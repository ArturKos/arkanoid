#!/usr/bin/env bash
# Record a hands-free Bricktron demo to a video file. Launches the game in its
# built-in attract mode (--demo, the paddle plays itself), captures the game
# window with ffmpeg/x11grab, then shuts the game down.
#
# Usage:
#   ./record-demo.sh [-d SECONDS] [-o OUTPUT] [--flatpak] [--no-audio]
#     -d SECONDS   recording length            (default 30)
#     -o OUTPUT    output video file           (default bricktron-demo.mp4)
#     --flatpak    run the installed Flatpak instead of build/bricktron
#     --no-audio   record video only
#
# Needs: ffmpeg (x11grab), xwininfo, wmctrl. Game audio is captured from the
# default PulseAudio sink's monitor (pactl); falls back to video-only if absent.
set -euo pipefail

DURATION=30
OUTPUT="bricktron-demo.mp4"
USE_FLATPAK=0
WITH_AUDIO=1
APP_ID="io.github.arturkos.Bricktron"
TITLE="Bricktron by Artur Kos"

while [ $# -gt 0 ]; do
  case "$1" in
    -d) DURATION="${2:?-d needs a value}"; shift 2 ;;
    -o) OUTPUT="${2:?-o needs a value}"; shift 2 ;;
    --flatpak) USE_FLATPAK=1; shift ;;
    --no-audio) WITH_AUDIO=0; shift ;;
    -h|--help) sed -n '2,13p' "$0"; exit 0 ;;
    *) echo "ERROR: unknown argument '$1'" >&2; exit 1 ;;
  esac
done

for tool in ffmpeg xwininfo wmctrl; do
  command -v "$tool" >/dev/null 2>&1 || { echo "ERROR: '$tool' not found." >&2; exit 1; }
done
[ -n "${DISPLAY:-}" ] || { echo "ERROR: no \$DISPLAY — need a running X session." >&2; exit 1; }

ROOT="$(cd "$(dirname "$0")" && pwd)"
# Resolve the output to an absolute path before any cd, so -o foo.mp4 lands in
# the directory the user ran the script from.
case "$OUTPUT" in
  /*) ;;
  *) OUTPUT="$PWD/$OUTPUT" ;;
esac

# --- 1. launch the game in attract mode --------------------------------------
if [ "$USE_FLATPAK" -eq 1 ]; then
  echo ">> Launching Flatpak $APP_ID --demo"
  flatpak run "$APP_ID" --demo &
else
  BIN="$ROOT/build/bricktron"
  [ -x "$BIN" ] || { echo "ERROR: $BIN missing — build it first (see README)." >&2; exit 1; }
  echo ">> Launching $BIN --demo"
  # Run from build/ so the assets resolve via the current directory.
  ( cd "$ROOT/build" && exec ./bricktron --demo ) &
fi
GAME_PID=$!

cleanup() { kill "$GAME_PID" 2>/dev/null || true; wait "$GAME_PID" 2>/dev/null || true; }
trap cleanup EXIT

# --- 2. wait for the window and read its geometry ----------------------------
echo ">> Waiting for the game window..."
INFO=""
for _ in $(seq 1 100); do
  if INFO="$(xwininfo -name "$TITLE" 2>/dev/null)"; then break; fi
  kill -0 "$GAME_PID" 2>/dev/null || { echo "ERROR: game exited before its window appeared." >&2; exit 1; }
  sleep 0.1
done
[ -n "$INFO" ] || { echo "ERROR: timed out waiting for the '$TITLE' window." >&2; exit 1; }

X=$(awk '/Absolute upper-left X/{print $4}' <<<"$INFO")
Y=$(awk '/Absolute upper-left Y/{print $4}' <<<"$INFO")
W=$(awk '/^  Width:/{print $2}' <<<"$INFO")
H=$(awk '/^  Height:/{print $2}' <<<"$INFO")
# libx264 + yuv420p needs even dimensions.
W=$(( W - W % 2 )); H=$(( H - H % 2 ))
echo ">> Window at ${X},${Y} size ${W}x${H}"

# Raise the window so nothing is captured on top of it, give it a beat to draw.
wmctrl -a "$TITLE" 2>/dev/null || true
sleep 0.5

# --- 3. pick the audio source (monitor of the default sink) ------------------
MONITOR=""
if [ "$WITH_AUDIO" -eq 1 ] && command -v pactl >/dev/null 2>&1; then
  SINK="$(pactl get-default-sink 2>/dev/null || true)"
  if [ -n "$SINK" ] && pactl list short sources 2>/dev/null | grep -q "^[0-9]*[[:space:]]${SINK}.monitor[[:space:]]"; then
    MONITOR="${SINK}.monitor"
    echo ">> Audio source: $MONITOR"
  else
    echo ">> WARNING: no monitor source for the default sink — recording video only." >&2
  fi
elif [ "$WITH_AUDIO" -eq 1 ]; then
  echo ">> WARNING: pactl not found — recording video only." >&2
fi

# --- 4. record ----------------------------------------------------------------
echo ">> Recording ${DURATION}s -> $OUTPUT"
FF=(ffmpeg -y -hide_banner -loglevel warning
    -thread_queue_size 1024
    -f x11grab -framerate 60 -video_size "${W}x${H}" -i "${DISPLAY}+${X},${Y}")
[ -n "$MONITOR" ] && FF+=(-thread_queue_size 1024 -f pulse -i "$MONITOR")
FF+=(-t "$DURATION" -c:v libx264 -preset veryfast -crf 18 -pix_fmt yuv420p)
[ -n "$MONITOR" ] && FF+=(-c:a aac -b:a 192k)
FF+=("$OUTPUT")
"${FF[@]}"

echo
echo ">> Done:"
ls -lh "$OUTPUT"
