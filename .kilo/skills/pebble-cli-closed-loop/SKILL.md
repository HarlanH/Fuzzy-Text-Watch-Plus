---
name: pebble-cli-closed-loop
description: Use when developing Pebble or Rebble watch apps or watchfaces with QEMU or the SDK emulator, verifying UI visually, or automating screenshots and emulator input (pebble screenshot, emu-*, logs).
---

# Pebble CLI closed-loop development

## When to apply

Use this skill whenever the task involves **driving the emulator or a connected watch** and **confirming the UI**—for example after layout or color changes, fuzzy-time logic, or PKJS behavior—without relying on pasted screen grabs.

**Prefer the CLI** over asking the user for screenshots when `pebble` is available and a target is reachable.

## Assumptions

- Rebble/Pebble SDK is installed; `pebble` is on `PATH`.
- Work from the **app project root** (where `package.json` / `appinfo.json` lives) for `pebble build`, `pebble install`, and consistent paths.

## Closed-loop workflow

1. **`pebble build`** — compile; fix errors until it succeeds.
2. **`pebble install`** — deploy to the target (see [Choose a target](#choose-a-target)).
3. **Drive state** — time, buttons, battery, Bluetooth, etc. (see [Emulator and watch control](#emulator-and-watch-control)) so the watchface shows the scenario under test.
4. **`pebble screenshot <path>.png --no-open`** — capture the framebuffer; inspect the image or attach it in the reply.
5. Optionally **`pebble logs`** (separate process or follow-up) for native or JS runtime output.

Repeat 2–5 until the screenshot and logs match expectations.

## Choose a target

| Situation | Typical invocation |
|-----------|-------------------|
| SDK launches / manages emulator | `pebble install --emulator <platform>` — platforms include `aplite`, `basalt`, `chalk`, `diorite`, `emery`, `flint`, `gabbro`. |
| QEMU already running | Pass **`--qemu [host]`** to commands that support it (see `pebble <cmd> --help`). Match **`--platform`** when the help requires it (e.g. with `--pypkjs`). |
| Phone / dev connection | `pebble install --phone <ip>` or env-based phone connection per SDK docs. |

If unsure what is connected, try **`pebble screenshot /tmp/pebble-check.png --no-open`** from the project root; if it succeeds, the CLI already sees a device.

## Screenshots

```bash
pebble screenshot build/emu.png --no-open
```

- **`--no-open`** — do not open the viewer; use for automation and agent-driven runs.
- **`--emulator <platform>`** — can launch/use the SDK emulator when applicable.
- **`--qemu [host]`** — attach to a QEMU instance when not using the default connection.
- Batch: **`--all-platforms`** or **`--gif-all-platforms`** for multi-platform captures (see `--help`).

Save artifacts under **`build/`** or **`/tmp/`** so they are easy to find and `.gitignore` can exclude them if needed.

## Emulator and watch control

Run **`pebble <command> --help`** before relying on rare flags. Commonly useful:

| Goal | Command |
|------|---------|
| Interactive sensor / control UI | `pebble emu-control` (optional `--port`) |
| Hardware buttons | `pebble emu-button click|push|release back|up|select|down` |
| Watch time | `pebble emu-set-time <HH:MM:SS or Unix UTC>` |
| 12h / 24h | `pebble emu-time-format` |
| Battery / charging | `pebble emu-battery` |
| Bluetooth state | `pebble emu-bt-connection` |
| Tap | `pebble emu-tap` |
| Accelerometer | `pebble emu-accel` |
| Compass | `pebble emu-compass` |
| App config page | `pebble emu-app-config` |

Many of these accept **`--emulator`**, **`--qemu`**, and **`--platform`** in the same way as `screenshot` / `install`; align flags with how the session is actually running.

## Agent behavior

- **Execute** `pebble` commands in the terminal rather than instructing the user to run them, unless the environment lacks the SDK or connection fails after reasonable retries.
- After a screenshot, **read the image file** when verifying layout, colors, or text—do not assume success from CLI exit code alone.
- If capture fails, check connection (emulator running, correct `--qemu` / `--emulator`), then **`pebble ping`** or reinstall.

## What not to do

- Do not suggest pasting QEMU window screenshots as the primary verification path if **`pebble screenshot`** can reach the same device.
- Do not omit **`--no-open`** in scripted or agent-driven screenshot runs unless the user explicitly wants the image opened locally.
