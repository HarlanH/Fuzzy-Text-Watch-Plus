#!/usr/bin/env bash
# Regenerate README / store screenshots with stable, descriptive filenames.
# Requires: pebble build (default PBW: build/Almost-Five-Watchface.pbw), Node for emu:apply-config.
#
# Filenames encode scenario:
#   aplite-en-almost9-bat   — aplite, English, ~08:58 ("almost nine"), low battery
#   basalt-en-905           — basalt, English, 09:05
#   diorite-en-940-nobt     — English, 09:40, Bluetooth disconnected (intended for Pebble Time Steel)
#   flint-jp-1005           — flint (round), Japanese, 10:05
#
# Aplite: Back x4 after install to leave the timeline idle screen.
# Diorite slot: captured on basalt (same 144×168 color layout); see README.
#
# Full caveats: README.md → "Screenshot limitations".
# Override minimum PNG size if your valid frames are smaller: MIN_SCREENSHOT_BYTES=700 npm run screenshots:store
#
# pebble install --emulator … can sit in _wait_for_qemu() for a long time (cold QEMU boot). That is the
# SDK waiting for the emulator process, not this script freezing. Optional: PEBBLE_INSTALL_TIMEOUT=600
# (seconds) to abort and retry manually; or run `pebble kill --force` first if a previous qemu is stuck.

set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
PBW="${PBW:-build/Almost-Five-Watchface.pbw}"

# Tiny PNGs (~600–800 B) are almost always a wedged framebuffer (e.g. "Ping" garbage), not the face.
MIN_SCREENSHOT_BYTES="${MIN_SCREENSHOT_BYTES:-900}"
# 0 or unset = no limit (pebble may wait indefinitely in _wait_for_qemu).
PEBBLE_INSTALL_TIMEOUT="${PEBBLE_INSTALL_TIMEOUT:-0}"

if [[ ! -f "$PBW" ]]; then
  echo "Missing $PBW — run pebble build first." >&2
  exit 1
fi

hard_reset() {
  pebble kill --force 2>/dev/null || true
  # QEMU/pypkjs need time to release ports; rushing causes the next install to fail.
  sleep 8
}

aplite_return_to_watchface() {
  local i
  for i in 1 2 3 4; do
    pebble emu-button click back --emulator aplite 2>/dev/null || true
    sleep 0.7
  done
}

apply_config_once() {
  local emu="$1" cfg="$2"
  npm run emu:apply-config -- --emulator "$emu" --file "$cfg"
}

screenshot_bytes() {
  wc -c <"$1" | tr -d ' '
}

capture_shot() {
  local emu="$1" outfile="$2"
  if ! pebble screenshot "$outfile" --no-open --emulator "$emu"; then
    return 1
  fi
  local sz
  sz=$(screenshot_bytes "$outfile")
  if [[ "$sz" -lt "$MIN_SCREENSHOT_BYTES" ]]; then
    echo "Screenshot too small (${sz} B < ${MIN_SCREENSHOT_BYTES} B); likely invalid framebuffer." >&2
    return 1
  fi
  return 0
}

# Wraps pebble install; optional wall-clock timeout around the whole install (including _wait_for_qemu).
pebble_install() {
  local emu="$1"
  echo "pebble install --emulator $emu …" >&2
  echo "  (SDK may wait in _wait_for_qemu until QEMU is ready — first launch is often 1–5+ minutes; Activity Monitor should show qemu-system CPU usage.)" >&2
  if [[ "$PEBBLE_INSTALL_TIMEOUT" =~ ^[0-9]+$ ]] && [[ "$PEBBLE_INSTALL_TIMEOUT" -gt 0 ]]; then
    python3 - "$PEBBLE_INSTALL_TIMEOUT" "$emu" "$PBW" <<'PY'
import subprocess, sys
timeout = int(sys.argv[1])
emu, pbw = sys.argv[2], sys.argv[3]
try:
    subprocess.run(["pebble", "install", "--emulator", emu, pbw], check=True, timeout=timeout)
except subprocess.TimeoutExpired:
    print(
        "pebble install exceeded timeout while waiting for QEMU (_wait_for_qemu).\n"
        "Try: pebble kill --force\n"
        "Then re-run, or raise PEBBLE_INSTALL_TIMEOUT, or kill stuck qemu-system-* in Activity Monitor.",
        file=sys.stderr,
    )
    sys.exit(124)
except subprocess.CalledProcessError as e:
    sys.exit(e.returncode)
PY
  else
    pebble install --emulator "$emu" "$PBW"
  fi
}

run_one() {
  local emu="$1" time="$2" outfile="$3" cfg="$4" bat="$5" bt="$6"
  local attempt shot_ok=0

  for attempt in 1 2 3; do
    echo "=== $outfile (attempt $attempt) ===" >&2
    hard_reset

    if ! pebble_install "$emu"; then
      echo "pebble install failed for $emu" >&2
      continue
    fi

    # Do not use pebble ping to gate readiness — it can return Pong before the watchface paints,
    # which produces bogus tiny PNGs. Wait for the app and JS to come up instead.
    sleep 28


    # One repl session only — repeated pebble repl often wedges QEMU so the *next* install fails.
    #if ! apply_config_once "$emu" "$cfg"; then
      #echo "Warning: emu:apply-config failed (weather/language may be default). Continuing to screenshot." >&2
      #sleep 6
    #else
      #sleep 6
    #fi

    pebble emu-battery --percent "$bat" --emulator "$emu"
    pebble emu-bt-connection --connected "$bt" --emulator "$emu"
    pebble emu-set-time "$time" --emulator "$emu"
    sleep 4

    if capture_shot "$emu" "$outfile"; then
      shot_ok=1
      echo "Wrote $outfile ($(screenshot_bytes "$outfile") B)" >&2
      break
    fi

    echo "Retrying after hard reset…" >&2
  done

  hard_reset

  if [[ "$shot_ok" -ne 1 ]]; then
    echo "Failed to capture $outfile after 3 attempts." >&2
    return 1
  fi
  return 0
}

mkdir -p screenshots

fail=0
#run_one basalt 09:05:00 screenshots/basalt-en-905.png screenshots/capture-config-en.yml 100 yes || fail=1
#run_one basalt 09:40:00 screenshots/diorite-en-940-nobt.png screenshots/capture-config-en.yml 100 no || fail=1
#run_one flint 10:05:00 screenshots/flint-jp-1005.png screenshots/capture-config-ja.yml 100 yes || fail=1
#run_one aplite 08:58:00 screenshots/aplite-en-almost9-bat.png screenshots/capture-config-en.yml 35 yes || fail=1
run_one diorite 04:58:00 screenshots/diorite-en-almost5.png screenshots/capture-config-en.yml 100 yes || fail=1

if [[ "$fail" -ne 0 ]]; then
  echo "One or more screenshots failed. Fix emulator/SDK state or capture manually; see README." >&2
  exit 1
fi

echo "All screenshot steps finished."
