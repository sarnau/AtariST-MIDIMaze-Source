# SOURCES

This is the original source code for MIDImaze. Split into several source and in Atari-tradition with all-caps filenames. I've heavily commented it, but didn't change the code at all. I have however added the option to add a few fixes for the original MIDImaze. The #defines for that can be found in `GLOBALS.H`. I would start with that file to get an overview.

These are the sources, converted to ANSI-C, however with the assembly code as a comment in front of it. They are in the same order as they are in the linked MIDImaze binary.


## `GLOBALS.H`
Contains all global variables, #defines and data types shared between several sources. MIDImaze might have had several header files, but this keeps it simple.

## `LOWMEM.C`
Read the VBL Clock. Used as a timer for the MIDI timeout. This contains the reason for the infamous MIDI-Ring Timout bug!

## `READMIDI.C`
Read a byte vis the BIOS from MIDI with a timeout.

## `MAIN.C`
`main()` function. Basic Atari setup and then calling `setup_game()`

## `RUNGAME.C`
Graphics initialization, data table setup, load and show the main screen and the copy protection check call. Calls `dispatch()` to enter the main game mode selection loop.

## `MAKE_DRAWLIST.C`
Create a list of 3D render object (walls, player and shots) to be drawn based on a certian position and direction within the maze.

## `SCREEN.C`
Screen double-buffering setup and switching code.

## `SOUND.C`
Sound initiazation for the shot and hit sound.

## `DISPATCH.C`
Loop to select between Master/Solo and Slave/MIDIcam.

## `ENDSHAPES.C`
Setup for the shapes shown at the end of the game: tongue and blink/eyebrows.

## `MAINGAME.C`
The main game loop, used by all play modes.

## `MIDICAM.C`
Support code for the MIDIcam: key commands, finding a player to follow, etc.


## `POPCHART.C`
The pop chart (the table of the kills)

## `MORESHAPES.C`
Additional shapes: the images for the note board and the crossed shape for the pop chart.

## `NOTEBOARD.C`
The score board for individual or team play, like notes in sheet music.

## `SAVESCREEN.C`
Saving/restoring the screen in combination of AES dialogs.

## `MISCOUTPUT.C`
Text output code via BIOS. Used for printing the player name and strings during a kill or at the end of the game.

## `DRAW2D.C`
Draws the 2D map of the maze. Also can draw the players.

## `DRONE.C`
Contains three functions. `calc_drone_angle_table()` was originally localed behind the `run_game()` and before `load_datas()`, but it made more sense to move it to the rest of the drone code. Sorry for that.

Setup is called at the start of the game to find an enemey for every drone (yes, there is only one permanent enemy per drone!). It considers teams or single-player mode.

Action is called in the game loop and it generates joystick input from the drones directly into `player_joy_table[]`. This is were all the intelligence or lack of is located.

    - void calc_drone_angle_table(void)
    - void drone_setup(int humanPlayers)
    - void drone_action(int player)

## `AESINIT.C`
AES initialization. Contains the RSC tree, which is compiled into MIDImaze.

## `MASTER.C`
Main loop for the master to control the menus, load a maze and start a game.

## `PREFDIALOG.C`
The play game dialog including the team dialog. Controlled by the master.

## `STRING.C`
Routine to copy a C-string.

## `SLAVE.C`
Main loop for the slave. All features are controlled by the Master, but the slave supports key commands to switch to other modes.

## `MIDICOMM.C`
MIDI communication to send/receive the game data, including the player names.

## `RSCMISC.C`
Two routines to show a dialog (with or without a button – the latter one is used for slaves)

## `CONTERM.C`
Set/reset the conterm flag. Only used for the slave to receive the modifier status in Bconin(CON)

## `LOAD_MAZE.C`
Load the *.MAZ file.

## `MAZE_OBJECT.C`
Store information in the `maze_datas[]` structure (the maze file). Also code to manage the linked lists of players/shots located in the maze.

## `MAKE_RENDERLIST.C`
Generate the 3D renderlist for walls and player/shots.

## `RND.C`
Random generator with a shared seed between all players. This guarantees that all players can generate matching random numbers, e.g. where will a new player appear. This avoids the computer to be out-of-sync or transferring the information during the game.

## `MAZE_SET_OBJ.C`
Positions all players/shots in a specific field to be added to the 3D render list.

## `SETUP_PLAYERS.C`
Initialize all player variables and find random positions in the maze for them.

## `JOYSTICKMOUSE.C`
All code related to joystick or mouse control. A mouse input is converted into joystick data for simplicity.

`ask_mouse()` was originally behind `SCREEN.C` and before `MISCOUTPUT.C`. I put it here to have all joystick/mouse related code together.

## `FASTMATH.C`
Contains sin/cos functions, which are used by the only exported function: `rotate2d()`, which is used to do 2D rotation with a specific angle. All is done in 16-bit integer math.

## `HAPPYINDICATOR.C`
The happy indicator above the maze view. Health state of the player.

## `DRAW3D.C`
Renders the 3D draw list. Draws the walls. Also has the math for the perspective distortion.

## `DRAWWALLS.C`
Adds walls to the 3D draw list. Include complex optimizations to only draw visible walls and also clips them to the minimal width visible.

## `DRAWSHAPE.C`
Initializes and draws shapes (players and shots). A shot is actually a player from behind (hence: it doesn't have a face).

## `GAMELOGIC.C`
The main game logic: takes the joystick input for a player and moves the player and it's shot, manages collisions with walls other player or shots. Also updates the score and stops the game loop, if a winner has been found.
