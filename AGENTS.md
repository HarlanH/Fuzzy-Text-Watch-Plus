## Project setup

### Overview

This is a Pebble/Rebble smartwatch watchface ("Almost Five") that shows fuzzy natural-language time. It is a native C app with a JS companion (PebbleKit JS). There are no backend services, databases, or Docker containers.

Product direction: avoid raw digits in user-facing copy where practical (fuzzy time, weather line uses spelled-out temperature bands, not numeric °F/°C).

Weather copy is **on-watch** from `strings/weather.yaml` via **`npm run codegen:weather`** (writes `src/generated/weather_i18n_gen.{h,c}`). PKJS sends Open-Meteo **`KEY_WEATHER_CODE`** + **`KEY_WEATHER_TEMP_F`** (°F only); the watch converts to °C for non-`LANG_EN` locales. Future tweaks and deferred ideas: `docs/weather-i18n-notes.md`.

**Date ordinals + low-battery label** come from `strings/ui.yaml` via **`npm run codegen:ui`** (`src/generated/ui_i18n_gen.{h,c}`). **Fuzzy-time phrases** remain in `lang-*.c`; optional full YAML migration is described in `docs/lang-codegen-future.md`.

### Services

| Service | Purpose | How to run |
|---|---|---|
| JS tests | PKJS + emulator config tool unit tests | `npm test` |
| Weather strings codegen | Regenerate C from `strings/weather.yaml` | `npm run codegen:weather` |
| UI strings codegen | Regenerate C from `strings/ui.yaml` (ordinals, `bat_low`) | `npm run codegen:ui` |
| Store screenshots | Regenerate `screenshots/*.png` for README (aplite uses Back to leave idle screen) | `pebble build` then `npm run screenshots:store` (first QEMU boot may sit in `_wait_for_qemu` several minutes; `PEBBLE_INSTALL_TIMEOUT=600` optional) |
| Pebble build | Compile watchface for all target platforms | `pebble build` |
| Pebble emulator | Run the watchface visually | `pebble install --emulator <platform>` (e.g. `basalt`, `flint`, `aplite`, `diorite`) |

### Key commands

- **Build:** `pebble build` — compiles for aplite, basalt, diorite, flint; output in `build/`
- **Clean:** `pebble clean`
- **Tests:** `npm test` (runs `node --test test/**/*.test.js`)
- **Install to emulator:** `pebble install --emulator basalt` (or `flint`, `aplite`, `diorite`)
- **Screenshot:** `pebble screenshot build/emu.png --no-open --emulator basalt`
- **Set emulator time:** `pebble emu-set-time HH:MM:SS --emulator basalt`
- **Apply dev config:** `npm run emu:apply-config -- --emulator flint`
- **Weather i18n codegen:** `npm run codegen:weather` (after editing `strings/weather.yaml`)
- **UI i18n codegen:** `npm run codegen:ui` (after editing `strings/ui.yaml`)

### Gotchas

- **Emulator screenshots:** Diorite QEMU often fails `pebble screenshot` / `pebble repl` with `TimeoutError` on some machines; the repo’s `npm run screenshots:store` uses basalt for the diorite-named PNG (same 144×168 layout). Aplite may show the timeline idle screen until Back is pressed. See README → *Screenshot limitations*.
- The `pebble` CLI emits Python `SyntaxWarning` lines about invalid escape sequences in `libpebble2`; these are harmless and do not affect functionality.
- The ARM linker warns about `LOAD segment with RWX permissions` on every platform — this is expected and does not affect the watchface.
- `emu-set-time` may not visually update the watchface display immediately if the emulator app was just installed; the fuzzy-time text updates on the next minute tick.
- The emulator config webview path (`emu-app-config`) can be unreliable; prefer `npm run emu:apply-config -- --emulator <platform>` for local settings iteration (see README).
- `pebble-tool` is installed via `uv tool install pebble-tool --python 3.12`. The SDK itself is installed separately via `pebble sdk install latest` and lives in `~/.pebble-sdk/`.
- System dependencies required for the emulator: `libsdl1.2debian`, `libfdt1`, `libpixman-1-0`, `libsdl2-2.0-0`.
- The Pebble SDK (~450MB) is installed by the update script (`pebble sdk install latest`), not baked into the Dockerfile, to keep the Docker image small enough for reliable pulls. It takes ~30s on first run each session.
