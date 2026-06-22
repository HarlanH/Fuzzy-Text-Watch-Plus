# Almost Five release notes

## 1.3.0

### Appstore text (short)

- Day-of-week on bottom line alternates with day-of-month.
- Gestures more reliable, enabling backlight and swapping bottom line content.
- Reduced CPU, RAM, and flash usage for better battery life and performance.

### Detailed

- **Day-of-week toggle:** When gesture backlight is on, tapping the watch now toggles the day name (e.g. "Monday") on the bottom line alongside the spelled day-of-month.
- **Gesture config simplified:** The backlight-on-gesture setting is now a simple on/off toggle (previously had multiple options). On enables both the backlight pulse and the day-of-week tap toggle.
- **Gesture debounce:** Added debounce to prevent rapid repeated tap events from toggling the display multiple times.
- **Performance:** Reduced tick frequency and added deduplication to lower CPU, RAM, and flash usage.

## 1.2.0

### Appstore text (short)

- On-watch weather line from Open-Meteo: sky condition and spelled-out temperature bands (no raw °F/°C digits), localized per language.
- Day-of-month is spelled out in words for every locale (YAML-driven), with low-battery status text localized too.
- Top status row rotates meeting, Bluetooth, battery, and weather; optional gesture tap cycles the row on color platforms.
- Noon and midnight show dedicated phrases in every supported language (not “twelve o'clock”).
- Time shows again immediately when you return to the watchface—no startup greeting splash.

### Detailed

- **Weather:** PKJS fetches Open-Meteo; watch receives `KEY_WEATHER_CODE` + `KEY_WEATHER_TEMP_F` (always °F on the wire). Copy comes from `strings/weather.yaml` via `npm run codegen:weather` into `weather_i18n_gen.{h,c}`; non–US English locales use °C bands in prose.
- **UI i18n:** `strings/ui.yaml` + `npm run codegen:ui` generate ordinals (1st–31st) and `bat_low` labels; `day_prefix` in YAML keeps aplite within flash limits.
- **Layout / interaction:** Hybrid top-row phrase rotation and gesture paging on color platforms; status row stays visible on load.
- **Time phrases:** Per-locale exceptions at 12:00 and 00:00 (e.g. noon/midnight, midi/minuit, hyphenated forms on narrow layouts). Hour words in “til/to” and similar phrases keep bold markup (`*$2`) after substitution—fixes cases like “twenty til five” rendering the hour in the wrong font.
- **Startup:** Removed greeting splash and companion “Greeting display time” setting (`KEY_MESSAGE_TIME`); watchface shows fuzzy time on load. Connection-lost overlay unchanged.
- **Emulator tooling:** `dev-config.yml` / `npm run emu:apply-config` may include optional `weather_code` and `weather_temp_f` for reproducible screenshots without the JS weather fetch.

### Screenshot automation status / remaining TODOs

- Added `npm run screenshots:store` with scenario-driven configs under `screenshots/capture-config-*.yml`.
- Aplite timeline-idle handling and screenshot retry/size guards are in place to reduce bad captures.
- Remaining follow-ups: true hardware diorite capture option, stronger screenshot-content validation, and optional automation for non-store platforms.

## 1.1.0

### Appstore text (short)

- Added a YAML-based local emulator config workflow for development (`dev-config.yml` + `npm run emu:apply-config`).
- The emulator config helper sends AppMessage tuples directly, bypassing flaky config-webview callbacks.
- Updated project docs and development guidance for the local emulator config flow.

### Detailed

- Added direct YAML-driven emulator config injection tooling:
  - `scripts/emu-apply-config.js`
  - `dev-config.yml`
  - test coverage for config mapping and tuple payload generation
- Added `yaml` dependency and npm script integration for local config workflows.
- Updated README with:
  - local emulator config instructions
  - screenshot gallery updates
- Removed legacy, unused hosted-config assets under `config/`.

## 1.0.0

first release
