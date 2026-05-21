Almost Five
===========

## Overview

This is a watch face for the [Pebble smartwatch](http://www.getpebble.com).
It is based on the [PebbleTextWatch](https://github.com/wearewip/PebbleTextWatch) by waerewip,
and [Fuzzy-Text-Watch-Plus](https://github.com/Sarastro72/Fuzzy-Text-Watch-Plus) by Sarastro72.

Compared to Fuzzy-Text-Watch-Plus, this fork adds day-of-month display, upcoming-meeting flags
and Bluetooth and battery indicators, additional languages, fixed backlight on gestures,
on-watch weather phrasing (Open-Meteo), and other small changes.

**Current app version:** 1.2.0 (see `package.json` and `RELEASE_NOTES.md`).


## For users

### Features

- Fuzzy time in natural language with 5 minute precision.
- Current languages: Swedish, English (US, default), English (UK), Norwegian, Dutch, Italian, Spanish, German (east), German (west), French, Japanese (romaji)
- The large and easy to read fonts of the original Text Watch
- Buzz and message when bluetooth connection is lost
- Top status row: meeting alerts, Bluetooth disconnect, low battery (below 40%), and **localized weather** (sky + spelled temperature band); on supported color watches you can **tap** to cycle what is highlighted when multiple items are active
- Localized strings for meeting alerts, **spelled day-of-month** (no day digits), and **low-battery** label
- **Weather** (companion): fetches Open-Meteo; watch shows a short phrase (for example clear / partly cloudy + “high teens”, “twenties”, etc.) with **no numeric temperature** in the line
- Configurable colors
- Supports all pebble models up to and including Pebble Time Round
- Configurable time offset for when each fuzzy phrase changes
- Gesture-triggered backlight pulse
- Bottom **spelled** day-of-month text (per language, from `strings/ui.yaml` on the watch)
- Nice staggered animation
- Between one and four lines of text, depending on need.
- The text rows are always centered vertically
- Smaller words may share a single line (such as "ten to")
- Refactored code for better flexibility/rewriteability

### Getting the watchface

This watchface is available on the Pebble App store as (pending).

### Screenshots

Filenames describe the scenario (platform, language, time of day, flags). Regenerate with **`npm run screenshots:store`** after `pebble build` (uses `screenshots/capture-config-*.yml` and `pebble emu-button` on aplite so the watchface is visible, not the timeline idle screen).

| Platform | Preview |
|---|---|
| Aplite (English, ~08:58 “almost nine”, low battery) | ![Aplite screenshot](screenshots/aplite-en-almost9-bat.png) |
| Basalt (English, 09:05) | ![Basalt screenshot](screenshots/basalt-en-905.png) |
| Diorite (English, 09:40, Bluetooth disconnected) | ![Diorite screenshot](screenshots/diorite-en-940-nobt.png) |
| Flint (Japanese, 10:05) | ![Flint screenshot](screenshots/flint-jp-1005.png) |

The **`diorite-en-940-nobt`** frame is recorded on the **basalt** emulator: both Pebble Time and Time Steel use the same 144×168 color layout, and some SDK installs cannot complete `pebble screenshot` against the diorite QEMU target even though the PNG is labeled for the Steel listing.

#### Screenshot limitations (what the CLI often cannot do reliably)

Automation and emulator bugs matter as much as the watchface:

- **Diorite (Time Steel) QEMU:** On many hosts, `pebble screenshot` and `pebble repl` never finish the initial `WatchVersion` handshake and raise `TimeoutError`. There is no fix in this repo—use the **basalt** emulator for the same 144×168 color framebuffer, or capture on a **real Time Steel** over the phone/dev bridge.
- **Aplite (OG Pebble) after `pebble install`:** The emulator may stay on the system “install an app / timeline” text even though the install succeeded. The app is not broken; send **Back** a few times (as `scripts/capture-store-screenshots.sh` does) until the watchface is visible, then shoot.
- **QEMU never becomes ready:** If `pebble install` spins in `_wait_for_qemu` past ~10 minutes with **no** `qemu-system` CPU use, something is wrong with the SDK emulator on that host (stale lock, missing deps). **`pebble kill --force`**, reboot, or capture on a **phone** instead.
- **Stale or wrong framebuffer:** If QEMU was left in a bad state or the wrong platform was last connected, `pebble screenshot` can produce a tiny PNG or nonsense (for example a single word). Run **`pebble kill --force`**, reinstall the face, wait for it to draw, then capture. **`npm run screenshots:store`** waits a long fixed delay after install (instead of trusting `pebble ping`), rejects shots under ~900 bytes, retries up to three times, uses **one** `emu:apply-config` per shot, and **`pebble kill --force`** with an 8s sleep after every capture so the next `pebble install` is not racing a wedged QEMU.
- **`pebble repl` / `emu:apply-config`:** Repeated Python repl sessions can wedge libpebble until the emulator is killed. The store script only opens repl once per frame. If tuple push fails, kill, reinstall, and retry once; worst case set options on a **phone** and screenshot there.
- **Platforms not in the store script:** Chalk, Emery, Gabbro, etc. are not automated here; add manual steps or extend the script if you need those store slots.
- **States that need a real watch:** Quiet-mode “SHH”, exact meeting banners, or carrier-specific notification stacks are easier to validate on **hardware** than in QEMU.

### Settings

Open this watch face’s settings in the Pebble / Rebble app to change appearance, language, time offset, backlight-on-gesture behavior, Bluetooth disconnect alerts, and Google Calendar iCal URLs.

The companion app uses an embedded `data:` URL for the configuration page, so no external hosted config site is required.

### Calendar and meeting status

- Add up to three Google Calendar iCal URLs in settings.
- Events with `TRANSP:TRANSPARENT` or `X-MICROSOFT-CDO-BUSYSTATUS:FREE` are ignored.
- Meeting status is shown in the top row (`meeting soon` / `meeting now`) and localized with the active watch language.
- Calendar polling is aligned to real 5-minute boundaries (`:00`, `:05`, `:10`, etc.) with minute boundary status checks to avoid delayed state transitions.

### Possible future features

- [ ] Japanese in kana/kanji: system fonts lack CJK glyphs; bundle a subset font and load it for the text layers that show time strings.
- [ ] More languages, better translations
- [ ] Configurable and improved text alignment
- [ ] Better support for round watch designs
- [ ] Test suite with screenshot review
- [ ] handle timeline notifications https://developer.repebble.com/tutorials/watchface-tutorial/part5/

## For developers

### Prerequisites

- Rebble/Pebble SDK tooling with the `pebble` CLI available in your shell.
- Node.js 18+ if you want to run PKJS regression tests (`npm test`).

### Build and install (examples)

- `pebble build` — compile the app; the bundle is written under `build/` (for example `build/Almost-Five.pbw`).
- `pebble install --phone 192.168.0.218` — install to a watch on the LAN; replace the IP with your phone’s address (Pebble app must allow developer connection).
- `pebble install --emulator flint` — install to a running emulator; replace `flint` with your target (for example `basalt`, `chalk`, `aplite`).

To remove build artifacts, run `pebble clean` or delete the `build/` directory.

### String codegen (weather + UI)

- **Weather:** edit `strings/weather.yaml`, then `npm run codegen:weather` (writes `src/generated/weather_i18n_gen.{h,c}`).
- **Day ordinals + low-battery label:** edit `strings/ui.yaml`, then `npm run codegen:ui` (writes `src/generated/ui_i18n_gen.{h,c}`).

See `AGENTS.md` for a concise command reference.

### Local emulator config (YAML, no config webview)

The emulator config webview callback can be unreliable on some platforms. For local iteration, this repo includes a YAML-driven path that sends settings directly as AppMessage tuples (bypassing HTML/webview callbacks).

1. Edit `dev-config.yml` with the values you want (for example `language`, `offset`, `gesture`, `bt_notification`, `strict_hour_phrases`). Optionally add **`weather_code`** and **`weather_temp_f`** (integers) to mimic PKJS weather tuples for screenshots or layout tests.
2. Install/run the watchface on your emulator target.
3. Apply settings:
   - `npm run emu:apply-config -- --emulator flint`

This command reads `dev-config.yml`, maps values to the app's message keys from `package.json`, and sends them directly to the running watch app over the Pebble transport.

Current limitation: this direct local config flow is not yet reliable on every supported emulator platform. If one platform fails to connect or apply, try another target (for example `flint`) for settings iteration, or use phone-side config for final validation.

### Store / README screenshots

Run **`npm run screenshots:store`** from the project root (after `pebble build`). That script installs to each emulator, applies `screenshots/capture-config-en.yml` or `capture-config-ja.yml`, on **aplite** sends several **Back** button presses so the face is shown instead of the system “install an app / timeline” prompt, and captures the diorite-named PNG using **basalt** (same color resolution as Time Steel).

**`pebble install` looks stuck:** The SDK may sit in `_wait_for_qemu()` with no new output for many minutes on a **cold** QEMU start—that is normal. Check Activity Monitor for `qemu-system-*` using CPU; give it time before Ctrl-C. If it never finishes, run **`pebble kill --force`**, look for zombie QEMU, then retry. Optional: **`PEBBLE_INSTALL_TIMEOUT=600 npm run screenshots:store`** aborts each install after 600 seconds so you get a clear error instead of an infinite wait.

Remaining screenshot TODOs (known follow-ups):

- Add an optional hardware capture path for true diorite shots so the labeled Steel frame is not emulator-substituted.
- Add automated validation beyond file size (for example image-diff or OCR smoke checks) to catch wrong-frame captures.
- Decide whether to include additional store targets (for example chalk/emery/gabbro) in the scripted flow.

### Troubleshooting

- If `pebble build` fails because `pebble` is missing, install or re-link your Pebble/Rebble SDK tooling.
- If configuration fails to open, inspect PKJS logs from the phone/emulator runtime.
- If settings save but do not apply, inspect phone logs for PKJS AppMessage payloads and ensure message keys match `package.json`.
