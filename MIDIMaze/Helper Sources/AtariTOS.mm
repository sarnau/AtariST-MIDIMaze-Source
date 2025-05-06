/*
 *  AtariTOS.mm
 *  Implement Atari specific BIOS,XBIOS,GEMDOS,AES and VDI functions - just enough to let MIDImaze run.
 *
 *  Created by Markus Fritze on 9/1/10.
 *
 */

#import "AppDelegate.h"
extern "C" {
#include "globals.h"
#include "rstest.h"
}
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// convert an Atari bitmap into our image
static void        plainConvert(const unsigned short *atariScreenBufP, unsigned char *rgbScreenBuf);

#define SIMULATES_MIDI 1 // This simulates a second player with the name "Network #1"

#if SIMULATES_MIDI
#define MIDIBUFFERSIZE 8192
static int gMIDIBufferInCount;
static unsigned char gMIDIBufferIn[MIDIBUFFERSIZE];
static int gMIDIBufferOutCount;
static unsigned char gMIDIBufferOut[MIDIBUFFERSIZE];
static int gMIDIState = -1;
static int gMIDIStateIndex = 0;
#endif

static int VT52cursorPosRow = 0;
static int VT52cursorPosColumn = 0;
static int VT52backgroundcolor = 0;
static int VT52textcolor = 0;

// Render an character onto the screen at current VT52 configuration (color, position)
static void renderChar(unsigned char);

#pragma mark -
#pragma mark BIOS

// VT52 is used by MIDImaze for text output right onto the screen.
// We render that directly into the bitmap for the same look.
void    Bconout(int dev, int c)
{
    static bool foundEsc = false;
    static int escFollowChars = 0;
    static char VT52code = 0;
    static char VT52params[2];

    if(dev == CON) {
        if(escFollowChars) {
            --escFollowChars;
            VT52params[escFollowChars] = c;
            if(escFollowChars == 0) {
                switch(VT52code) {
                case 'b':
                        VT52textcolor = VT52params[0];
//                        printf("Bconout(%d,Set Foreground color %d)\n",dev,VT52params[0]);
                        break;
                case 'c':
                        VT52backgroundcolor = VT52params[0];
//                        printf("Bconout(%d,Set Background color %d)\n",dev,VT52params[0]);
                        break;
                case 'Y':
                        VT52cursorPosColumn = VT52params[0]-32;
                        VT52cursorPosRow = VT52params[1]-32;
//                        printf("Bconout(%d,Set Pos %d,%d)\n",dev,VT52params[0]-32,VT52params[1]-32);
                        break;
                }
            }
            return;
        }
        if(foundEsc) {
            VT52code = c;
            if(c == 'b' || c == 'c') {
                escFollowChars = 1;
            } else if(c == 'Y') {
                escFollowChars = 2;
            }
            foundEsc = false;
            return;
        }
        switch(c) {
        case 7:
//                printf("Bconout(%d,BEL)\n",dev);
                NSBeep();
                break;
        case 27:foundEsc = true; return;
        default:
//                printf("Bconout(%d,'%c')\n",dev,c);
                renderChar(c);
                break;
        }
        foundEsc = false;
    } else if (dev == MIDI) {
#if SIMULATES_MIDI
//        printf("Bconout(MIDI,%#2.2x)\n",c & 0xff);
        if(gMIDIBufferInCount < MIDIBUFFERSIZE)
            gMIDIBufferIn[gMIDIBufferInCount++] = c;
#endif
    }
}

#if SIMULATES_MIDI
static void simulateMIDI()
{
    // data to process?
    while(gMIDIBufferInCount > 0) {
        static int machines_online = -1;
        static const char *name = "Network #1"; // Dummy name for our test player
        static bool skipOwnName = false;
        static size_t skipCount = 0;
        unsigned char c = gMIDIBufferIn[0];
        memmove(gMIDIBufferIn, gMIDIBufferIn+1, gMIDIBufferInCount);
        gMIDIBufferInCount--;
        if(gMIDIState > 0 && !(c >= MIDI_COUNT_PLAYERS && c <= MIDI_NAME_DIALOG)) {
            ++gMIDIStateIndex;
            switch(gMIDIState) {
            case 1: // game ongoing: joystick data. Send our first, then receive all others
                    if(gMIDIStateIndex == 1) {
                        gMIDIBufferOut[gMIDIBufferOutCount++] = JOYSTICK_LEFT|JOYSTICK_BUTTON;
                    } else { // at the end of the round, filter out our own package
                        skipCount = 1;
                        gMIDIStateIndex = 0;
                    }
                    break;
            case MIDI_NAME_DIALOG:
                    static bool nameSend = false;
                    if(gMIDIStateIndex == 1) {
                        c = c + 1;
                    } else if(gMIDIStateIndex == 2) {
                        machines_online = c;
                        nameSend = false;
                    } else if(gMIDIStateIndex <= 2 + machines_online) {
                        // forward the 0x00 bytes directly
                    } else {
                        if(!nameSend) {
                            int j = 0;
                            do {
                                gMIDIBufferOut[gMIDIBufferOutCount++] = name[j];
                            } while(name[j++]);
                            skipOwnName = true;
                            nameSend = true;
                        }
                    }
                    printf("MIDI_NAME_DIALOG => %#2.2x (%d)\n",c,gMIDIStateIndex);
                    break;
            case MIDI_COUNT_PLAYERS:
                    switch(gMIDIStateIndex) {
                    case 1: c = c + 1; break;
                    case 2: machines_online = c; c = MIDI_COUNT_PLAYERS; gMIDIState = -1; break;
                    }
//                        printf("MIDI_COUNT_PLAYERS => %#2.2x\n",c);
                    break;
            case MIDI_SEND_DATA:
                    if(gMIDIStateIndex == 1) {
                        int j = 0;
                        do {
                            gMIDIBufferOut[gMIDIBufferOutCount++] = name[j];
                        } while(name[j++]);
                        skipOwnName = true;
                    }
//                        printf("MIDI_SEND_DATA => #%d %#2.2x\n",gMIDIStateIndex,c);
                    if(gMIDIStateIndex == 50+4096) { // all data received?
                        gMIDIState = 1; // game is now ongoing
                        gMIDIStateIndex =  0;
                    }
                    break;
            default: break;
            }
        } else {
            switch(c) {
            case MIDI_COUNT_PLAYERS:
                gMIDIState = MIDI_COUNT_PLAYERS;
                gMIDIStateIndex = 0;
                break;
            case MIDI_SEND_DATA:
                gMIDIState = MIDI_SEND_DATA;
                gMIDIStateIndex = 0;
                break;
            case MIDI_NAME_DIALOG:
                gMIDIState = MIDI_NAME_DIALOG;
                gMIDIStateIndex = 0;
                break;
            }
        }
        if(skipCount > 0) {
            --skipCount;
            skipOwnName = false;
        } else {
            gMIDIBufferOut[gMIDIBufferOutCount++] = c;
            if(c == 0x00 && skipOwnName) {
                skipCount = strlen(name) + 1;
            }
        }
    }
}
#endif

long    Bconin( int dev )
{
    if(dev == CON) {
        return gBIOSKeyCode;
    } else if (dev == MIDI) {
#if SIMULATES_MIDI
        simulateMIDI();
        if(gMIDIBufferOutCount > 0) {
            unsigned char c = gMIDIBufferOut[0];
            memmove(gMIDIBufferOut, gMIDIBufferOut+1, gMIDIBufferOutCount);
            gMIDIBufferOutCount--;
//            printf("Bconin(MIDI) <= %#2.2x %c\n", c, c);
            return c;
        } else {
//            printf("Bconin(MIDI) <= TIMEOUT\n");
        }
#endif
    }
    return 0;
}

int     Bconstat( int dev )
{
    if(dev == CON) {
        usleep(1000*20);    // 20ms delay, slow the game down to roughly match an Atari ST
        if(gBIOSKeyCode)
            return 1;
    } else if (dev == MIDI) {
//        printf("Bconstat(MIDI)\n");
#if SIMULATES_MIDI
        simulateMIDI();
        return gMIDIBufferOutCount > 0;
#endif
    }
    return 0; // no key pressed or byte received
}

#pragma mark -
#pragma mark GEMDOS

int Fopen(const char *filename, int mode)
{
//    printf("Fopen(%s,%d)\n",filename, mode);
    // Load from the application bundle
    if(!strcmp(filename, "MIDIMAZE.D8A") || !strcmp(filename, "MIDIMAZE.MAZ")) {
        filename = [[NSBundle.mainBundle resourcePath] stringByAppendingPathComponent:@(filename)].fileSystemRepresentation;
    }
    // we only support read-only anyway
    mode = O_RDONLY;
	return open(filename, mode);
}

long    Fread( int handle, long count, void *buf )
{
//    printf("Fread(%d,%ld,%p)\n",handle, count, buf);
	long ret = read(handle, buf, count);
	// This part needs byte-swapping
	if(count == 66742) {
        unsigned short *wbuf = (unsigned short *)buf;
        for(int i=0; i<66742/2; ++i)
            wbuf[i] = INTELSWAP16(wbuf[i]);
    }
	return ret;
}

int Fclose(int handle)
{
//    printf("Fclose(%d)\n",handle);
	return close(handle);
}

// only used to build a path for maze files, which is ignored by macOS
int Dgetdrv(void)
{
    return 0; // drive A
}

int Dgetpath(char *path, int drive)
{
    path[0] = '/';
    path[1] = '/';
    return 0;
}

#pragma mark -
#pragma mark XBIOS

long Random(void)
{
#if 1 // repeatable random numbers (for testing)
    static long _random_seed = 43;
    // This code is identical to the Atari TOS
    _random_seed = _random_seed*3141592621+1;
    return (_random_seed >> 8) & 0xffffff;
#else
	return rand();
#endif
}

// We just play the sampled original sounds, instead of emulating the sound chip
void    *Dosound( void *buf )
{
//    printf("Dosound(%p)\n",buf);
    if(buf == sound_shot_ptr) {
        static NSSound *player = nil;
        static dispatch_once_t once;
        dispatch_once(&once, ^{
            NSString *filename = [NSBundle.mainBundle pathForResource:@"MIDIMaze Shot" ofType:@"mp3"];
            if(filename) {
                player = [[NSSound alloc] initWithContentsOfFile:filename byReference:NO];
            }
        });
        [player stop];
        [player play];
    } else if(buf == sound_hit_ptr) {
        static NSSound *player = nil;
        static dispatch_once_t once;
        dispatch_once(&once, ^{
            NSString *filename = [NSBundle.mainBundle pathForResource:@"MIDIMaze Hit" ofType:@"mp3"];
            if(filename) {
                player = [[NSSound alloc] initWithContentsOfFile:filename byReference:NO];
            }
        });
        [player stop];
        [player play];
    }
    return NULL;
}

// Used to save/restore the sound chip registers
// Not needed on macOS
char    Giaccess( char data, int regno )
{
//	printf("Giaccess(%d,%d)\n",regno,data);
	return 0;
}

// This is the physical screen, similar to the on in the Atari
static char physBuffer[32000];
static void *phyScreen = physBuffer;
static void *logScreen;

static void updateScreenView()
{
    // convert the Atari bitmap at the Physscreen() into an RGB bitmap
    // and trigger AppKit to update it on the main thread.
    // OpenGL, Metal are all a faster solution, but for MIDImaze it really doesn't matter.
    if(gImageRep.bitmapData) {
        plainConvert((unsigned short*)physBuffer, gImageRep.bitmapData);
        [NSOperationQueue.mainQueue addOperationWithBlock:^(){
            gAppDelegate.imageView.needsDisplay = YES;
        }];
    }
}

unsigned short physPalette[16];

int     Setcolor( int colornum, int color )
{
//    printf("Setcolor(%d,%#x)\n",colornum,color);
    physPalette[colornum] = color;
    updateScreenView(); // necessary, because when the player is shot, the screen flickers
	return 0;
}

void    Setpalette( void *pallptr )
{
//    printf("Setpalette(%p)\n", pallptr);
    for(int colornum=0; colornum<16; ++colornum)
        physPalette[colornum] = ((unsigned short *)pallptr)[colornum];
    updateScreenView();
}

void Setscreen(void *laddr, void *paddr, int rez)
{
//    NSString *aStr = [NSString stringWithFormat:@"%p", laddr];
//    if(laddr == physBuffer)
//        aStr = @"physBuffer";
//    NSString *bStr = [NSString stringWithFormat:@"%p", paddr];
//    if(paddr == physBuffer)
//        bStr = @"physBuffer";
//    NSLog(@"Setscreen(%@,%@,%d)",aStr,bStr,rez);
	if(laddr != (void*)-1)
		logScreen = laddr;
	if(paddr != (void*)-1) {
		phyScreen = paddr;
    }
    updateScreenView();
}

// MIDImaze uses Vsync for delay loops.
// It assumes (being a 'good' US application): 60Hz in color and 70Hz in b/w. In 50Hz
// the timing is a bit off, but nobody really noticed it. 
void Vsync(void)
{
    updateScreenView();
    usleep(1000*1000/50); // Delay down to ~50Hz - otherwise MIDImaze would be _way_ too fast.
}

int Getrez()
{
//	printf("Getrez()\n");
//    return 2; // bw (which does work, try it out by uncommenting this line!)
	return 0; // color
}

void *Physbase()
{
//    printf("Physbase()\n");
	return phyScreen;
}

#pragma mark -
#pragma mark AES

// Only implement what is necessary for MIDImaze
short evnt_multi(short flags,short bclk,short bmsk,short bst,short m1flags,short m1x,short m1y,short m1w,short m1h,short m2flags,short m2x,short m2y,short m2w,short m2h,short *mepbuff,short tlc,short thc,short *pmx,short *pmy,short *pmb,short *pks,short *pkr,short *pbr)
{
//    printf("evnt_multi()\n");
    if(gSelectedMenuItem != MMSelectedMenuItemNone) {
        mepbuff[0] = MN_SELECTED;
        switch(gSelectedMenuItem) {
        case MMSelectedMenuItemNone: break;
        case MMSelectedMenuItemAbout: mepbuff[3] = TITLE_ABOUT; mepbuff[4] = ABOUT; break;
        case MMSelectedMenuItemLoad:  mepbuff[3] = TITLE_MAZE; mepbuff[4] = MAZE_LOAD; break;
        case MMSelectedMenuItemReset: mepbuff[3] = TITLE_MAZE; mepbuff[4] = MAZE_RESET_SCORE; break;
        case MMSelectedMenuItemPlay:  mepbuff[3] = TITLE_MAZE; mepbuff[4] = MAZE_PLAY; break;
        case MMSelectedMenuItemQuit:  mepbuff[3] = TITLE_MAZE; mepbuff[4] = MAZE_QUIT; break;
        case MMSelectedMenuItemNames: mepbuff[3] = TITLE_MAZE; mepbuff[4] = MAZE_SET_NAMES; break;
        }
        gSelectedMenuItem = MMSelectedMenuItemNone;
        return MU_MESAG;
    } else {
        mepbuff[0] = 0;
    }
    // Keycode with alt (Option) pressed
    int keyCode = gAltKeyCode;
    if(keyCode) {
        gAltKeyCode = 0;
        *pks = (keyCode >> 16) & 0xffff;
        *pkr = keyCode & 0xffff;
        return MU_KEYBD;
    }
    return 0;  // nothing to do
}

short form_dial(short flag,short littlx,short littly,short littlw,short littlh,short bigx,short bigy,short bigw,short bigh)
{
    return 0;
}

// Helper code to connect the Atari RSC indices with macOS tags in the views
__kindof NSView *viewForTagSub(NSView *mainview, NSInteger tag)
{
    // found the tag? => done
    if(mainview.tag == tag)
        return mainview;

    // iterate over all subviews; return view, if found
    for(NSView *view in mainview.subviews) {
        __kindof NSView *retView = viewForTagSub(view, tag);
        if(retView)
            return retView;
    }
    // view with tag not found
    return nil;
}

__kindof NSView *viewForTag(NSWindow *window, NSInteger tag)
{
    return viewForTagSub(window.contentView, tag);
}

// This uses macOS dialogs for the Atari code.
// It requires some hacking to connect the RSC indices with the tags
// and update the controls both ways. But this is not designed to be
// pretty, but to just get it to work.
short form_do(OBJECT *form,short start)
{
    int treeIndex = -1;
    for(int i=0; i<NUM_TREE; ++i) {
        if(form == rsrc_object_array[i]) {
            treeIndex = i;
            break;
        }
    }
//    printf("form_do(rsrc_object_array[%d], %d)\n", treeIndex, start);
    switch(treeIndex) {
    case RSCTREE_PLAY_DIALOG: {
            __block NSModalResponse ret;
            dispatch_sync(dispatch_get_main_queue(), ^{
                NSArray<NSArray <NSNumber *> *> *selectedRsc = @[@[@RELOAD_FAST,@RELOAD_SLOW],@[@REGEN_SLOW,@REGEN_FAST],@[@REVIVE_SLOW,@REVIVE_FAST],@[@PREF_1LIFE,@PREF_2LIVES,@PREF_3LIVES],@[@PREF_SINGLES,@PREF_TEAMS]];
                do {
                    NSTextField *tf = viewForTag(gAppDelegate.playDialog,PREF_TITLE);
                    // machines online
                    tf.stringValue = @((char*)form[PREF_TITLE].ob_spec);
                    for(NSArray <NSNumber *> *radioList in selectedRsc) {
                        for(NSNumber *rsrcID in radioList) {
                            NSButton *button = viewForTag(gAppDelegate.playDialog,rsrcID.integerValue);
                            button.state = (form[rsrcID.integerValue].ob_state & SELECTED) == SELECTED;
                        }
                    }
                    // Numbers of drones
                    for(NSNumber *rsrcID in @[@DUMB_VAL,@PLAIN_VAL,@NINJA_VAL]) {
                        NSButton *button = viewForTag(gAppDelegate.playDialog,rsrcID.integerValue);
                        button.title = @((char*)form[rsrcID.integerValue].ob_spec);
                    }
                    ret = [NSApp runModalForWindow:gAppDelegate.playDialog];
                    for(NSArray <NSNumber *> *radioList in selectedRsc) {
                        for(NSNumber *rsrcID in radioList) {
                            NSButton *button = viewForTag(gAppDelegate.playDialog,rsrcID.integerValue);
                            form[rsrcID.integerValue].ob_state &= ~SELECTED;
                            if(button.state) {
                                if(![radioList containsObject:@(ret)] || ret == rsrcID.integerValue)
                                    form[rsrcID.integerValue].ob_state |= SELECTED;
                            }
                        }
                    }
                    if(ret == PREF_TEAMS) break; // Team
                    Boolean foundIt = false;
                    for(NSArray <NSNumber *> *radioList in selectedRsc) {
                        if([radioList containsObject:@(ret)]) {
                            foundIt = true;
                            break;
                        }
                    }
                    if(!foundIt)
                        break;
                } while(1);
                if(ret == PREF_NAH || ret == PREF_OK)
                    [gAppDelegate.playDialog orderOut:gAppDelegate];
            });
            return (int)ret;
    }
    case RSCTREE_TEAM_DIALOG: {
            __block NSModalResponse ret;
            dispatch_sync(dispatch_get_main_queue(), ^{
                do {
                    for(int i=0;i<PLAYER_MAX_COUNT;++i) {
                        NSTextField *field = viewForTag(gAppDelegate.teamDialog,rsc_playername_rscindices[i]);
                        field.stringValue = @((char*)form[rsc_playername_rscindices[i]].ob_spec);
                        for(int j=0; j<PLAYER_MAX_TEAMS; ++j) {
                            int kk = team_group_rscindices[i][j];
                            NSButton *button = viewForTag(gAppDelegate.teamDialog,kk);
                            button.state = (form[kk].ob_state & SELECTED) == SELECTED;
                        }
                    }
                    NSButton *button = viewForTag(gAppDelegate.teamDialog,FRIENDLY_FIRE);
                    button.state = (form[FRIENDLY_FIRE].ob_state & SELECTED) == SELECTED;
                    ret = [NSApp runModalForWindow:gAppDelegate.teamDialog];
                    form[FRIENDLY_FIRE].ob_state &= ~SELECTED;
                    if(button.state)
                        form[FRIENDLY_FIRE].ob_state |= SELECTED;
                    for(int i=0;i<PLAYER_MAX_COUNT;++i) {
                        for(int j=0; j<PLAYER_MAX_TEAMS; ++j) {
                            int kk = team_group_rscindices[i][j];
                            NSButton *button = viewForTag(gAppDelegate.teamDialog,kk);
                            form[kk].ob_state &= ~SELECTED;
                            if(button.state) {
                                bool retInGroup = false;
                                for(int l=0; l<PLAYER_MAX_TEAMS; ++l) {
                                    if(ret == team_group_rscindices[i][l]) {
                                        retInGroup = true;
                                        break;
                                    }
                                }
                                if(!retInGroup || ret == kk)
                                    form[kk].ob_state |= SELECTED;
                            }
                        }
                    }
                } while(ret != TEAM_OK);
                [gAppDelegate.teamDialog orderOut:gAppDelegate];
            });
            break;
    }
    case RSCTREE_SET_NAME: {
            __block NSModalResponse ret;
            dispatch_sync(dispatch_get_main_queue(), ^{
                gAppDelegate.nameDialogName.stringValue = @(rs_tedinfo[0].te_ptext);
                ret = [NSApp runModalForWindow:gAppDelegate.nameDialog];
                NSString *str = gAppDelegate.nameDialogName.stringValue;
                for(NSUInteger i=0; i<str.length && i<10; ++i) {
                    (rs_tedinfo[0].te_ptext)[i] = [str characterAtIndex:i];
                }
            });
            return (int)ret;
    }
    case RSCTREE_ABOUT: {
            dispatch_sync(dispatch_get_main_queue(), ^{
                [NSApp runModalForWindow:gAppDelegate.aboutDialog];
            });
            return ABOUT_WOW;
    }
    default: break;
    }
    return 0;
}

short menu_tnormal(OBJECT *tree,short titlenumm,short normalit)
{
    return 0;
}

// Convert Atari ST alerts into a macOS alert. Again: just enough to get them to work!
short form_alert(short defbut,const char *astring)
{
//    printf("form_alert(%d, \"%s\")\n", defbut, astring);
    NSString *alertStr = @(astring);
    __block NSInteger ret;
    dispatch_sync(dispatch_get_main_queue(), ^{
        NSArray<NSString *> *alertStrings = [alertStr componentsSeparatedByString:@"]["];
        
        NSAlert *alert = [[NSAlert alloc] init];
        switch([alertStrings[0] characterAtIndex:1]) {
        case '1': alert.alertStyle = NSAlertStyleWarning; break;
        case '2': alert.alertStyle = NSAlertStyleInformational; break;
        case '3': alert.alertStyle = NSAlertStyleCritical; break;
        case '0':
        default: break;
        }
        NSString *str = [alertStrings[1] stringByReplacingOccurrencesOfString:@"|" withString:@"\n"];
        alert.messageText = str;
        for(NSString *button in [alertStrings[2] componentsSeparatedByString:@"|"]) {
            NSString *buttonStr = button;
            if([buttonStr hasSuffix:@"]"])
                buttonStr = [buttonStr substringWithRange:NSMakeRange(0, buttonStr.length-1)];
            [alert addButtonWithTitle:buttonStr];
        }
        ret = [alert runModal];
    });
    gAltKeyCode = 0;
    return (int)ret;
}

short form_center(OBJECT *tree,short *pcx,short *pcy,short *pcw,short *pch)
{
    int treeIndex = -1;
    for(int i=0; i<NUM_TREE; ++i) {
        if(tree == rsrc_object_array[i]) {
            treeIndex = i;
            break;
        }
    }
//    printf("form_center(rsrc_object_array[%d])\n", treeIndex);
    return 0;
}

// File selector, this also _loads_ the maze, because the Atari ST code can't deal with
// longer pathnames, etc. And to avoid crashes we just load it directly and bypass the Atari code.
short fsel_input(char *pipath,char *pisel,short *pbutton)
{
//    printf("fsel_input(\"%s\", \"%s\")\n", pipath, pisel);
    __block NSInteger ret;
    __block NSURL *fileURL;
    dispatch_sync(dispatch_get_main_queue(), ^{
        NSOpenPanel *open = NSOpenPanel.openPanel;
        open.allowedFileTypes = @[@"MAZ"];
        ret = [open runModal];
        fileURL = open.URL;
    });
    if(ret != NSModalResponseOK) {
        *pbutton = FALSE;
        return FALSE;
    }
    *pbutton = FALSE; // avoid the path code, which is Atari specific
    graf_mouse(HOURGLASS, 0);
    maze_loaded_flag = LoadMaze(fileURL.fileSystemRepresentation, fileURL.lastPathComponent.fileSystemRepresentation) >= 0;
    graf_mouse(ARROW, 0);
    return TRUE;
}

short graf_mouse(short m_number,const MFORM *m_addr)
{
    switch(m_number) {
// never hide it, it is not necessary in macOS and blocks the menu bar usage
//    case M_ON: [NSCursor unhide]; break;
//    case M_OFF: [NSCursor hide]; break;
//    case ARROW: [NSCursor arrowCursor]; break;
//    case HOURGLASS: /* does not exist in macOS */ break;
    default: break; 
    }
    return 0;
}

short menu_bar(OBJECT *tree,short showit)
{
    return 0;
}

// Redraw certain objects in a RSC tree. This is used for dialogs without buttons,
// which are the types used for slaves and MIDIcams.
short objc_draw(OBJECT *tree,short drawob,short depth,short xc,short yc,short wc,short hc)
{
    int treeIndex = -1;
    for(int i=0; i<NUM_TREE; ++i) {
        if(tree == rsrc_object_array[i]) {
            treeIndex = i;
            break;
        }
    }
//    printf("objc_draw(rsrc_object_array[%d],%d,%d,%d,%d,%d,%d)\n", treeIndex, drawob,depth,xc,yc,wc,hc);
    switch(treeIndex) {
    case RSCTREE_MENU:
    case RSCTREE_PLAY_DIALOG:
    case RSCTREE_TEAM_DIALOG:
    case RSCTREE_SET_NAME:
    case RSCTREE_ABOUT:
            break;
    case RSCTREE_SLAVE:
    case RSCTREE_MIDIRING_TIMEOUT:
    case RSCTREE_GAME_TERMINATED:
    case RSCTREE_MAZE_ERROR:
    case RSCTREE_MOUSE_CTRL:
    case RSCTREE_JOYSTICK_CTRL:
    case RSCTREE_NUMBER_XX:
    case RSCTREE_SUSPENDED:
    case RSCTREE_MIDICAM:
            dispatch_sync(dispatch_get_main_queue(), ^{
                NSMutableArray<NSString *> *lines = [NSMutableArray array];
                int rscIndex = 0;
                do {
                    if(tree[rscIndex].ob_type == G_STRING) {
                        [lines addObject:@((char*)tree[rscIndex].ob_spec)];
                    }
                    if(tree[rscIndex].ob_flags & LASTOB)
                        break;
                    ++rscIndex;
                } while(1);
                gAppDelegate.line1.objectValue = (lines.count > 0) ? lines[0] : @"";
                gAppDelegate.line2.objectValue = (lines.count > 1) ? lines[1] : @"";
                gAppDelegate.line3.objectValue = (lines.count > 2) ? lines[2] : @"";
                NSRect mainWindowFrame = gAppDelegate.window.frame;
                NSSize buttonlessWindowSize = gAppDelegate.buttonlessWindow.frame.size;
                gAppDelegate.buttonlessWindow.frameOrigin = NSMakePoint((mainWindowFrame.size.width - buttonlessWindowSize.width) / 2.0 + mainWindowFrame.origin.x, (mainWindowFrame.size.height - buttonlessWindowSize.height) / 2.0 + mainWindowFrame.origin.y);
                [gAppDelegate.buttonlessWindow makeKeyAndOrderFront:gAppDelegate];
            });
            break;
    default:
            break;
    }
    return 0;
}

short rsrc_obfix(OBJECT *tree,short obj)
{
    return 0;
}

short wind_create(short kind,short wx,short wy,short ww,short wh)
{
//    printf("wind_create(%d,%d,%d,%d,%d)\n", kind, wx,wy,ww,wh);
    return 0;
}

short wind_open(short handle,short wx,short wy,short ww,short wh)
{
//    printf("wind_open(%d,%d,%d,%d,%d)\n", handle, wx,wy,ww,wh);
    return 0;
}

short wind_close(short handle)
{
//    printf("wind_close(%d)\n", handle);
    return 0;
}

short wind_delete(short handle)
{
//    printf("wind_delete(%d)\n", handle);
    return 0;
}

// To avoid an endless loop, always return 0 to tell the Atari: nothing needs to be redrawn.
short wind_get(short w_handle,short w_field,short *pw_x,short *pw_y,short *pw_w,short *pw_h)
{
    if(w_field == WF_FIRSTXYWH || w_field == WF_NEXTXYWH) {
        *pw_w = 0;
        *pw_h = 0;
    }
    return 0;
}

short wind_update(short beg_update)
{
//    printf("wind_update(%d)\n", beg_update);
    dispatch_sync(dispatch_get_main_queue(), ^{
        [gAppDelegate.buttonlessWindow orderOut:gAppDelegate];
    });
    return 0;
}

#pragma mark -
#pragma mark VDI

void vro_cpyfm(short handle,short wr_mode,short *pxyarray,MFDB *psrcMFDB,MFDB *pdesMFDB)
{
    // never called, because wind_get() never returns anything
}

#pragma mark -
#pragma mark Screen update

// This routine converts the Atari ST plane format into RGB
// It's neither very pretty, not really fast. But because MIDImaze is way way too fast
// on a Mac anyway and the profiler can't even find a performance hit in this
// function, I don't care for now. OpenGL/Metal would be a better choice anyway.
static void        plainConvert(const unsigned short *atariScreenBufP, unsigned char *rgbScreenBuf)
{
    // 3 resolutions: 0 (320x200x4), 1 (640x200x2), 2 (640x400x1)
    int width = screen_rez != 0 ? SCREEN_BW_WIDTH : SCREEN_COL_WIDTH;
    int height = screen_rez == 2 ? SCREEN_BW_HEIGHT : SCREEN_COL_HEIGHT;
    int planeCount = (screen_rez == 2) ? SCREEN_BW_PLANES : (screen_rez == 1) ? 2 : SCREEN_COL_PLANES;

    const unsigned char *atariScreenBuf = (const unsigned char*)atariScreenBufP;

    // convert Atari ST plane screen format into RGB
    for(int y=0; y<height; ++y)
    {
        for(int x=0; x<width/16; ++x)
        {
            unsigned short    planes[4];
            for(int planeIndex=0; planeIndex<planeCount; ++planeIndex)
            {
                planes[planeIndex] = (atariScreenBuf[0] << 8) | atariScreenBuf[1];
                atariScreenBuf += sizeof(unsigned short);
            }
            for(int bitMask=0x8000; bitMask!=0; bitMask>>=1)
            {
                int colorIndex = 0;
                for(int planeIndex=0; planeIndex<planeCount; ++planeIndex)
                {
                    if(planes[planeIndex] & bitMask) colorIndex += 1 << planeIndex;
                }
                if(planeCount == 1)
                {
                    unsigned char bwValue = colorIndex ? 0xFF : 0x00;
                    if(physPalette[colorIndex] & 1) // Bit 0 = invert
                        bwValue = ~bwValue;
                    *rgbScreenBuf++ = bwValue;
                    *rgbScreenBuf++ = bwValue;
                    *rgbScreenBuf++ = bwValue;
                } else {
                    int rgb777 = physPalette[colorIndex];
                    *rgbScreenBuf++ = (((rgb777 >> 8) & 0xF) * 255) / 7;
                    *rgbScreenBuf++ = (((rgb777 >> 4) & 0xF) * 255) / 7;
                    *rgbScreenBuf++ = (((rgb777 >> 0) & 0xF) * 255) / 7;
                }
            }
        }
    }
}

// not pretty, but it gets the VT52 font rendering done for the MIDImaze case
// Performance is not important, because it is only used when the player died/won
#include "AtariST8x8.c"
#include "AtariST8x16.c"

static void renderChar(unsigned char c)
{
    int fontHeight = screen_rez == 2 ? 16 : 8;
    int planeCount = (screen_rez == 2) ? SCREEN_BW_PLANES : (screen_rez == 1) ? 2 : SCREEN_COL_PLANES;
    int lineOffset = screen_rez == 2 ? SCREEN_BW_LINEOFFSET : SCREEN_COL_LINEOFFSET;

    const unsigned char *fontPtr = fontHeight == 16 ? atariST8x16 : atariST8x8;
    // position of the font character in the struct (16 characters x 16 character matrix)
    fontPtr += (c & 0x0f) + ((c >> 4) * 16 * fontHeight);
    unsigned char *atariScreenBuf = (unsigned char *)logScreen;
    if(!atariScreenBuf) return;
    atariScreenBuf += VT52cursorPosRow * fontHeight * lineOffset + (VT52cursorPosColumn >> 1) * planeCount * sizeof(unsigned short) + (VT52cursorPosColumn & 1);
    for(int y=0; y<fontHeight; ++y) {
        unsigned char fontMask = fontPtr[y * 16];
        if(planeCount == 1) {
            *atariScreenBuf = ((VT52backgroundcolor & 1) == 1) ? ~fontMask : fontMask;
        } else {
            for(int bit=7; bit>=0; --bit) {
                int bitMask = 1 << bit;
                if(fontMask & bitMask) {
                    if(VT52textcolor & 1)
                        atariScreenBuf[0] |= bitMask;
                    else
                        atariScreenBuf[0] &= ~bitMask;
                    if(VT52textcolor & 2)
                        atariScreenBuf[2] |= bitMask;
                    else
                        atariScreenBuf[2] &= ~bitMask;
                    if(VT52textcolor & 4)
                        atariScreenBuf[4] |= bitMask;
                    else
                        atariScreenBuf[4] &= ~bitMask;
                    if(VT52textcolor & 8)
                        atariScreenBuf[6] |= bitMask;
                    else
                        atariScreenBuf[6] &= ~bitMask;
                } else {
                    if(VT52backgroundcolor & 1)
                        atariScreenBuf[0] |= bitMask;
                    else
                        atariScreenBuf[0] &= ~bitMask;
                    if(VT52backgroundcolor & 2)
                        atariScreenBuf[2] |= bitMask;
                    else
                        atariScreenBuf[2] &= ~bitMask;
                    if(VT52backgroundcolor & 4)
                        atariScreenBuf[4] |= bitMask;
                    else
                        atariScreenBuf[4] &= ~bitMask;
                    if(VT52backgroundcolor & 8)
                        atariScreenBuf[6] |= bitMask;
                    else
                        atariScreenBuf[6] &= ~bitMask;
                }
            }
        }
        atariScreenBuf += lineOffset;
    }
    VT52cursorPosColumn++;
}
