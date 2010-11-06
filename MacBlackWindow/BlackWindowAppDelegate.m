//
//  BlackWindowAppDelegate.m
//  BlackWindow
//
//  Created by Matteo Bertozzi on 11/6/10.
//  Copyright 2010 Matteo Bertozzi. All rights reserved.
//

#import "BlackWindowAppDelegate.h"

@implementation BlackWindowAppDelegate

@synthesize window;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	// Insert code here to initialize your application 
}

- (IBAction)closable:(id)sender {
    window.isClosable  = ([sender state] == NSOnState);   
}

- (IBAction)minimizable:(id)sender {
    window.isMiniaturizable = ([sender state] == NSOnState);   
}

- (IBAction)zoomable:(id)sender {
    window.isResizable = ([sender state] == NSOnState);
}

- (IBAction)documentEdited:(id)sender {
    [window setDocumentEdited:([sender state] == NSOnState)];
}

@end
