//
//  AppDelegate.m
//  MIDIMaze
//
//  Created by Markus Fritze on 27.09.17.
//  Copyright © 2017 Markus Fritze. All rights reserved.
//

#import "AppDelegate.h"
extern "C" {
#import "globals.h"
}

int gBIOSKeyCode = 0;
int gAltKeyCode = 0;
static int gJoystickMask = 0;

// 'implement' the atari specific functions for the joystick
void init_joystick(void)
{
    gJoystickMask = 0;
}

void exit_joystick(void)
{
    gJoystickMask = 0;
}

int ask_joystick(void)
{
    return gJoystickMask;
}



NSBitmapImageRep *gImageRep;
AppDelegate *gAppDelegate;

@interface MMMImageView : NSImageView
@end

@implementation MMMImageView
- (void)awakeFromNib
{
    // build an image bitmap for the rendering
    gImageRep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL pixelsWide:Getrez() != 0 ? SCREEN_BW_WIDTH : SCREEN_COL_WIDTH pixelsHigh:Getrez() == 2 ? SCREEN_BW_HEIGHT : SCREEN_COL_HEIGHT bitsPerSample:8 samplesPerPixel:3 hasAlpha:NO isPlanar:NO colorSpaceName:NSCalibratedRGBColorSpace bitmapFormat:0 bytesPerRow:0 bitsPerPixel:0];
    if(gImageRep.bitmapData) {
        NSImage *img = [[NSImage alloc] init];
        [img addRepresentation:gImageRep];
        self.image = img;
    }

    [self.window makeFirstResponder:self];
}

// emulate keyboard/joystick
- (void)keyDown:(NSEvent *)event
{
    NSString *characters = event.charactersIgnoringModifiers;
    if(characters.length > 0) {
        unichar character = [characters characterAtIndex:0];
        if (character == NSUpArrowFunctionKey) {
            gJoystickMask |= JOYSTICK_UP;
        } else if (character == NSDownArrowFunctionKey) {
            gJoystickMask |= JOYSTICK_DOWN;
        } else if (character == NSLeftArrowFunctionKey) {
            gJoystickMask |= JOYSTICK_LEFT;
        } else if (character == NSRightArrowFunctionKey) {
            gJoystickMask |= JOYSTICK_RIGHT;
        } else if (character == '0') {
            gJoystickMask |= JOYSTICK_BUTTON;
        } else if (character == 'p') {
            dumpMapInfos();
        } else {
            int scanCode = 0;
            switch(character) { // Generate Atari ST scancodes, but only the ones MIDImaze actually uses
            case  27: scanCode = 0x01; break;
            case ' ': scanCode = 0x39; break;
            case 'q': scanCode = 0x10; break;
            case 'r': scanCode = 0x13; break;
            case  13: scanCode = 0x1C; break;
            case 's': scanCode = 0x1F; break;
            case 'j': scanCode = 0x24; break;
            case 'c': scanCode = 0x2e; break;
            case 'm': scanCode = 0x32; break;
            }
            // MIDImaze only uses the control key for key-commands
            int controlFlags = 0;
            if(event.modifierFlags & NSEventModifierFlagOption)
                controlFlags = 8;
            gAltKeyCode = (controlFlags << 16) | (scanCode << 8) | character;
            gBIOSKeyCode = (((controlFlags << 16) | (scanCode << 8)) << 8) | character;
        }
    }
}

- (void)keyUp:(NSEvent *)event
{
    NSString *characters = event.characters;
    if(characters.length > 0) {
        unichar character = [characters characterAtIndex:0];
        if (character == NSUpArrowFunctionKey) {
            gJoystickMask &= ~JOYSTICK_UP;
        } else if (character == NSDownArrowFunctionKey) {
            gJoystickMask &= ~JOYSTICK_DOWN;
        } else if (character == NSLeftArrowFunctionKey) {
            gJoystickMask &= ~JOYSTICK_LEFT;
        } else if (character == NSRightArrowFunctionKey) {
            gJoystickMask &= ~JOYSTICK_RIGHT;
        } else if (character == '0') {
            gJoystickMask &= ~JOYSTICK_BUTTON;
        } else {
            gBIOSKeyCode = 0;
        }
    }
}
@end

@implementation AppDelegate
MMSelectedMenuItem gSelectedMenuItem = MMSelectedMenuItemNone;

// Trigger macOS menu items and forward them to the AES
- (IBAction)aboutMM:(id)sender {
    gSelectedMenuItem = MMSelectedMenuItemAbout;
}
- (IBAction)loadMaze:(id)sender {
    gSelectedMenuItem = MMSelectedMenuItemLoad;
}
- (IBAction)resetScore:(id)sender {
    gSelectedMenuItem = MMSelectedMenuItemReset;
}
- (IBAction)play:(id)sender {
    gSelectedMenuItem = MMSelectedMenuItemPlay;
}
- (IBAction)names:(id)sender {
    gSelectedMenuItem = MMSelectedMenuItemNames;
}
- (IBAction)MMquit:(id)sender {
    gSelectedMenuItem = MMSelectedMenuItemQuit;
}
- (IBAction)aboutOK:(id)sender {
    [NSApp stopModal];
    [self.aboutDialog orderOut:self];
}
- (IBAction)playDialogButton:(id)sender {
    [NSApp stopModalWithCode:[sender tag]];
}
- (IBAction)nameDialogButton:(id)sender {
    [NSApp stopModalWithCode:[sender tag]];
    [self.nameDialog orderOut:self];
}
- (IBAction)teamDialogButton:(id)sender {
    [NSApp stopModalWithCode:[sender tag]];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // fix the aspect ratio to Atari ST
    self.window.contentAspectRatio = NSMakeSize(SCREEN_BW_WIDTH,SCREEN_BW_HEIGHT);
    gAppDelegate = self;

    // MIDImaze runs in a separate thread, synchronized via GCD
    [NSThread detachNewThreadWithBlock:^{
        setup_game();
        dispatch_sync(dispatch_get_main_queue(), ^{
            [NSApp terminate:self];
        });
    }];
}

@end

static void dumpDroneInfo()
{
    for(int ply=0; ply<playerAndDroneCount; ++ply)
    {
        static const char *dirStr[8] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" };
        printf("%2d : Y:%#4.4x/%6.3f X:%#4.4x/%6.3f %2s(0x%-2.2x/%3.0f) %d %#2.2x\n", ply,
               player_data[ply].ply_y & 0xFFFF, player_data[ply].ply_y * 1.0/128.0,
               player_data[ply].ply_x & 0xFFFF, player_data[ply].ply_x * 1.0/128.0,
               dirStr[player_data[ply].ply_dir / 32], player_data[ply].ply_dir & 0xFF, player_data[ply].ply_dir / 256.0 * 360.0,
               player_data[ply].ply_lives, player_joy_table[ply]);
        if(player_data[ply].dr_type) {
            printf("   : D:%c RT:%2d UR:%2d --:%d FD:%2.2x LOCK:%d CT:%2d P:%2d\n",  player_data[ply].dr_type ?: '#', player_data[ply].dr_rotateCounter, player_data[ply].dr_upRotationCounter, player_data[ply].dr_isInactive, player_data[ply].dr_fireDirection, player_data[ply].dr_targetLocked, player_data[ply].dr_currentTarget, player_data[ply].dr_permanentTarget);
            if(player_data[ply].dr_dir[0]) {
                printf("   : %2d ", player_data[ply].dr_fieldResetTimer);
                for(int i=0; i<6; ++i) {
                    if(player_data[ply].dr_dir[i] < 0)
                        break;
                    if(i == player_data[ply].dr_fieldIndex)
                        printf("->");
                    printf("%s", dirStr[(player_data[ply].dr_dir[i] & 0xff) / 32]);
                    printf("[%d,%d]", player_data[ply].dr_field[i].y, player_data[ply].dr_field[i].x);
                    printf(" ");
                }
                printf("\n");
            }
        }
    }
}

// Debug: dump info about the look of the maze and where all players are
void dumpMapInfos(void)
{
    printf("   ");
    for(int fieldX=0; fieldX<=maze_size; fieldX++)
        printf("%d", fieldX % 10);
    printf("\n");
    for(int fieldY=0; fieldY<=maze_size; fieldY++)
    {
        printf("%2d:",fieldY);
        for(int fieldX=0; fieldX<=maze_size; fieldX++)
        {
            int obj = maze_datas[fieldY * MAZE_MAX_SIZE + fieldX];
            char c = ' ';
            if(((fieldY & fieldX) & 1)==0 && obj == MAZE_FIELD_WALL)
            {
                c = '#';
            } else {
                if(obj >= 0 && obj < 16) c = 'a' + obj;
                if(obj >= 16 && obj < 32) c = 'A' + obj - 16;
            }
            printf("%c", c);
        }
        printf("\n");
    }
    dumpPlayerInfo();
}

// Debug: dump info where all players are
void dumpPlayerInfo(void)
{
    dumpDroneInfo();
    printf("\n");
}
