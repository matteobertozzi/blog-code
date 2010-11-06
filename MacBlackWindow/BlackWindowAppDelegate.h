//
//  BlackWindowAppDelegate.h
//  BlackWindow
//
//  Created by Matteo Bertozzi on 11/6/10.
//  Copyright 2010 Matteo Bertozzi. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "BlackWindow.h"

@interface BlackWindowAppDelegate : NSObject <NSApplicationDelegate> {
    BlackWindow *window;
}

@property (assign) IBOutlet BlackWindow *window;

- (IBAction)closable:(id)sender;
- (IBAction)minimizable:(id)sender;
- (IBAction)zoomable:(id)sender;
- (IBAction)documentEdited:(id)sender;
 
@end
