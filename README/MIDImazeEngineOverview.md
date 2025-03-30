# MIDImaze Source Overview

## General data types

Positions of objects are always in the Y/X order. A single maze field is 256 units large `MAZE_CELL_SIZE`. The coordinates are always position, with Y/X 0,0 being in the top-left corner of a maze. Negative numbers _can_ occur if e.g. a delta between two players is calculated or in case of an underflow (during collision detection)

A viewing direction is in degrees, but not 360 or 2PI, but instead 256 angle units. 0 is `PLAYER_DIR_NORTH`, moving clock-wise.  It is always a positive number.

Using 256 units makes it very easy to do clipping and masking of an overflow, by just masking the lower 8 bits.

MIDImaze does not use any floating point math. The types above are good enough. The sin/cos calculation for the 2D rotation is done with a 65 words lookup tables, which is loaded at launch.

The maze file is documented in the original manual. It is text based and lines have to end with CR/LF (as typical on an Atari ST):

    14
    XXXXXXXXXXXXXXX
    X.............X
    X.XXX.XXXXXXX.X
    X.X.........X.X
    X.X.XXX.XXX.X.X
    X.......X.....X
    …

The first 2 digits are with size of the square maze (the maximum is 62 - it also always has to be an even number). The right and lower wall are not counted. If X _and_ Y are an odd number, then this field has to be a '.', which means that it is always empty. The 4 fields directly around it can then be either a wall 'X' or empty '.'. The corners are unimporant during gameplay and only set to make the maze file more readable for a human. It is very important to understand that walls in MIDImaze have no thickness at all, despite a maze file looking like it.

MIDImaze stores the maze into `maze_datas[64*64]` and the size in `maze_size`. The engine takes a coordinate Y/X divides it by 128 and or's 1 to it to find a "field coordinate". This coordinate will be a '.' between 4 potential walls around it. This allows MIDImaze to easily test if there is a wall in either of the 4 compass directions. Corners (Y/X are both even) are never tested. The connection between fields (= coordinates within the maze file) and cell coordinates (absolute coordinates of players and walls) is very important! Look at `draw_2Dmap()` to see how the 2D maze is drawn and at `set_ply_2Dmap()` to see how players are positioned inside the maze.

It is also important to make sure that all no-go spaces have all 4 possible walls set in the area. On positioning a player, MIDImaze tests if a player would end up in an area with 4 walls around and ignore these areas. If the area is just a bit larger, then a player can find themselves in a space with no way out.

All player information is stored in `player_data[PLAYER_MAX_COUNT]`. Elements starting with `dr_` are only used by drones.

In this array the human players come first (0 is the master), followed by the drones (in the order Target, Standard, Ninja) and filled up with potential MIDIcams (which are automatically rotating fixed-positioned cameras in the maze). Depending on the code only the human players are iterated over or all players including the drones. `own_number` is the number of the playing user at a computer. 0 is the master, 1 is the first one behind the master, etc. This number is set during MIDI setup. A MIDIcam changes `own_number` to allow following and watching other players.

Players and shots are stored at odd-coordinates as linked lists (placed via `set_object()`, which is called from `set_all_player()` to position all players and their shots). If nothing is at that field, it contains -1, otherwise an index to the `player_data[]`. If `index >= PLAYER_MAX_COUNT`, then it is a shot (only one per player is possible, so you can just subtract `PLAYER_MAX_COUNT` to get the correct player). If it is a player, the next element index is in `.ply_plist`, if it is a shot, the next element index is in `.ply_slist`. -1 ends the list.

This is all that is needed to play MIDImaze. There are more data structures related to 3D rendering, but for the game logic, this is it.


## The game with the main loop

### `main()`
Entry point, only initializes AES and VDI, then calls `setup_game()`.
On return it restores the colors and closes VDI 

### `setup_game()`
Check for the correct scren resolution (640x200x4 is not supported) and initializes variables and data structures. Also loads all artwork, including loading and showing the splash screen (which calls the copy protection check). Then calls `dispatch()` to continue.

### `dispatch()`
This loop switches between all the different game modes:
  - Master - This computer is the master in a MIDI ring, similar to solo
  - Solo - A single player without a MIDI ring.
  - Slave - This computer is a slave in the MIDI ring. It is controlled by the master. Most dialogs are passive.
  - MIDIcam - Similar to slave, but the Computer just allows watching the game.
  - Automatic - Default at launch: tries to detect if the MIDI ring. The last computer to go online will complete the ring and become the master.
  - Quit - Quit the game

### `master_solo_loop()`
Main loop for the master and solo player mode. Allows the player to load a maze (MIDIMAZE.MAZ is loaded as the default), configure and start the game, etc. All is available via the mouse in the standard Atari way: menus and dialogs. Via key commands it is possible to switch back to automatic and slave. Also switching between mouse/joystick control of the game is possible via a key command.

### `slave_midicam_loop()`
Main loop for the slave and MIDIcam mode. Controlled by the master via MIDI messages. Via key commands it is possible to switch to other modes and the mouse/joystick control of the game. The game starts by MIDI messages from the master.

### `game_loop()`
The main game, called by all 4 game modes. Returns when a game is over. After initializing a bunch of variables and state, transmitting it via MIDI, showing the 2D map as an overview for 5s, it enters end 'endless' loop, that is only left it a player/team won or the game is terminated. This loop does the following (I leave out the special cases for the MIDIcam)

  1. Did the player got hit? If so, flash the screen in the color of the gunman.
  1. Update the notes score table, if necessary
  1. One of the following cases:
    1. If the 2D map is to be displayed, do that.
    1. If the player is alive, calculate the 3D maze and display it. If a shot is available, draw the crosshair.
    1. The player is dead, Display a large smiley of the gunman with the "Have a nice day!" message.
  1. If the score of the current player changed, update the pop chart
  1. If we got a winner, leave the loop
  1. Get the joystick/mouse input and store it in `player_joy_table[]` - this table contains all input for all players (and drones)
  1. Check the keyboard. The space bar toggles between 2D/3D map. The master can terminate the game with the Esc key.
  1. In master/slave mode: send all joystick data to all other players and receive theirs.
  1. Calculate the joystick data for the drones
  1. Reset the "got hit" flag for all players (see above)
  1. Check if the master wants to terminate the game. If so, do that.
  1. Process the `move_player()` for all players and drones. Every round a different player is processed first. This avoids that a player with a lower number will always have an advantage (e.g. shooting first).
    

## Game Logic

The game logic is inside `GAMELOGIC.C`, it has two main functions:

###  `move_player`
Call from the `game_loop()` once per player. It does the following:

  1. If the refresh counter is active, count it down. If it hits 0, add a new live to the player and restart the refresh timer, if the maximum number of lives has not been reached. Also check if the player had no lives (= was dead), then revive with the number of lives as defined by the master. Find a new starting position in the maze and hide the 2D map. If the player is the player on this machine, play a sound (for gaining a live) and update the happiness quotient.
  1. If the player is currently dead, still allow a shot to continue and update it's reload counter and return.
  1. Process the joystick data and deal with the rotation of the player. That requires no further check, a rotation is always possible.
  1. If the fire button was pressed and a shot available, play the shot sound (on the current machine only), set the shot direction as the viewing direction and it's coordinates as the players coordinates. Update the reload counter.
  1. If a shot is active, process the shot in `move_shoot`
  1. Count the reload counter down, if necessary
  1. Deal with Joystick up/down. Pick the Y/X speed from a lookup table, based on the current viewing angle.
  1. Add the speed to the Y/X position of the player and check the 9 fields around the player for any other players to avoid collisions. Push the player back, to avoid an overlap of two players.
  1. Check if the player is now too close to a wall, if so, enforce a distance to the wall(s)
  1. If the player is a drone: check if the drone moved/turned at all or has an intention of moving or a target. If not: mark the drone as inactive. This will trigger a new motion in the next round.
  1. Update main `player_data` structure with the new player position and direction
  1. Because the player collision test is buggy, it can happen that a player ends up being pushed into the next field without a wall check. To avoid that a player is stuck in a little cell, it is checked if the player is surrounded by walls. If so, a new position is searched and the player jumps to it.


### `move_shoot`
Similar to `move_player`, this function manages a shot for a player.

  1. The shot is moved 3 times/steps in this code. Which makes it 3 times faster than the player.
  1. After moving the shot by one step, check if it collided with a wall. If so: remove it and exit
  1. Check the 9 fields around the shot for players. Consider their size and see if the shot (which has no radius for that matter) hit one of the players (not counting the gunman, to avoid a player shooting themselves). If the current player has been hit, play a sound, mark the player as hit and store the gunman with the player. Check if the shot was hitting a team member with friendly fire active and more than one life left or just hitting. Set the refresh counter for the hit player, subtrace a live. If the lives went down to 0, update the refresh counter with the revive time, add a point to the gunman. Check if the game was won by the gunman or it's team. Store the hit player as the looser within the gunman to show the hit later. Remove the shot. Update the happiness counter, if the current player was hit.
  1. Update main `player_data` structure with the new shot position
    

### `hunt_ply_pos`

Not technically part of the game logic, but still interesting. This routine places a player within the maze. It tries 666 times to find a position and if it fails, the game is over. This typically happens in mazes which are too small for all players. It picks a random field in the maze and checks if it is surrounded by walls. If so, it will move on. It then calulates a minimal distance to all players currently alive based on the `tries` loop counter. In a perfect scenario a new player will not show up within a 5 field radius to any other player to not be immediately get killed. However with each failed try, the distance is reduced, till it becomes a negative number, which always matches.

If a position is found, a random direction is picked (only N,E,S and W are possible) and drone variables initialized, if the player is a drone.


### `_rnd()`

MIDImaze uses two different random number generators. The Atari ST XBIOS function `Random()` and it's own internal `_rnd()`. This is really important in MIDI play, because all players need to use the same random generator, which all have to be perfectly in-sync. Otherwise a killed player would pop up in different places for different players, which would be really bad. But in some cases MIDImaze (namely the MIDIcam mode) is not allowed to use the shared random generator. MIDIcams are totally passive, but e.g. pick random players to follow. This randomness is not synced via the network, so the Atari function is used here. In all other cases the master generates two random bytes via `Random()` and shares them before the game with all slaves.


## Drones

DRONE.C is the main code to manage drones. It has two main functions: one for setup, called before the game and one that is called from game_loop() to generate joystick data for the drones.

There are 3 drone types:
- Target ("Very Dumb")
- Standard ("Plain Dumb") 
- Ninja ("Not so Dumb")

This target drone never fires a shot, it just moves around. The other drones will never fire at each other, even in different teams. They will only fire at human oponents.

### Drones motion

This is the drone motions explained, if there is no active target, which is mostly the same for all drone types. Because Standard and Ninjas typically have a target, they often behave a bit differently. Especially the Ninja tries to make a plan to get closer to the player.

Drones have an 'inactive' state, which is set inside `move_player()`. After doing all the motion and collision calculation, the code checks if the drone moved at all or at least rotated (or has the intention to rotate or to fire at a target). If none of that is true, a flag is set to make the drone as "inactive". The drone code will then generate new motion data in the next round and reset the variable.

`drone_move()` generate a normal motion pattern for any drone type, if no target is locked or tracked down (e.g. because the target is dead). 

A drones has two counters: `dr_rotateCounter` for all rotations triggered by inactivity and turn-around (in case of a dead-end) and `dr_upRotationCounter` just to get around corners. It works as an auto-repeat to feed the same joystick data into the game code several times in a row.

In the move code, `dr_rotateCounter` is checked first. As long as it's not zero, it feeds the old joystick motion code right back. If the drone is inactive, it follows three patterns:

- if facing north or east:
    - 50% chance to turn right 90 degrees
    - 25% chance to turn left 90 degrees
    - 25% chance to turn left 180 degrees
- if facing south or west:
    - 50% chance to turn left 90 degrees
    - 25% chance to turn right 90 degrees
    - 25% chance to turn right 180 degrees
- if facing in a non-90 degree aligned angle (this can probably only happen, if the drone is stuck in a group of players - I never saw that one in games)
    - It tries to find the target player
If the drone is active, it then checks `dr_upRotationCounter`. As long as it's not zero, it feeds the old joystick motion code right back.

If `drone_move()` returns NO, because no joystick data was generated by a timer, or the drone was not inactive, the following code path happens:

Standard and Ninjas do their different checks to find the target. If the target is dead, it then matches the behavior of the Target:
First check all 4 directions around the drone in `drone_check_directions()`. This simply provides info in which of the 4 directions the drone is blocked by walls. Then `drone_generate_joystickdata()` is called to generate final joystick data for the drone. If a target is locked, the drone will just shoot – no further motion necessary. Target drones will always follow a given direction till they are blocked and only then change direction. Standard/Ninja on the other hand will always try to take a turn, if possible. They follow this pattern:

- Drone faces north and north is available:
    - east available? => take that right turn
    - west available? => take that left turn
    - otherwise just go north
- Drone faces east and east is available:
    - south available? => take that right turn
    - north available? => take that left turn
    - otherwise just go east
- Drone faces south and south is available:
    - east available? => take that left turn
    - west available? => take that right turn
    - otherwise just go south
- Drone faces west and west is available:
    - south available? => take that left turn
    - north available? => take that right turn
    - otherwise just go west

If a drone is facing a certain direction, but that one is blocked, it will turn this way:

- facing north: east before west before turn around
- facing east: north before south before turn around
- facing south: east before west before turn around
- facing west: north before south before turn around

As you can see by the order of checks and no randomness: drones will prefer to go north/east, if there is no other chance. In many mazes with a path all around the outside perimeter, Target drones will end up running in circles in big packs and only take a turn if accidentally the formed a pulk and were blocked from moving. Standard drones often circle around after they found the player, but they can be easily outrun - once a player is more than 3 fields away, the drone will simply give up and go back into waiting for the player while moving around.

Ninja drones are extremely focused on hunting down the player to the point that they are almost never inactive. They are also  unfair, because they 'known' where the player is, relative to them and wait for the player to cross their x or y coordinate anywhere in the maze. If that is the case, then the drone will make a plan to get closer to the player by minimizing the distance with the option to take up to 3 turns on the way. Once a Ninja is in tracking mode, there is very little to get rid of the Ninja, if you don't know where the Ninja is. They can get stuck, but without that knowledge they will unstuck once you move again.

### Drone Target

The Standard and Ninja drones have two types of targets. Both have a permanent target - assigned at the start of the game. The permanent target is always a human player. This player will be hunted down as long as the player is alive. If the player is dead, then the next human player from a list is picked. If a human player shoots a drone, that player will become the current target immediately for the drone.

#### Standard Drone Target

Without a current target alive or in team play without a current target, the Standard drone will just move around without a plan or randomness. That is even simpler than the Target drone (and it looks like a bug). If the target is alive, it will find a move, just the Target drone, but if no move is pending, it will call `drone_sub_standard()` which will try to find the target player. If shot, the drone will change direction immediately to face the gunman (unfair...). Otherwise it will check if the target is at the same field either on the Y or the X axis. If so, it will look down all fields along the other axis till it either finds a blocking wall or the player. If found, it will try to adjust the aim (player has to be <=800 units or about 3 fields away and alive), turn towards the player and lock onto the player. When locked to the player, it tries to move closer to the player, if there is a direct line towards the player. If not, then the lock is released and the Standard drone will just move around.

As you can see: by moving away from a Standard drone, just around the corner, you can get rid of it. And they can be easily shot without shooting back, if you shoot diagonal at the drone.

#### Ninja Drone Target

The Ninja follows a similar pattern as the Standard dron, but with more nastiness. While a Standard drone looses sight easily, just like a human player, the Ninja will setup a plan to get closer to the player even without being able to see the player. It does so, by playing unfair: the computer knows were the player is. The Ninja then plans up to 3 steps ahead (one step being a movement from one intersection in the maze to the next) to get closer to the player. If the simple Standard drone algorithm doesn't work, it will then follow this plan. This makes it extremely hard to impossible to move in the maze without having a Ninja actively hunting down the player. The tactic firing diagonal only works for a very brief moment, because the Ninja will immediately track you down and fire back.

The plan of a Ninja works very well in small mazes (too few corners to get away) or in large mazes, which are not overly complex (a lot of open spaces, many ways to get from a to b, etc). The problem is of course, that complex mazes are also hard for human players.

There is one little thing: drones will never go diagonal in an open area! They can only go in 90 degree angles (shooting works at every angle, so you need to keep moving)

## 3D rendering

MIDImaze pulls a lot of tricks to get the 3D rendering of the maze done well on an 8MHz 68000. I'll explain them here.

Rendering is done in two steps:

  1. Generate a render list of all objects (walls, players and shots) via `make_draw_list()`
  1. Draw the renderlist via `draw_list()`

Almost all of the tricky math is done during the generation of the render list.

### Generate a render list

The render is generate for a specific position/direction, which are kept in 3 global variables. It then calculates a viewmatrix for that position. That viewmatrix deals the with 2D rotation of all visible fields with only four 2D rotations. It does so by rotating just the 4 corners and then interpolating all the values between them. After that the players and shots are positioned inside the maze via `set_all_player()`. This allows quick lookup of visible object based on the viewmatrix. Otherwise MIDImaze would have to iterate over all players many times.

The next step is building the render list itself inside `draw_maze_generate_renderlist()`. It does so by starting the front to the distance (up to 8 fields) from the player, rendering all walls and players/shots field by field.

It has a clever optimization to avoid calculating invisible walls and objects. Walls all have the same height, which means: any wall in front of another wall will hide it, same with players/shots. So MIDImaze uses that to have a simple `table_list[]` which stores all X-Left to X-Right positions of all walls which are already blocking the view of potential walls behind them. This list is kept sorted and also automatically merges two walls which partly overlap each other. This is possible, because this table is not used to render, but just to check if any part of the horizon is still visible. If at any point the horizon is fully covered by walls (which can happen immediately, if the player stands close in front of a single wall) as tested via `objecttable_check_view_fully_covered()`, then the render list code stops and leaves.

To render a wall `draw_mazes_set_wall()` is called. This takes the rotated positon of the wall right out of `viewmatrix_delta`. It then calles `draw_mazes_clip_wall()` to check if the wall is even visible within the viewport of the player (which is 90 degrees wide). If a wall is partly visible, the wall will be clipped to the minimum size necessary by calculating the intersection between the left or right side of the viewport with the wall. If the wall is visible, it's two sides are converted into the 2D perspective via `calc_yx_to_xh()` and then tried to be added to the render list via `objecttable_set_wall()`, which checks if the wall is actually visible (as tested via `objecttable_check_if_hidden()`). If the wall is visble, it is added via `to_draw_list()`.

Players and shots are added via `draw_mazes_set_object()`. This routine takes a maze field and iterates over all objects currently on it. The objects are rotated around the viewposition and stored in little array on the stack `objects[10]`. This array is then sorted by distance and the objects are added back to front from that list to our render list, including clipping via `objecttable_check_if_hidden()`. There are 32 different sizes for the player/shot avaiable. A shot is actually just a player without a face and at 1/4 of the size. The face also needs to be rotated in relation to the viewing direction of the viewport. The objects are scaled for perspective and added to the renderlist via `to_draw_list()`

### Draw the render list

First the sky and floor is drawn. This is just filling the top half of screen area with the sky color and the bottom half with the floor color.

The render list is processed back to front to deal with overlapping. A wall is drawn via `draw_wall()` and also receives to vertical lines on both sides via `draw_vline`. For players and shots `draw_shape()` will do the rendering.

#### `draw_wall`

A wall is just a filled rectangle around the horizon line. The tilted wall is then drawn a two filled triangles on top and below the rectangle. This is done by calculating the slope and then just draw shorter and shorter horizontal lines, similar to the Bresenham algorithm.

#### `draw_shape`

A shape is drawn at a specific X coordinate, also centered at the horizon line at a specific size (0-32; large to small) with an optional shadow on the floor. MIDImaze actually only has 24 different sized shapes, it uses less shapes at distance. The actual size varies between 2 and 53 lines. The shadow is calculated at launch from the ball shape and has 1/4 of it's height.

The shadow needs to be clipped to the window area and is only drawn when visible. The shape consists out of two images: a ball, which is colored in the player color and a frame and face via `blit_draw_shape_color` or `blit_draw_shape_bw`. Interesting thing: the `imageMask` which is a parameter to the blitting function points **behind** the image to be drawn! That works very well, but can be a bit confusing.

## Bugs in MIDImaze

MIDImaze has some bugs and some quirks, which I'll explain here.

  1. The most important one is the random MIDI timeout, which I fixed in Midi-Maze 2. You can find the fix by searching for  `BUGFIX_MIDI_TIMEOUT`. MIDImaze didn't use a 32-bit timer for a timeout and depending on the state of all machines in the network, an overflow could occur, which resulted in a negative number. But a negative number from the MIDI code is treated as an error code and the game stops.
  1. If a wall is fully horizontal with no slope, a division by zero occurs. This is not a problem on an Atari ST, which ignores the division by zero by default, but on other machines it is. Search for `BUGFIX_DIVISION_BY_ZERO` to see the fix.
  1. Players can have custom names. If you used a name with an 8-bit character (like German umlauts) MIDImaze will convert that into a negative number and treat it again as an error value. In this case the game simply can't start because of a MIDI error. Search for `BUGFIX_UMLAUTS_IN_NAMES` to see this fix.
  1. If seems late in the testing it was found that the crossed-out smiley sign was from bottom-left to top-right, while in the packaging it was from top-left to bottom-right. Instead of changing the 
`smileybuster_img` directly, the icon is flipped at launch via a little routine: `flip_crossedsmil_img`. Not visible in the game, but a quirk.
  1. Weird, but not a problem: the sine-table is 65 words, which are loaded from the 'MIDIMAZE.D8A' file and the remaing 191 words are calculated via flipping or mirroring these words. Not sure if saving 382 bytes is worth the performance hit at runtime…
