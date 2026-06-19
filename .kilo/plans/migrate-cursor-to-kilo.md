# Plan: Migrate .cursor/ Configuration to Kilo

## Current State

| Path | Purpose |
|---|---|
| `.cursor/Dockerfile` | Cursor Cloud dev environment image (Node 18, Python 3.12, uv, pebble-tool, emulator libs) |
| `.cursor/environment.json` | Cursor Cloud build/install script (`npm ci && pebble sdk install latest`) |
| `.cursor/skills/pebble-cli-closed-loop/SKILL.md` | Skill: closed-loop Pebble emulator development with screenshots |
| `AGENTS.md` | Project-level AI instructions (title says "Cursor Cloud specific instructions" but content is tool-agnostic) |

No `.kilo/` directory or `kilo.jsonc` exists at the project level yet.

## Changes

### 1. Create `.kilo/skills/pebble-cli-closed-loop/SKILL.md`

Move the skill verbatim from `.cursor/skills/pebble-cli-closed-loop/SKILL.md`. Both Cursor and Kilo follow the [Agent Skills](https://agentskills.io) spec — the SKILL.md format is identical. The content is already tool-agnostic (describes `pebble` CLI commands, not Cursor features).

### 2. Update `AGENTS.md` heading

Change `## Cursor Cloud specific instructions` to `## Project setup`. The rest of the content (services table, key commands, gotchas) is already tool-agnostic and appropriate for any AI agent.

### 3. Delete `.cursor/` directory

After migration, remove the entire `.cursor/` folder:
- `Dockerfile` — Cursor Cloud infrastructure; Kilo runs locally or in its own cloud. If this Dockerfile is needed for other purposes (CI, team dev setups), move it to `docs/dev-environment/` first. Recommend removing since the `pebble` ecosystem setup is already documented in AGENTS.md.
- `environment.json` — Purely Cursor Cloud, no equivalent or value.
- `skills/` — Already migrated to `.kilo/skills/`.

### 4. Create `kilo.jsonc` (project-level)

Minimal project config to wire up the skill path explicitly (optional — `.kilo/skills/` is auto-scanned, but being explicit is clearer):

```jsonc
{
  "$schema": "https://kilo.ai/config.json"
}
```

This is intentionally empty beyond the schema declaration. The skill auto-discovery from `.kilo/skills/` handles loading. A project `kilo.jsonc` can be expanded later if project-level `instructions` or permission overrides are needed.

## What Does NOT Migrate

| Cursor feature | Reason |
|---|---|
| `Dockerfile` / `environment.json` | Kilo has no cloud dev environment concept. Setup is local. |
| Cursor-specific skill loading | Not needed — Kilo auto-scans `.kilo/skills/` |

## Files Touched

| Action | File |
|---|---|
| Create | `.kilo/skills/pebble-cli-closed-loop/SKILL.md` |
| Create | `kilo.jsonc` |
| Edit | `AGENTS.md` (heading only) |
| Delete | `.cursor/` (entire directory) |
