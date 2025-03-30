//
//  AppDelegate.h
//  MIDIMaze
//
//  Created by Markus Fritze on 27.09.17.
//  Copyright © 2017 Markus Fritze. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef NS_ENUM (NSInteger, MMSelectedMenuItem) {
    MMSelectedMenuItemNone = -1,
    MMSelectedMenuItemAbout,
    MMSelectedMenuItemLoad,
    MMSelectedMenuItemReset,
    MMSelectedMenuItemPlay,
    MMSelectedMenuItemNames,
    MMSelectedMenuItemQuit,
};

extern MMSelectedMenuItem gSelectedMenuItem;
extern int gBIOSKeyCode;
extern int gAltKeyCode;

extern NSBitmapImageRep *gImageRep;
@class AppDelegate;
extern AppDelegate *gAppDelegate;

@interface AppDelegate : NSObject <NSApplicationDelegate,NSWindowDelegate>
@property (weak) IBOutlet NSWindow *window;
@property (weak) IBOutlet NSImageView *imageView;
@property (weak) IBOutlet NSWindow *aboutDialog;
@property (weak) IBOutlet NSPanel *buttonlessWindow;
@property (weak) IBOutlet NSTextField *line1;
@property (weak) IBOutlet NSTextField *line2;
@property (weak) IBOutlet NSTextField *line3;
@property (weak) IBOutlet NSPanel *playDialog;
@property (weak) IBOutlet NSPanel *nameDialog;
@property (weak) IBOutlet NSTextField *nameDialogName;
@property (weak) IBOutlet NSPanel *teamDialog;
@end

