# MIDI Communication

After launch a 0x00 byte is send onto the ring. If there is a reply, the ring is complete and the computer becomes a master. If there is no reply, it will default to slave. This means: the last computer who closes the ring, will be the master.

The first non-zero byte a slave receives triggers the start of the game. These are the comments recognized by MIDImaze. If a slave receives a command, it is immediately forwarded to all other slaves (and at some point back to the master).

## `MIDI_COUNT_PLAYERS` (0x80)
The master sends this, followed by a 0x00. Each slaves who receives it, increments the number and sends it further on. Once the master receives the byte back, it is known how many machines are online in the ring. The master then sends the final number around once more, so that all slaves receive it as well.
 
## `MIDI_RESET_SCORE` (0x81)
A single-byte command, which resets the score table for all players.

## `MIDI_TERMINATE_GAME` (0x82)
Triggered as a joystick command, when the master presses the Escape key. This will trigger an alert on the master and stop the game. The master can then pick to continue (`MIDI_START_GAME`) or terminate the game (`MIDI_TERMINATE_GAME`).

## `MIDI_SEND_DATA` (0x83)
After `MIDI_START_GAME` this package contains all shared data:

  1. The player names, zero terminated. Each player sends their name first and then excepts all other players names.
  1. A byte with the `maze_size`
  1. A byte with the `reload_time`
  1. A byte with the `regen_time`
  1. A byte with the `revive_time`
  1. A byte with the `revive_lives`
  1. 3 bytes with the number of different drones
  1. 4096 (64x64) bytes with the maze data
  1. A byte with the `team_flag`
  1. 16 bytes with the team group for each player
  1. A byte if `friendly_fire` is active
  1. Two bytes (high/low) for `_random_seed` to initialize the random generator

As you can see, this could be heavily optimized (= faster) and should potentially also have a checksum.

All flags could be packed into a single byte (they are all 1 or 2 bits large). The player teams could be compressed into 4 bytes (4 teams = 2 bits per player), the number of drones into 2 bytes (4 bits per type). On the plus side: a master could change e.g. the time values and all slaves would pick that up.

And last - but also the most important one - the maze data itself: we only need to transmit the walls, which can only sit at certain coordinates and then one bit per wall is enough. For a 14x14 maze, that would be (14-2)/2 = 6 bits horizontally to the square (for the verical walls) = 36 bits or 5 bytes, instead of 4096! Even for a 62x62 maze, that only would be 113 bytes. I don't think further RLE compression or so would do anything meaningful, because the speed of MIDI is fast enough – even without any optimizations it takes less than 2 seconds to transfer all the data. But I think sending less data would make it more stable at the start.

Anyway, MIDImaze doesn't compress any of that.


## `MIDI_START_GAME` (0x84)
Start the game (followed by `MIDI_SEND_DATA`), also used to continue a gave after the first `MIDI_TERMINATE_GAME` request.

## `MIDI_ABOUT` (0x85)
A single-byte command, which shows the about box to all players. Only the master has an enabled OK button.

## `MIDI_NAME_DIALOG` (0x86)
Allows all players to enter their name. It happens in 4 phases:

  1. just like `MIDI_COUNT_PLAYERS` the number of the players is counted.
  2. After that the master and all slaves have a chance to modify their name via the dialog.
  3. Each then expects a 0x00 byte to be send for every player online. Because the dialog is modal, this can only happen, after the player confirmed the dialog. This means: all players are waiting till the last one confirms the dialog and sends the pending bytes around.
  4. Now each player sends their name (plus a zero byte at the end) first and then expects all names from the other players. Because of the ring nature, the last name received is the name of the local player again. This completes the name transmission.


## Joystick data

During the game each player sends their joystick daa onto the ring and receives the data from all other players. Each joystick data is a single byte in the standard joystick bitmask (left,right, up,down,button for the first 5 bits). Only `MIDI_TERMINATE_GAME` is recognized during gameplay.
