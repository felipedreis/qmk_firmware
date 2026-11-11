# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Nature of this repo

A personal fork (`felipedreis/qmk_firmware`) of upstream QMK, used to maintain the owner's keyboard keymaps. Treat the firmware core (`quantum/`, `tmk_core/`, `platforms/`, `drivers/`, `lib/`) as vendored upstream: personal work is almost entirely keymap files under `keyboards/`. The current focus is the splitkb Aurora Sofle v2 (`keyboards/splitkb/aurora/sofle_v2/`, build target `splitkb/aurora/sofle_v2/rev1`).

Recent history is just keymap edits on top of upstream commits, so keep changes scoped to the keymap rather than touching core code.

## Commands

The `qmk` CLI is not installed in this environment (`which qmk` fails). Install via `python3 -m pip install qmk` / `brew install qmk/qmk/qmk`, then `qmk setup`/`qmk doctor`. Tool deps are in `requirements.txt` / `requirements-dev.txt`.

- Compile a keymap: `qmk compile -kb splitkb/aurora/sofle_v2/rev1 -km default` (or `make splitkb/aurora/sofle_v2/rev1:default`)
- Flash: `qmk flash -kb <keyboard> -km <keymap>` (or `make <kb>:<km>:flash`)
- Convert a Configurator JSON keymap to C: `qmk json2c <keymap.json>`
- Lint a keyboard/keymap: `qmk lint -kb <keyboard>`
- Format C: `qmk format-c -a`; format JSON: `qmk format-json`
- Unit tests (host-side, in `tests/`): `make test:all`, or a single suite `make test:tap_hold_configurations`, a single test `make test:<suite>:<TestName>` (e.g. `make test:basic`)
- Python CLI tests: `qmk pytest`

## Architecture notes

- **Keyboard definitions** live in `keyboards/<vendor>/<board>/[<rev>/]`. Config is layered and merged from parent dirs down to the revision dir: `info.json` (data-driven config, layouts, matrix, features), `config.h`, `rules.mk`, plus a `<board>.c/.h`. Settings in `info.json` are preferred over `config.h`/`rules.mk` for new work.
- **Keymaps** live in `<keyboard>/keymaps/<name>/`. A keymap is either `keymap.c` (+ optional `config.h`, `rules.mk`) or a Configurator-style `keymap.json` (compiled directly, and the owner's `sofle_v2/keymaps/default` is a `keymap.json`; its `"keyboard"` field must match the build target `.../rev1` and `"layout"` must name a layout defined in `info.json`).
- A parent keyboard dir such as `sofle_v2/` holds shared code; hardware variants (e.g. `rev1/`) inherit from it. Keymaps under the parent dir are found by any revision.
- **Core**: `quantum/` (features: combos, tap dance, RGB, OLED, process_keycode handlers), `tmk_core/` (protocol/USB), `platforms/` (AVR/ChibiOS/etc. HAL), `drivers/` (peripherals). Generic build logic is in `builddefs/` and the root `Makefile`.
- `data/` holds the JSON schemas and mappings (keycodes, constants) the CLI validates against; `users/` is for shared userspace code across keymaps; `layouts/` is for community layouts.
- Docs are in `docs/` (Docsify, published at docs.qmk.fm).
