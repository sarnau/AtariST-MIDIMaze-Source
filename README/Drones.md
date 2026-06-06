# MIDImaze Drones (Computer Players)

MIDImaze ships three computer-controlled opponents, internally called *drones*. They are defined in `src/globals.h:83-85` and implemented in `src/drone.c` (~4800 lines, of which the **vast majority is the Ninja**).

| Type     | Constant         | Char        | Label in UI  | Author's nickname |
|----------|------------------|-------------|--------------|-------------------|
| Target   | `DRONE_TARGET`   | `'r'` (114) | `(Target)`   | Very Dumb         |
| Standard | `DRONE_STANDARD` | `'l'` (108) | `(Standard)` | Plain Dumb        |
| Ninja    | `DRONE_NINJA`    | `'k'` (107) | `(Ninja)`    | Not so Dumb       |

A drone is just a player whose joystick state is synthesized by `drone_action()` into `player_joy_table[]`. The rest of the game (collision, shooting, scoring) treats drones identically to humans, so **all three types share the same `move_player()` physics** — the differences are entirely in how joystick input is generated.

## At-a-glance behavior

### Drone Target — "Very Dumb"
Pure cannon fodder. **Never fires** (the `case DRONE_TARGET` branch in `drone_action()` has no targeting code at all, `drone.c:417-431`). Walks **straight ahead** until a wall stops it — even at intersections it keeps going straight rather than cornering, because `drone_generate_joystickdata()` calls only `drone_move_up()` for this type (`drone.c:1101, 1116, 1131, 1146`). No target tracking, no team awareness, no reaction to being hit.

### Drone Standard — "Plain Dumb"
A reflexive shooter with **no pathfinding**. Knows the target's exact coordinates but only fires when the target is on the **same row or column** and visible down a straight corridor (`drone.c:1496-1547`). Effective range ~3.5 fields (`abs(delta) > 800` in `drone_aim2target()`, `drone.c:1216-1226`, with `MAZE_CELL_SIZE = 256`). Switches target if hit by a human in another team (or in solo). Drones of any type **never** attack other drones, even across teams (`drone.c:447-467`). Source comment calls out the exploit explicitly: *"standard drones can be easily shot diagonally!"* (`drone.c:1492`) — they can't see one cell off-axis.

### Drone Ninja — "Not so Dumb"
Everything Standard does, **plus** multi-step path planning, **plus** stuck-detection. The plan lives in `dr_dir[6]` / `dr_field[6]` of `player_data` (`globals.h:185-191`) — fields that Target and Standard never touch (note the *"not used by the standard drone anyway"* comments at `drone.c:437-465`). Same ~3.5-field shot range; same target-switching-on-hit rules.

---

# Pathfinding deep dive

The drone code is layered. From outside in:

```
drone_action(player)                  // per-frame dispatch on dr_type
 └─ drone_move(player)                // prelude every frame, regardless of type
 └─ drone_sub_{ninja,standard}        // type-specific intent
 └─ drone_check_directions(player)    // 4-cell wall sense
 └─ drone_generate_joystickdata(...)  // unified joystick emitter
```

Plus a handful of helpers:

- `drone_sub_findMoveToTarget()` — re-orient toward target when LoS is lost.
- `drone_isTargetIsVisibleN/S/E/W()` — straight-corridor line-of-sight scans.
- `drone_sub_ninja_plan()` — the heavyweight multi-step planner.
- `drone_sub_ninja_{north,south,east,west}()` — pre-baked L/U-shaped escape plans.
- `drone_aim2target()` / `drone_delta_into_direction()` — integer aiming math.
- `drone_set_position()` — teleport-unstick.
- `drone_move_{up,upleft,upright}` / `drone_turn_around()` — joystick primitives.

## 1. Shared infrastructure

### Coordinate trick: "odd-field indices"
Everywhere in the drone code you see `(ply_y >> MAZE_FIELD_SHIFT) | 1` and `(ply_x >> MAZE_FIELD_SHIFT) | 1`. With `MAZE_FIELD_SHIFT = 7` and `MAZE_CELL_SIZE = 256`, this maps a world coordinate to the **odd-coordinate "." cell** between four potential walls. The four neighbours at `±1` are the wall slots; the four neighbours at `±2` are the next reachable cells. Every wall sense and every plan step works on odd indices and advances by `±2`.

### Joystick synthesis
The output of the whole subsystem is a single word in `player_joy_table[player]`: bits for `JOYSTICK_UP`, `JOYSTICK_LEFT`, `JOYSTICK_RIGHT`, `JOYSTICK_DOWN`, `JOYSTICK_BUTTON`. Drones never send `JOYSTICK_DOWN` (no walking backwards) except as part of the "locked & firing" macro. From `gamelogic.c`'s perspective there's no difference between this word and a human's actual joystick.

### Rotation counters
Two repeating timers govern in-progress turns:

| Field | Purpose | Set in | Read in |
|-------|---------|--------|---------|
| `dr_rotateCounter`     | Multi-frame 90° / 180° turn-in-place (or random-wander turn). Value chosen so that `PLAYER_MOTION_ROTATE = 8` deg/step × counter ≈ 90°/180°. Sign distinguishes left from right. | `drone_move` (random wander), `drone_turn_around` | top of `drone_move` |
| `dr_upRotationCounter` | The first 45° of a forward-into-corner turn. After it expires, `drone_generate_joystickdata()` carries the remaining 45° as long as the corner is still possible. | `drone_move_upleft`, `drone_move_upright`  | `drone_move` |

While either counter is non-zero, `drone_move()` simply re-emits the cached `dr_joystick` word and decrements. This is how a 90° turn at an intersection takes ~7 frames instead of 1.

### The motion primitives (`drone.c:4782-4825`)
Each writes both `player_joy_table[player]` and `dr_joystick` (the cache used by the rotation counters). They also OR in `JOYSTICK_BUTTON` whenever `dr_targetLocked` is set, so the drone keeps firing through its turn:

- `drone_move_up`        → `UP`
- `drone_move_upright`   → `UP|RIGHT`, sets `dr_upRotationCounter = 3` (= `(256/8)/8 − 1`)
- `drone_move_upleft`    → `UP|LEFT`,  sets `dr_upRotationCounter = 3`
- `drone_turn_around`    → `RIGHT`,    sets `dr_rotateCounter = 15` (= `(256/8)/2 − 1`, a 180° spin)

### Wall sense: `drone_check_directions()` (`drone.c:594-614`)
Given the drone's odd-field cell (or an arbitrary one when `useAltCoord` is set — the planner uses this to probe hypothetical positions), it sets four out-params `*canNorthPtr`, `*canSouthPtr`, `*canEastPtr`, `*canWestPtr` by checking the four adjacent wall slots. That's the entire wall model — there's no concept of partial walls or diagonals.

### The "inactive" feedback flag
At the end of `move_player()` in `gamelogic.c:253-259`, **after** physics has resolved the frame, the engine checks:

```
dronesActiveFlag && playerY == ply_y && playerX == ply_x && ply_dir == direction
 && dr_rotateCounter == 0 && !dr_targetLocked
 → dr_isInactive = TRUE;
```

i.e. "the drone produced joystick input but moved nowhere, rotated nowhere, and isn't shooting." This is the *only* place `dr_isInactive` is set TRUE. `drone_move()` reads it next frame to inject either a random turn (Target/Standard/Ninja with no plan) or — for the Ninja only — a teleport-unstick.

---

## 2. The per-frame prelude: `drone_move()` (`drone.c:496-589`)

Runs before any type-specific logic. Returns `YES` if it already produced a joystick word (caller should not run further); `NO` to continue to `drone_sub_*`.

```
if dr_targetLocked:                       return NO   // hand off to shooting code
if Ninja AND inactive AND no plan AND no target:
    drone_set_position to first available compass direction; return NO
if dr_rotateCounter != 0:                              // mid-rotation
    feed cached joystick, decrement, return YES
if dr_isInactive (and rotation idle):
    if facing N or E:
        50% turn right 90°
        25% turn left 90°
        25% turn left 180°
    if facing S or W:                                  // mirror
        50% turn left 90°
        25% turn right 90°
        25% turn right 180°
    if facing off-axis:
        drone_sub_findMoveToTarget(); return NO
    return YES
if dr_upRotationCounter != 0:                          // mid corner-kick
    feed cached joystick, decrement, return YES
return NO
```

Two notes:

- The Target drone hits the "inactive → random turn" branch too. The off-axis case for Target would call `drone_sub_findMoveToTarget()`, but the drone has no target and the function silently no-ops on the missing data. In practice Target drones can only enter the off-axis state if a player collision shoved them mid-frame.
- "50% / 25% / 25%" is implemented as two independent `_rnd(256) & 1` rolls — perfectly fair on a uniform RNG, which is what `_rnd()` uses (the shared seed described in [`MIDImazeEngineOverview.md`](MIDImazeEngineOverview.md) §Random).

---

## 3. The unified joystick emitter: `drone_generate_joystickdata()` (`drone.c:1085-1207`)

Called by every type at the end of its frame. Two inputs: the drone's current facing (`ply_dir`) and the four `drone_can_*` global flags.

```
if dr_targetLocked:                       JOYSTICK_BUTTON; return  // just shoot

if facing in a direction that's open:
    if DRONE_TARGET:    drone_move_up                  // straight only
    else (Standard/Ninja):                              // corner if possible
        if perpendicular "right turn" is open:    drone_move_upright
        else if "left turn" is open:              drone_move_upleft
        else                                       drone_move_up

if facing in a direction that's blocked:
    if perpendicular "right" is open:    drone_move_upright
    else if "left" is open:              drone_move_upleft
    else                                  drone_turn_around
```

Two design notes hidden in the source:

- **Asymmetric corner bias.** Which "perpendicular" is tested first depends on the facing — N tests East first, E tests South first, S tests East first, W tests South first (`drone.c:1100-1158`). With a fair RNG, this still produces a global drift toward **east and south**. In mazes with a perimeter corridor this is why Target packs migrate clockwise.
- **The Target drone's lack of cornering.** This is implemented as a single `if (dr_type == DRONE_TARGET)` check inside the "facing direction is open" branch — when forward *is* blocked, the corner branch is identical for all three types. So a Target *will* round a corner, it just won't divert into a side passage that opens up alongside a still-open forward path.

---

## 4. Target pathfinding

Trivially short. `case DRONE_TARGET` in `drone_action()`:

1. If the drone is dead, reset state.
2. `drone_move()` — handles random-turn-on-inactive, mid-rotation feed-through. If it returns `YES`, done.
3. `drone_check_directions()`.
4. `drone_generate_joystickdata()` — runs through the table above (with the straight-line variant).

There's no target lookup, no LoS, no plan. The only "intelligence" is the corner-fallback when forward is blocked.

---

## 5. Standard pathfinding

`case DRONE_STANDARD` in `drone_action()` (`drone.c:433-488`):

1. Dead → reset.
2. Hit-by-human → maybe re-assign `dr_currentTarget` to the gunman (skipping drone gunmen, and respecting team membership).
3. If target dead or absent in team mode → wander like Target.
4. `drone_move()`. If `YES`, done.
5. `drone_check_directions()`.
6. `drone_sub_standard(player)`.
7. `drone_generate_joystickdata()`.

### `drone_sub_standard()` (`drone.c:1462-1555`)
Picks the target (the gunman if just hit, otherwise `dr_currentTarget`) and reads its exact maze coordinates. Then:

- If `ply_hitflag`: **face the gunman immediately**. `ply_dir = drone_delta_into_direction(dy, dx)` — instant snap, no rotation animation. This is the "unfair turn-and-fire" reflex.
- Else compute `(deltaY, deltaX)` in odd-field coords:
  - `deltaY == 0` → check the appropriate `drone_isTargetIsVisible{East,West}` along the row. On a hit, `drone_aim2target()`; on success, set `dr_targetLocked = TRUE` (next frame `drone_generate_joystickdata()` just fires).
  - `deltaX == 0` → mirror, North/South.
  - Both non-zero (target off-axis) → if currently locked, the lock is broken, and `drone_sub_findMoveToTarget()` re-orients.

That's all the Standard "pathfinding" there is — it walks the maze under `drone_generate_joystickdata()` (the corner-biased version), and the only intent injected from above is "if I happen to share a row/column with the target, lock and shoot." A target one cell off-axis is invisible.

### `drone_sub_findMoveToTarget()` (`drone.c:622-737`) — shared with Ninja
Re-orient toward the target when LoS was lost. Same odd-field coords; uses `drone_can_*`:

```
if same row, target W, can go W → needs2GoWest
if same row, target E, can go E → needs2GoEast
if same col, target N, can go N → needs2GoNorth
if same col, target S, can go S → needs2GoSouth
else (off-axis):
    primary preference = vertical (N/S)
    if primary blocked, fall back to horizontal (W/E) toward target
if nothing matched AND no plan pending:
    re-align facing to the nearest cardinal direction that is open
```

The function ends by stamping `ply_dir` to the chosen cardinal direction *immediately* (another snap) and zeroing out the three other `drone_can_*` flags so `drone_generate_joystickdata()` is forced down that path.

---

## 6. Ninja pathfinding

`case DRONE_NINJA` in `drone_action()` (`drone.c:308-415`):

1. Dead → reset, **including** the plan slots (`dr_dir[0] = 0`, `dr_fieldIndex = 0`, `dr_field[0].y = 0`).
2. Hit-by-human → maybe re-assign target, and again clear the plan.
3. No target in team mode → wander like Target.
4. Target dead → look up the next live human via `dr_humanEnemies[]`, or fall back to wander.
5. `drone_move()`. If `YES`, done.
6. `drone_sub_ninja(player)`.
7. `drone_generate_joystickdata()`.

### `drone_sub_ninja()` (`drone.c:742-1073`) — intent dispatcher
The intent is one of four "needs2Go" globals (`drone_needs2GoNorth`/`South`/`East`/`West`), translated at the bottom of the function into a single `drone_can_*` so `drone_generate_joystickdata()` is forced down that path.

```
read target, compute (deltaY, deltaX) in odd-field coords
wall-sense → drone_can_*
if no plan in progress (dr_dir[0] == 0):
    same row (deltaY == 0):
        target W: if canWest → set needs2GoWest; check LoS → lock if visible
                  else        → try drone_sub_ninja_plan('w')
                                  fallback chain (see below)
        target E: mirror
    same col (deltaX == 0):  mirror with N/S
    off-axis (both ≠ 0):
        target NW/NE: if canNorth → needs2GoNorth
                       else if (canWest or canEast toward target) → that direction
                       else      → plan-less L-fallback chain
        target SW/SE: mirror
if a plan exists (dr_dir[0] != 0):
    walk the plan (see below)
```

The "plan-less L-fallback chain" looks like (using the "want north" case from `drone.c:834-842`):

```c
if (drone_sub_ninja_plan(player, 'n'))         { }
else if ((_rnd(256) & 1) && drone_sub_ninja_west(player, 's'))  { }
else if (drone_sub_ninja_west(player, 'n'))    { }
else if (drone_sub_ninja_west(player, 's'))    { }
else if (drone_sub_ninja_east(player, 'n'))    { }
else if (drone_sub_ninja_south(player, 'e'))   { }
else                  drone_sub_ninja_south(player, 'w');
```

The `_rnd(256) & 1` is the asymmetry-breaker. Without it, two Ninjas in mirror positions would always pick the same detour and end up dancing. With it, ~50% of the time the W/S variant is tried first; the rest of the chain is a fixed priority of "side helpers", trying ever weirder detours.

### `drone_sub_ninja_plan()` (`drone.c:1560-3035`) — the heavyweight planner
This is the function. ~1500 lines, one nearly-symmetric `if (wantedDirChar == 'n' | 's' | 'e' | 'w')` branch each.

What it does, abstractly, for `wantedDirChar == 'n'`:

```
hypothesis-1: detour W, then N
  starting at our cell, walk W one cell at a time:
    at each W cell, look N:
      if N cell exists:
        walk N one cell at a time:
          if target visible N from here → emit plan {W, N}; return YES
          if E cell exists:
            walk E one cell at a time:
              if target visible E       → emit plan {W, N, E}; return YES
              if branch toward target:
                walk N or S one cell at a time:
                  if target visible      → emit plan {W, N, E, N|S}; return YES
                  if axis turn toward target → emit plan {W, N, E, N|S, W|E}
            (dead-end checks abort each loop)

hypothesis-2: same shape, but detour E first, then N
  (only attempted if hypothesis-1 failed and east is open)

return NO if neither hypothesis produced a plan.
```

The plan is encoded into the persistent fields:

| Field | Meaning |
|-------|---------|
| `dr_dir[0..5]`    | Sequence of `PLAYER_DIR_*` constants (or `PLAYER_DIR_NORTH+256` as the "north" marker, because raw 0 means "no plan"). `-1` terminates. |
| `dr_field[0..5]`  | Waypoint odd-field cells. Each `(y, x)` is where the *next* direction change happens. `y == 0` terminates. |
| `dr_fieldIndex`   | Index into both arrays — the current step. |

Search budget: the planner literally walks corridors one cell at a time using `drone_check_directions(... useAltCoord=1, hypotheticalY, hypotheticalX)`, so its cost is `O(corridor-length × branch-cells)` — not BFS, no priority queue, no memo of visited cells. It can re-walk the same cells across hypotheses. **Termination depends entirely on dead-end / "left the maze" guards** (`playerFieldX <= 0`, `playerFieldX > MAZE_MAX_SIZE - 1`, and the `!canN && !canS && !canW` etc. dead-end tests inside each loop).

The planner is **greedy and target-coordinate-driven**: at each branch point it tests "is the target visible from here?" or "is the target on the side I'm thinking of branching into?" before exploring. There's no notion of distance minimization or shortest path — it's "walk perimeters of the obstruction until you can see the target or you've taken 3 turns." For the kind of mazes MIDImaze ships, this is enough; in pathologically long corridors it can fail and trigger the L-fallback chain above.

### `drone_sub_ninja_{north,south,east,west}()` (`drone.c:3040-4781`) — pre-baked L/U escapes
Each is ~440 lines, again one branch per `wantedDirChar`. They are *not* general path planners. Each one encodes a **single fixed plan** for a specific corner geometry. Example, `drone_sub_ninja_west(player, 'n')` (`drone.c:4342-4416`) — used when "I want north, but north *and* west are blocked, and south is open":

```
walk S one cell at a time:
  at each S cell, try W:
    if W open, walk W one cell at a time:
      at each W cell, try N:
        if N open, walk N one cell at a time:
          if N exhausted and E re-opens → emit fixed plan:
            dr_dir   = [S, W, N+256, E, -1]
            dr_field = [<entry S cell>, <W cell>, <N exhausted>, <one cell E>, end]
            return YES
return NO if shape doesn't match.
```

That's a U-bend: go south to get under the obstruction, head west past it, climb north around it, step east to align with the original goal direction. The other functions encode mirror variants. They exist because the heavyweight `drone_sub_ninja_plan()` only handles "go around to one side" patterns and would fail in these dual-blocked geometries.

### Plan execution (`drone.c:923-1057`)
Once `dr_dir[0]` is non-zero, the bottom half of `drone_sub_ninja()` consumes the plan:

```
fieldIndex = dr_fieldIndex
if we arrived at dr_field[fieldIndex]:
    fieldIndex++; dr_fieldIndex++; dr_fieldResetTimer = 0
if dr_fieldResetTimer++ > 78:                    // ~78 frames without arriving
    dr_field[fieldIndex].y = 0                   // abort the segment
if dr_field[fieldIndex].y == 0 OR dr_dir[fieldIndex] == -1:
    // segment ended (target hit, dead-end, or timeout)
    reset plan; carry the last segment's direction as the new "needs2Go"
else if dr_dir[fieldIndex] indicates N|S|E|W and the matching can* is open:
    if first segment (fieldIndex == 0): force-align facing with drone_set_position
    set the matching needs2Go
    on each segment we check LoS; if the target is visible, lock & abort the plan
```

Two important consequences:

- **Plan abandonment is cheap.** Any segment that fails to progress in 78 frames is dropped; the drone falls back to its current cardinal intent and `drone_sub_ninja()` re-plans next frame.
- **The plan opportunistically aborts on a clean shot.** While running plan segment N, if the target is visible down the current corridor, the drone fires immediately and discards the rest of the plan. This is the "Ninja waits for you to cross its line" behavior — when the plan happens to align the drone's facing with the target, no further movement is needed.

### Anti-stuck mechanisms (in order of cost)

1. **Mid-frame teleport-unstick** in `drone_move()` (`drone.c:505-521`): Ninja-only. If the drone is flagged inactive *and* has no plan *and* no target lock, snap it to the centre of its cell facing the first open cardinal direction. This handles "drone got jammed between two other drones and now can't rotate."
2. **Random-turn-on-inactive** in `drone_move()` (`drone.c:538-575`): all types. Same 50/25/25 random turn used during normal wandering also kicks in here.
3. **Plan timeout** (`drone.c:937`): if a waypoint hasn't been reached in 78 frames, the segment is discarded.
4. **Plan failure → L-fallback chain** (`drone.c:784-892`): if `drone_sub_ninja_plan()` returns NO, the directional helpers are tried in priority order.

### Aiming math: `drone_aim2target()` and `drone_delta_into_direction()` (`drone.c:1216-1274`)
The actual fire decision is range-gated to `abs(deltaY) > 800 || abs(deltaX) > 800` (~3.5 cells of 256 units), and integer-only. `drone_delta_into_direction()` is `atan2`-equivalent without floats: lookup into a 33-entry sine table built at boot by `calc_drone_angle_table()` (`drone.c:47-72`). It handles `deltaX > deltaY` and `deltaY > deltaX` separately so the table covers only the 0–45° quadrant; the four quadrants are mapped at the end. Idle quirk: `deltaY == deltaX == 0` returns angle 128 (south).

---

## 7. Worked example: Ninja blocked on the X-axis

Setup: Ninja at odd-field `(7, 5)`, target at `(7, 11)`. Same row. There's a wall directly east of `(7, 5)`, but the corridor reopens at `(7, 9)`. North of `(7, 5)` is open; `(5, 5)` opens east into `(5, 11)`.

Frame N entry, `drone_sub_ninja()`:

```
deltaY = 0, deltaX = 6 > 0 → target east
canEast = FALSE → bypass the "shoot if visible" branch
hit drone.c:806 — "drone wants to go east, but can't":
  drone_sub_ninja_plan(player, 'e')
```

Inside `drone_sub_ninja_plan('e')`:

```
hypothesis-1: try detour N then E
  at (7,5): canNorth ✓
  walk N: at (5,5), canEast ✓
  branch E from (5,5):
    isTargetIsVisibleEast(5,5)? target is on row 7, not row 5 → NO
    walk E one cell at a time along row 5...
    at (5,11): canSouth ✓ and targetFieldY (7) > playerFieldY (5)
    emit plan {N, E, S}:
      dr_dir   = [N+256, E, S, -1]
      dr_field = [(5,5), (5,11), (7,11), end]
    return YES
```

Frame N+1, plan execution branch of `drone_sub_ninja()`:

```
fieldIndex = 0, dr_field[0] = (5,5), dr_dir[0] = N+256
ply_dir != NORTH → drone_set_position(player, 'n') (snaps & faces N)
needs2GoNorth ← TRUE
drone_generate_joystickdata(): facing N, canNorth ✓, canEast ✗ → drone_move_up
```

Walk N until the drone arrives at `(5,5)`. `dr_fieldIndex` increments to 1. `dr_dir[1] = E`. The drone takes a 90° corner via `drone_move_upright` (since canEast becomes ✓ at `(5,5)`). On every frame thereafter the plan-execution code also runs `drone_isTargetIsVisibleEast()` checks; the moment the drone reaches `(5,11)` it sees the target two cells south is *not* visible east, but `dr_dir[2] = S` directs it south. Once it turns south at `(5,11)`, the LoS check at `drone.c:1019-1034` notices the target directly south, calls `drone_aim2target()`, sets `dr_targetLocked`, **discards the rest of the plan**, and `drone_generate_joystickdata()` switches to "just shoot."

If the target moves out of the (7,11) cell before the drone arrives, plan execution will either complete the final south leg (and the drone is now at `(7,11)` with `dr_dir[3] = -1`, so the plan ends and the drone re-enters `drone_sub_ninja()`'s top half on the next frame), or — if a wall flag flips mid-plan because a player blocks a cell — the 78-frame timeout abandons the current segment.

This is the canonical Ninja sequence: **single-shot planner → forced cardinal moves → opportunistic LoS shot.** The Ninja is dangerous because the planner runs every time the drone is at an intersection without a plan, so as long as the maze topology permits a 1–3-turn route, the Ninja effectively always has a current plan.

---

# Firing deep dive

Drones do not "shoot" directly. Like humans, they set a bit in `player_joy_table[player]` and let `move_player()` / `move_shoot()` in `gamelogic.c` actually spawn and propagate the projectile. The drone-specific part of firing is therefore just three things: **deciding to fire**, **aiming**, and **emitting `JOYSTICK_BUTTON`**. Everything from there onward is the same physics every player runs.

## 1. The lock state machine: `dr_targetLocked`

A single boolean in `player_data`. When TRUE, the drone is in "fire, don't move" mode.

```
set TRUE only via:           drone_aim2target() returning YES
set FALSE in:                death/reset (drone.c:312,326,335,388,403,422,438,452,461,475)
                              drone_sub_findMoveToTarget() entry (drone.c:641)
                              every drone_sub_ninja[_plan] branch where LoS is lost or replanned
                              drone_sub_standard() LoS-lost branches
                              drone_sub_ninja plan-execution clean-fire branch (drone.c:980,1001,1022,1043)
```

Once TRUE the drone shortcuts through three places:

- `drone_move()` returns NO immediately (`drone.c:500`), so the per-frame prelude doesn't synthesize a turn or random wander.
- `drone_generate_joystickdata()` fires the dedicated `JOYSTICK_BUTTON` branch at the top (`drone.c:1091-1095`):
  ```c
  if (player_data[player].dr_targetLocked) {
      player_joy_table[player] = JOYSTICK_BUTTON;
      return;
  }
  ```
  No `UP`, no `LEFT/RIGHT`. The drone stands still and pumps the trigger.
- The motion primitives in `drone.c:4770-4825` (`drone_move_up`, `drone_move_upleft`, `drone_move_upright`, `drone_turn_around`) inspect the flag at call time and **OR `JOYSTICK_BUTTON` into the cached joystick word** when locked. So if the drone happens to be mid-rotation (a corner kick or a 90/180° turn-in-place) at the moment the lock fires, it continues to fire each frame *while* completing the turn. The cached word is what `drone_move()` re-emits each frame while `dr_rotateCounter` / `dr_upRotationCounter` is non-zero.

The lock is **single-frame fragile**. Every type-specific subroutine that runs after `drone_move()` either re-asserts the lock (by calling `drone_aim2target()` again) or clears it. There is no "remember the last good aim" — if the LoS check fails next frame, the drone drops the lock and resumes navigation.

## 2. Line-of-sight scanners: `drone_isTargetIsVisible{North,South,East,West}` (`drone.c:1279-1406`)

Four near-identical functions. Given a player and an optional alternative `(fieldY, fieldX)` start cell (the planner uses this to test hypotheticals):

```
read target's odd-field (targetFieldY, targetFieldX)
if target on a different column/row than the start cell → NO
if start cell already on or past the target's column/row in the wrong direction → NO
walk the corridor cell-by-cell (±2 in the scan direction):
    each step, check the wall slot between this cell and the next
    stop when you hit either a wall or the target cell
return YES iff you arrived at the target cell.
```

A few details:

- **Same-cell short-circuit.** If the target is *in* the start cell, the function returns YES without scanning (`drone.c:1297-1298`). This handles the "drone and target standing on the same field" case the drone is allowed to fire.
- **No partial visibility.** Either the corridor is fully clear from drone-cell to target-cell or there's no shot. The drone has no concept of "peek around a corner" — that's what `drone_sub_ninja_plan()` is for, not the LoS scanner.
- **L-shaped paths are invisible.** The Standard drone with the target one cell off-axis fails *every* LoS check, which is the source of the well-known "shoot Standards diagonally" exploit.
- **Used everywhere targeting happens:** `drone_sub_standard()` (4 sites), `drone_sub_ninja()` direct-shot branch (4 sites), `drone_sub_ninja_plan()` per-cell hypothesis tests (many sites), and the Ninja plan-execution clean-fire shortcut (`drone.c:978, 999, 1020, 1041`).

## 3. Aim math: `drone_aim2target()` and `drone_delta_into_direction()` (`drone.c:1216-1274`)

`drone_aim2target()` is the only place that actually rotates the drone toward the target. Three things:

1. **Range gate** (`drone.c:1225-1227`):
   ```c
   if (abs(deltaY) > 800 || abs(deltaX) > 800 || ply_lives <= 0) return NO;
   ```
   `800` in raw units, with `MAZE_CELL_SIZE = 256`. The gate is L∞ (Chebyshev): a 1600×1600 square centred on the drone. **In practice that simplifies to "≤ 3.125 cells along one axis"**, because every caller has already established (via the LoS scanner) that the target is on the drone's exact row or column — so one delta is 0 and the gate reduces to `|other delta| ≤ 800`. The author's source comment says "a bit more than 3 fields away."
2. **Snap aim** (`drone.c:1235`):
   ```c
   player_data[player].ply_dir = drone_delta_into_direction(deltaY, deltaX);
   ```
   The drone's facing is **assigned directly** — there is no rotation interpolation, no `dr_rotateCounter`. This is the second piece of "drone cheating": humans have to rotate at `PLAYER_MOTION_ROTATE = 8` degrees per frame, drones teleport their gun. Combined with the snap re-aim that `drone_sub_standard()` does on `ply_hitflag` (`drone.c:1485`), this is why drones never miss the first shot when a player walks into their corridor: the moment the LoS scanner reports YES, the gun is already pointed at the target.
3. **Dead variable**: `dr_fireDirection` is set in `drone_aim2target()` and `drone_sub_findMoveToTarget()` but is never read anywhere. Comment marks it as "only set, never read." Probably leftover from an earlier design where the drone tracked aim independently from movement direction.

`drone_delta_into_direction()` is an **integer `atan2`** that returns a MIDImaze direction unit (0..255, north = 0, clockwise). The implementation:

```c
// abs() the deltas, remember signs
if (deltaX <= deltaY) angle =       table[muls_divs(32, deltaX, deltaY)];
else                  angle = 64 -  table[muls_divs(32, deltaY, deltaX)];
// 4-way quadrant remap based on (signY, signX)
// final 90° flip so 0 = north
return (128 - angle) & 0xff;
```

The 33-entry `drone_angle_table[]` is built once at boot by `calc_drone_angle_table()` (`drone.c:47-72`). Construction is a **reverse lookup**: for each candidate angle `i ∈ [0, 32]`, rotate a length-1000 vector by `i` units, then index by `muls_divs(32, x, y)` (a fixed-point `32 · x / y`). Gaps are filled by carrying the previous value forward. The result is a quantized inverse-tan with ~5.6° resolution (one unit out of 256 ≈ 1.4°, so a 4-unit table-step ≈ 5.6°). Because the table only covers the 0..45° octant, the function reflects across the diagonal (`deltaX <= deltaY` vs not) and remaps quadrants explicitly. The author's comment includes the float-math equivalent for anyone curious:

```c
return 0xff & ((int)(atan2(deltaY, deltaX) * 256 / (2 * M_PI) + 64));
```

Edge case: `deltaY == deltaX == 0` returns angle 128 (south). Harmless because the range gate would have triggered first when the drone and target share a cell only after a LoS-YES — and at that moment the drone's existing `ply_dir` is irrelevant.

## 4. From intent to bullet: how `JOYSTICK_BUTTON` becomes a shot

The drone's frame ends with a write to `player_joy_table[player]`. Some frames later in the same iteration of `game_loop()` (`maingame.c`), every player's joystick word — including drones' — is passed through `move_player()` in `gamelogi.c`. That function contains the only `JOYSTICK_BUTTON` consumer:

```c
// gamelogi.c:124
if (((joystickData & JOYSTICK_BUTTON) == JOYSTICK_BUTTON) && ply_reload == 0) {
    if (player == own_number) Dosound(sound_shot_ptr);
    ply_shootr = direction;             // direction was just rotated by LEFT/RIGHT bits
    ply_shoot  = 10;                    // "active shot" flag (initial value of 10 is dead-code; never decremented)
    ply_reload = reload_time;           // arm the cooldown
    ply_shooty = playerY;
    ply_shootx = playerX;
}
```

Three things matter for drones here:

- **The shot is fired in `direction`, not `ply_dir`.** `direction` is `ply_dir` rotated by ±`PLAYER_MOTION_ROTATE` if `JOYSTICK_LEFT` or `JOYSTICK_RIGHT` was also sent (`gamelogi.c:117-121`). Normally the locked-and-firing drone sends `JOYSTICK_BUTTON` alone (the dedicated branch at `drone.c:1093`), so `direction == ply_dir` and the shot leaves at the snap-aimed angle. But if `dr_targetLocked` flipped to TRUE *during* a corner kick (so the cached joystick word contains, e.g., `BUTTON|UP|RIGHT`), the engine applies the +8° rotation first and then fires — the shot leaves at `ply_dir + 8`. Next frame the drone will re-snap with `drone_aim2target()` and the drift won't compound, but the first shot during the rotation can miss by up to 8°.
- **Reload, not "shot in flight", gates re-fire.** The test is `ply_reload == 0` only. There is no `&& ply_shoot == 0`. So a drone can re-fire even if its previous shot is still travelling — provided the reload timer has elapsed. That matches the game's behavior where holding fire produces a continuous stream every `reload_time` frames.
- **Sound is local.** `Dosound()` is only called when `player == own_number`. Drones are never `own_number`, so **drones fire silently on your screen** — you only hear shots you fired yourself. The same applies to `move_shoot()`'s hit-sound check (`gamelogi.c:346`): it fires when the *shooter* is `own_number`, so you hear your own hits, not the hit that just landed on you. Whether you're a victim is reflected only through `update_happiness_quotient_indicator()` and the screen flash.

## 5. Reload timing: `TIME_RELOAD_FAST` / `TIME_RELOAD_SLOW`

Defined in `globals.h:106-107` as `10` and `30` frames. The actual value is in `reload_time`, set by the master via the preferences dialog (`prefdlg.c:209`) and broadcast to slaves. **Drones use the same global** — there is no separate `drone_reload_time`. So:

- Fast reload (10 frames @ ~50 fps ≈ 0.2 s): a Ninja with continuous LoS can drop a human in `lives × 0.2 s`.
- Slow reload (30 frames ≈ 0.6 s): much longer window to escape after the first shot.

This is the only knob that changes drone offensive pressure without changing drone count.

## 6. Shot physics: `move_shoot()` (`gamelogi.c:289-398`)

After firing, the shot is an independent particle. Each `game_loop()` frame:

```
for zCoord in 0..3:                    // 3x player speed
    shotY += xy_speed_table[shotDirection].deltaY
    shotX += xy_speed_table[shotDirection].deltaX
    if crossed a cell boundary AND that boundary is a wall slot:
        ply_shoot = 0; break          // shot dies on walls
    for each of the 9 cells in {±2,0} × {±2,0} around the shot cell:
        walk the linked list of players (ply_plist chain) in that cell
        for each candidate player who is not the shooter and is alive:
            if |target.ply_y - shotY| <= 48 AND |target.ply_x - shotX| <= 48:  // PLAYER_RADIUS
                register the hit; apply team / friendly_fire rules; ply_shoot = 0; stop
```

Notes worth noting in a drone context:

- **3× faster than the drone can move** (`for zCoord = 0; zCoord < 3`). Combined with the snap-aim, a drone's shot reaches a target one cell away in ~3 frames once `JOYSTICK_BUTTON` lands. That's about 60 ms on real Atari ST hardware. There is no human reaction.
- **Hitbox is a square** (`PLAYER_RADIUS = 48` units, L∞), even though the symbol is named "radius" and the comment says "radius." The shot itself has zero radius — `move_shoot()` only checks `|dy| <= 48 && |dx| <= 48` against each player's centre. A shot grazing the edge of a player's circle visually but not within the square misses; conversely, a shot inside the square but outside the inscribed circle hits.
- **Friendly fire rule** (`gamelogi.c:356`): the life-decrement block runs when "not in team mode, OR teams differ, OR (in team mode, same team, friendly_fire flag asserts that wounds are allowed, AND the target has more than 1 life)." Reading the source variables literally, in team mode with friendly fire off you can knock teammates down but never kill them — they stay pinned at 1 life. The drone benefits from this implicitly: it doesn't check team membership before firing, so a teammate Ninja can shoot you, just not finish you off.
- **`ply_hitflag` and `ply_gunman` are set even when no life was lost.** Both are checked by `drone_sub_standard()` / `drone_sub_ninja()` to swing the drone's `dr_currentTarget` over to the human who just shot it — which is how a drone "noticed" the hit even on a no-damage friendly-fire grace event.

## 7. Recap: end-to-end firing sequence

For the canonical Ninja-locks-and-shoots case:

```
frame N
  drone_action(player)
    drone_sub_ninja()
      same-row check (deltaY==0, target W, canWest)
        drone_isTargetIsVisibleWest() → YES
          drone_aim2target()
            range gate: |deltaX| <= 800 ✓
            ply_dir = drone_delta_into_direction(0, deltaX)  // = PLAYER_DIR_WEST or close
            return YES
          dr_targetLocked = TRUE
          (reset any pending plan)
    drone_generate_joystickdata()
      dr_targetLocked → player_joy_table[player] = JOYSTICK_BUTTON
  // ...all other players resolve...
  move_player(player, JOYSTICK_BUTTON, ...)
    direction = ply_dir  (no LEFT/RIGHT, no change)
    JOYSTICK_BUTTON && ply_reload == 0:
      spawn shot at (ply_y, ply_x) heading direction
      ply_reload = reload_time

frame N+1
  drone_action(player)
    drone_move(): dr_targetLocked → return NO
    drone_sub_ninja(): same-row check → still LoS YES → still snap-aim → re-assert lock
    drone_generate_joystickdata(): JOYSTICK_BUTTON
  move_player(...)
    JOYSTICK_BUTTON && ply_reload != 0:  no fire this frame; ply_reload--
  move_shoot(...) advances the previous frame's shot by 3 cells; collision-checks; potential hit.

frame N+reload_time
  reload expires; next BUTTON triggers shot 2.
```

The drone is unable to "lead" a moving target: it always aims at the target's *current* position, not its predicted position. A human strafing diagonally at full speed across the drone's corridor will be aimed at, then shot — but the shot travels in a straight line at the drone's old aim. If the strafer is moving fast enough that the diagonal velocity carries them past the corridor in less than 3 shot-frames, the shot misses. (In practice this rarely helps because the corridor is the only place LoS exists, and you have to leave it to dodge — at which point the lock breaks and the drone goes back to navigating.)

---

## What they all share

- **Assigned target.** `drone_setup()` (`drone.c:78`) walks teams (or the solo human roster) and gives each drone a `dr_currentTarget` and a `dr_permanentTarget`. The permanent target is always a human; if it dies, the drone falls back to the next live human via `dr_humanEnemies[]`.
- **No drone-on-drone violence**, including across teams. The hit-reaction checks in `drone.c:321, 375, 447` explicitly skip gunmen whose `dr_type` is `DRONE_NINJA` or `DRONE_STANDARD`.
- **Synthesized joystick.** `drone_generate_joystickdata()` writes into `player_joy_table[player]`. From `move_player()`'s perspective the drone is just another player.
- **Inactivity flag** set in `gamelogic.c:258`, consumed in `drone_move()` as the entry point for random turns or (Ninja) teleport-unstick.
- **Integer-only aim.** No floats anywhere; sin/cos via 33-entry table.
- **Drones can only travel along 90° axes.** Open-area diagonals are reachable to shots (`drone_aim2target` rotates to arbitrary angles), but not to movement — `drone_generate_joystickdata()` always commits to one cardinal direction at a time. Source comment: *"drones will never go diagonal in an open area!"* (see [`MIDImazeEngineOverview.md`](MIDImazeEngineOverview.md) §Drones).

## Quick summary

> **Target** = moving piñata; just `drone_check_directions()` + `drone_generate_joystickdata()` with no corner bias and no targeting.
> **Standard** = same wandering loop but with corner bias, plus `drone_sub_standard()` which fires only on perfect row/column alignment within ~3.5 cells.
> **Ninja** = same firing plus `drone_sub_ninja_plan()` (greedy ≤3-turn coordinate-driven planner) plus `drone_sub_ninja_{N,S,E,W}()` U-bend fallbacks plus a 78-frame plan timeout plus a teleport-unstick.

## UI / debugging hooks

- The preferences dialog labels them via `str_drone_type_br_array[] = { "(Target)", "(Standard)", "(Ninja)" };` in `src/prefdlg.c:245`.
- Enable `DEBUG_2D_MAZE` in `src/globals.h` to draw every opponent and every Ninja's current plan on the 2D overhead map. This is the fastest way to see the planner output.
