# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repository is

This is **reverse-compiled C source code** for the original 1987 Atari ST game *MIDImaze* (Hybrid Arts). The C in `src/` was produced by hand-converting the original 68000 assembly back to ANSI C using the Python tooling in `InverseCompiler/`. Comments, function names, and variable names are the author's; the **control flow is intentionally identical to the original binary** so the game can (in principle) be re-linked to produce the original `MIDIMAZE.PRG`. Do not refactor for style — preserve the original structure.

The shared game sources live in `src/`. They are built for two targets:

- **Atari ST / MiNT** via `gui/tos/Makefile` (cross-compiler `m68k-atari-mint-gcc`). There is also a legacy top-level `Makefile` that builds a similar `midimaze.prg`.
- **macOS** via `MIDIMaze.xcodeproj` (Xcode). The same `src/*.c` files are compiled with platform glue in `MIDIMaze/` and TOS/BIOS/AES/VDI stubs in `MIDIMaze/Helper Sources/AtariTOS.mm`.

## Common commands

### Build the macOS app
Open `MIDIMaze.xcodeproj` in Xcode and build, or:
```
xcodebuild -project MIDIMaze.xcodeproj -scheme MIDIMaze
```
The macOS build runs without MIDI; control with cursor keys, fire with `0`, `Esc` quits, `Return` starts a game.

### Build the Atari ST binary (cross-compile)
Requires the `m68k-atari-mint-gcc` toolchain.
```
cd gui/tos && make           # produces midimaze.prg
cd gui/tos && make DEBUG=1   # debug build (no optimization, debug symbols, NatFeats logging)
cd gui/tos && make clean
```
The older top-level `Makefile` exists for reference but `gui/tos/` is the current TOS build.

There is no test suite. There are no linters configured beyond `-Werror -Wall -Wundef -Wstrict-prototypes -Wmissing-prototypes` in the TOS Makefile.

### Helper Python scripts (repo root)
- `read_d8a.py` — decode `MIDIMAZE.D8A` art assets (sprites, title screen, palette).
- `read_MZE.py` — decode the MIDI-Maze 2 `.MZE` maze format.

## Architecture

### Coordinate / direction conventions (read this before touching game logic)
- All math is **16-bit integer**, no floats. `sin`/`cos` come from a 65-word table in `fastmath.c`; `rotate2d()` is the only exported rotation primitive.
- Positions are `(Y, X)` (Y first), origin top-left. A maze cell is `MAZE_CELL_SIZE = 256` units; max maze is 64x64 cells. `MAZE_FIELD_SHIFT = 7` (= `MAZE_CELL_SIZE/2`) turns a coordinate into a "field index"; OR-ing 1 lands you on the odd-coordinate ".\" cell between four potential walls — this is how the game checks for walls in the 4 compass directions.
- Directions are **0..255 (256 units = 360°)**, 0 = north, clockwise. Overflow handled by masking the low 8 bits. The drone code occasionally uses 256 as a sentinel that means "north" (because 0 is overloaded as "no plan").
- Walls have **zero thickness** even though the maze file looks like they don't.

### Player and object storage
- All players live in `player_data[PLAYER_MAX_COUNT]` (16). Order is: humans first (index 0 = master), then drones (Target → Standard → Ninja), then MIDIcams. `own_number` is the index of *this* machine's player; MIDIcam mode mutates `own_number` to follow other players.
- Fields prefixed `ply_` are universal; `dr_` are drone-only. `dr_dir[6]` / `dr_field[6]` are used **only by the Ninja** for multi-step path plans.
- Each maze cell holds the head of a linked list of objects: indices `0..15` are players (chain through `.ply_plist`), indices `16..31` are shots (chain through `.ply_slist`, one shot per player). `-1` terminates. See `maze_obj.c`.

### Networking determinism
MIDImaze runs 16-player MIDI-ring multiplayer by sending **only joystick states** between machines each frame and re-simulating identically on every node. Two things make this work:
1. `_rnd()` in `rnd.c` uses a shared seed broadcast by the master before the game. Every machine generates the same random sequence (respawn positions, etc.). The Atari XBIOS `Random()` is only used by code that *must not* affect the shared simulation (e.g. MIDIcam picking which player to watch).
2. `move_player()` rotates which player is processed first each frame so low-numbered players don't always get the shot-first advantage.

If you change anything in `gamelogic.c`, `rnd.c`, or the joystick path, you risk desyncing networked play.

### Main loop layout
`main()` → `setup_game()` (resource load, copy-protection screen) → `dispatch()` (mode switcher) → one of:
- `master_solo_loop()` — Master in a MIDI ring, or Solo (`master.c`)
- `slave_midicam_loop()` — Slave or MIDIcam (`slave.c`)

All four modes ultimately call `game_loop()` (`maingame.c`), which per frame: handles hit-flash, draws 2D/3D/death-screen, reads local input into `player_joy_table[]`, **MIDI-syncs joystick data**, runs drones, then `move_player()` for each player. `gamelogic.c` (`move_player` / `move_shoot`) is the authoritative game-state mutator.

### Drones (computer players)
`drone.c` is huge (~4800 lines) because **the vast majority is the Ninja**. Three types, defined in `globals.h`:
- `DRONE_TARGET` ('r') — wanders, never fires.
- `DRONE_STANDARD` ('l') — fires only when target is on the same row/column within ~3.5 fields.
- `DRONE_NINJA` ('k') — same firing, plus path planning (`drone_sub_ninja_plan` and the `_north/_south/_east/_west` variants populate `dr_dir[]`/`dr_field[]` with up to 6 waypoints), plus stuck-detection timeouts. Ninjas "cheat" by knowing the target's exact coordinates.

Drones never attack other drones. `drone_setup()` assigns each drone a permanent human target before the game; `drone_action()` runs each frame and writes synthesized joystick data into `player_joy_table[]`, so the rest of the game treats drones identically to humans.

For a deeper walk-through see [`README/MIDImazeEngineOverview.md`](README/MIDImazeEngineOverview.md).

### 3D rendering pipeline
Two phases: `make_draw_list()` builds a list of visible walls/players/shots from the current viewer position; `draw_list()` (in `draw3d.c`) renders it with the perspective math. Walls are added via `drawwall.c` (heavily optimized — only visible spans, clipped to minimal width). Player and shot sprites share the same shape system in `drawshape.c`; a shot is just a player sprite drawn from behind (no face). All blitting/line-drawing has separate b/w (`blit_bw.c`, `draw_bw.c`) and color (`blit_col.c`, `draw_col.c`) paths — these were originally hand-written 68k assembly, kept as comments above the C.

### Build-time toggles (`src/globals.h`)
Switch behavior between "exactly the original" and "modern, fixed":
- `BUGFIX_MIDI_TIMEOUT` — fixes the infamous MIDI-ring timeout (root cause in `lowmem.c`).
- `BUGFIX_DIVISION_BY_ZERO` — div-by-zero guard for horizontal-wall rendering; not needed on real 68000 (which silently ignores it).
- `BUGFIX_UMLAUTS_IN_NAMES` — 8-bit chars in player names break MIDI transmission; this fixes it.
- `ATARI_LONG_HACK` (grep for it) — needed because original code assumes `sizeof(long) == 4`; required on 64-bit hosts.
- `NON_ATARI_HACK` — auto-set off `__atarist__`; disables MIDI master/slave detection at launch.
- `DEBUG_2D_MAZE` — shows all opponents and Ninja path plans on the 2D map.

To re-create the exact original binary, set the BUGFIX flags to 0.

## Subprojects worth knowing about

- `MIDIMaze/Helper Sources/AtariTOS.mm` — macOS implementation of every BIOS/XBIOS/AES/VDI call the game uses. The original `src/*.c` runs unmodified on top of this layer.
- `MIDIMaze/AppDelegate.mm` — runs the game loop on a background thread and bridges it to the Cocoa UI via GCD. The Atari framebuffer is wrapped in an `NSBitmapImageRep`; an Atari ST 8x8/8x16 font and minimal VT52 are reimplemented to preserve the look.
- `gui/tos/gemlib/` — vendored minimal GEM (AES + VDI) library for the MiNT build.
- `InverseCompiler/` — the Python pipeline (`68k2CNew.py`, `findFunctions.py`, `termParser.py`, `symbolManager.py`) used to convert `MIDIMAZE.S` (the disassembled binary) back into the C in `src/`. Reference only; not part of the build.
- `MIDIMAZE.PD/` — the original Public Domain release of the game binary plus manual and mazes.
- `MIDIMAZE2/` — assets and binaries for MIDI-Maze 2 (a separate sequel). Not built from this source tree; the README mentions a possible future branch.
- `README/` — architecture notes (start with `MIDImazeEngineOverview.md`) and file-format docs for `.D8A`, `.MAZ`, MIDI protocol, compressed title screens, and the Atari screen bitmap.
- `src/README.md` — one-line description of every `.c` in `src/`. Useful as an index.
