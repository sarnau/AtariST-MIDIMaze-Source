/************************************************************
 *** Send/Receive the MIDI package for setting up a game
 ************************************************************/
#include "globals.h"


/************************************************************
 *** send all player names to all MIDI slaves
 ************************************************************/
void midi_send_playernames(void) {
char *name;
int j;
int i;

    if(user_is_midicam) {
        /* The MIDIcam only monitors, but doesn't actively play. */
        /* It therefore doesn't send it's own name, but just echos all MIDI bytes */
        i = midicam_player_count;
        do {
            /* store and echo the name for player #i as a zero-terminated string */
            name = (char *)rsrc_object_array[RSCTREE_TEAM_DIALOG][rsc_playername_rscindices[i]].ob_spec;
            j = 0;
            do {
#if BUGFIX_UMLAUTS_IN_NAMES
                int ret = get_midi(MIDI_DEFAULT_TIMEOUT);
                if(ret < 0)
                    return;
                name[j] = ret;
#else
                name[j] = get_midi(MIDI_DEFAULT_TIMEOUT);
                if(name[j] < 0) /* BUG: this will fail the transmission with Umlauts in the name */
                    return;
#endif
                Bconout(MIDI, name[j]);
            } while(name[j++]);

            if(--i < 0) i = machines_online-1;

        } while(i != midicam_player_count);

    } else {

        /* an active player has to send it's own name to all other players */
        i = own_number;
        do {
            /* send the name for player #i as a zero-terminated string */
            name = (char *)rsrc_object_array[RSCTREE_TEAM_DIALOG][rsc_playername_rscindices[i]].ob_spec;
            j = 0;
            do {
                Bconout(MIDI, name[j]);
            } while(name[j++]);

            if(--i < 0) i = machines_online-1;

            /* receive the name for player #i as a zero-terminated string */
            name = (char *)rsrc_object_array[RSCTREE_TEAM_DIALOG][rsc_playername_rscindices[i]].ob_spec;
            j = 0;
            do {
#if BUGFIX_UMLAUTS_IN_NAMES
                int ret = get_midi(MIDI_DEFAULT_TIMEOUT);
                if(ret < 0)
                    return;
                name[j] = ret;
#else
                name[j] = get_midi(MIDI_DEFAULT_TIMEOUT);
                if(name[j] < 0) /* BUG: this will fail the transmission with Umlauts in the name */
                    return;
#endif
            } while(name[j++]);
        } while(i != own_number);
    }
}

/************************************************************
 *** Print the name of a specific player via VT52
 *** Used by the MIDIcam, announcing a shooter and for the winner
 ************************************************************/
void print_playername(int player) {
const char *name;
int i;

    /* Is this a human player (not a drone) */
    if(player < machines_online) {
        for(name = (char *)rsrc_object_array[RSCTREE_TEAM_DIALOG][rsc_playername_rscindices[player]].ob_spec; *name; name++)
            Bconout(CON, *name);

    } else if(player >= playerAndDroneCount) {
        /* This is always a MIDIcam */
        bconout_string("(Remote)");

    } else { /* this has to be a drone. They are always sorted by type. */
        player -= machines_online;
        for(i = 0; i < DRONE_TYPES; i++) {
            if(active_drones_by_type[i] > player) {
                static const char *droneNames[DRONE_TYPES] = { "Target #","Std.Drone ","Ninja #" };
                for(name = droneNames[i]; *name; name++)
                    Bconout(CON, *name);
                bconout_dec_number(player+1);
                break;
            }
            player -= active_drones_by_type[i];
        }
    }
}

/************************************************************
 *** int send_datas(void)
 ************************************************************/
int send_datas(void) {
int midiByte = 0;
int mazeSizeInBytes;
int i;

    /* remove all pending MIDI bytes */
    while(Bconstat(MIDI))
        Bconin(MIDI);

    /* package start */
    Bconout(MIDI, MIDI_SEND_DATA);
    midiByte = get_midi(MIDI_DEFAULT_TIMEOUT);
    if(midiByte < 0)
        return FAILURE;
    if(midiByte != MIDI_SEND_DATA)
        return FAILURE;

    /* send all player name */
    midi_send_playernames();
    /* size of the maze */
    Bconout(MIDI, maze_size);
    maze_size = get_midi(MIDI_DEFAULT_TIMEOUT);
    if(maze_size < 0)
        return FAILURE;

    /* several preferences */
    Bconout(MIDI, reload_time);
    reload_time = get_midi(MIDI_DEFAULT_TIMEOUT);
    if(reload_time < 0)
        return FAILURE;
    Bconout(MIDI, regen_time);
    regen_time = get_midi(MIDI_DEFAULT_TIMEOUT);
    if(regen_time < 0)
        return FAILURE;
    Bconout(MIDI, revive_time);
    revive_time = get_midi(MIDI_DEFAULT_TIMEOUT);
    if(revive_time < 0)
        return FAILURE;
    Bconout(MIDI, revive_lives);
    revive_lives = get_midi(MIDI_DEFAULT_TIMEOUT);
    if(revive_lives < 0)
        return FAILURE;

    /* number of drones per type */
    Bconout(MIDI, active_drones_by_type[0]);
    midiByte = get_midi(MIDI_DEFAULT_TIMEOUT);
    if(midiByte < 0)
        return FAILURE;
    Bconout(MIDI, active_drones_by_type[1]);
    midiByte = get_midi(MIDI_DEFAULT_TIMEOUT);
    if(midiByte < 0)
        return FAILURE;
    Bconout(MIDI, active_drones_by_type[2]);
    midiByte = get_midi(MIDI_DEFAULT_TIMEOUT);
    if(midiByte < 0)
        return FAILURE;

    /* transmit all maze bytes */
    /* Only after 50 bytes are transmitted, we expects data back. */
    /* This buffering increases performance. */
    mazeSizeInBytes = MAZE_MAX_SIZE*MAZE_MAX_SIZE;
    for(i = 0; i < mazeSizeInBytes; i++) {
        Bconout(MIDI, maze_datas[i]);
        if(i >= 50)
            midiByte = get_midi(MIDI_DEFAULT_TIMEOUT);
        if(midiByte < 0)
            return FAILURE;
    }
    for(i = 0; i < 50; i++)
        midiByte = get_midi(MIDI_DEFAULT_TIMEOUT);
    if(midiByte < 0)
        return FAILURE;

    /* Team configuration */
    Bconout(MIDI, team_flag);
    for(i = 0; i < PLAYER_MAX_COUNT; i++)
        Bconout(MIDI, player_data[i].ply_team);
    for(i = 0; i <= PLAYER_MAX_COUNT; i++)
        midiByte = get_midi(MIDI_DEFAULT_TIMEOUT);
    if(midiByte < 0)
        return FAILURE;
    Bconout(MIDI, friendly_fire);
    friendly_fire = get_midi(MIDI_DEFAULT_TIMEOUT);
    if(friendly_fire < 0)
        return FAILURE;

    return SUCCESS;
}

/************************************************************
 *** int receive_datas(void)
 ************************************************************/
int receive_datas(void) {
int midiByte = 0;
int mazeSizeInBytes;
int i;

    /* expect a package start */
    midiByte = get_midi(MIDI_DEFAULT_TIMEOUT);
    if(midiByte < 0)
        return FAILURE;
    if(midiByte != MIDI_SEND_DATA)
        return FAILURE;
    Bconout(MIDI, MIDI_SEND_DATA);

    /* forward the player names */
    midi_send_playernames();

    /* the maze size */
    maze_size = get_midi(MIDI_DEFAULT_TIMEOUT);
    if(maze_size < 0)
        return FAILURE;
    Bconout(MIDI, maze_size);

    /* several preferences */
    reload_time = get_midi(MIDI_DEFAULT_TIMEOUT);
    if(reload_time < 0)
        return FAILURE;
    Bconout(MIDI, reload_time);
    regen_time = get_midi(MIDI_DEFAULT_TIMEOUT);
    if(regen_time < 0)
        return FAILURE;
    Bconout(MIDI, regen_time);
    revive_time = get_midi(MIDI_DEFAULT_TIMEOUT);
    if(revive_time < 0)
        return FAILURE;
    Bconout(MIDI, revive_time);
    revive_lives = get_midi(MIDI_DEFAULT_TIMEOUT);
    if(revive_lives < 0)
        return FAILURE;
    Bconout(MIDI, revive_lives);

    /* number of drones per type */
    active_drones_by_type[0] = get_midi(MIDI_DEFAULT_TIMEOUT);
    if(active_drones_by_type[0] < 0)
        return FAILURE;
    Bconout(MIDI, active_drones_by_type[0]);
    active_drones_by_type[1] = get_midi(MIDI_DEFAULT_TIMEOUT);
    if(active_drones_by_type[1] < 0)
        return FAILURE;
    Bconout(MIDI, active_drones_by_type[1]);
    active_drones_by_type[2] = get_midi(MIDI_DEFAULT_TIMEOUT);
    if(active_drones_by_type[2] < 0)
        return FAILURE;
    Bconout(MIDI, active_drones_by_type[2]);

    /* transmit all maze bytes */
    mazeSizeInBytes = MAZE_MAX_SIZE*MAZE_MAX_SIZE;
    for(i = 0; i < mazeSizeInBytes; i++) {
        midiByte = get_midi(MIDI_DEFAULT_TIMEOUT);
        if(midiByte < 0)
            return FAILURE;
        Bconout(MIDI, midiByte);
        maze_datas[i] = midiByte;
    }

    /* Team configuration */
    team_flag = get_midi(MIDI_DEFAULT_TIMEOUT);
    if(team_flag < 0)
        return FAILURE;
    Bconout(MIDI, team_flag);
    for(i = 0; i < PLAYER_MAX_COUNT; i++) {
        player_data[i].ply_team = get_midi(MIDI_DEFAULT_TIMEOUT);
        if(player_data[i].ply_team < 0)
            return FAILURE;
        Bconout(MIDI, player_data[i].ply_team);
    }
    friendly_fire = get_midi(MIDI_DEFAULT_TIMEOUT);
    if(friendly_fire < 0)
        return FAILURE;
    Bconout(MIDI, friendly_fire);

    return SUCCESS;
}
