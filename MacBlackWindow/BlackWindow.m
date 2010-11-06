//
//  BlackWindow.m
//  BlackWindow
//
//  Created by Matteo Bertozzi on 10/29/10.
//  Copyright 2010 Matteo Bertozzi. All rights reserved.
//

#import "BlackWindow.h"

@implementation BlackWindow

@synthesize isMiniaturizable;
@synthesize isResizable;
@synthesize isClosable;

- (id)initWithContentRect:(NSRect)contentRect 
                styleMask:(NSUInteger)aStyle 
                  backing:(NSBackingStoreType)bufferingType 
                    defer:(BOOL)flag
{
    if ((self = [super initWithContentRect:contentRect 
                                styleMask:NSBorderlessWindowMask
                                 backing:bufferingType
                                   defer:flag]))
    {    
        self.isMiniaturizable = YES;
        self.isResizable = YES;
        self.isClosable = YES;
        [self setHasShadow:YES];
        [self setOpaque:NO];
    }

    return(self);
}

- (void)dealloc {
    [super dealloc];
}

- (BOOL)canBecomeKeyWindow {
    return(YES);
}

@end
