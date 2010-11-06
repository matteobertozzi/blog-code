//
//  ResizeCorner.m
//  BlackWindow
//
//  Created by Matteo Bertozzi on 10/30/10.
//  Copyright 2010 Matteo Bertozzi. All rights reserved.
//

#import "ResizeCorner.h"


@implementation ResizeCorner

- (void)drawRect:(NSRect)rect {
	NSRect bounds = [self bounds];
	[[NSColor blackColor] set];
	[NSBezierPath strokeLineFromPoint:NSZeroPoint toPoint:NSMakePoint(NSMaxX(bounds), NSMaxY(bounds))];
}

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent {
    return YES;
}

- (void)mouseDown:(NSEvent *)theEvent {
	[super mouseDown:theEvent];
}

- (void)mouseDragged:(NSEvent *)theEvent {
    NSWindow *window = [self window];
	NSRect frame = [window frame];
    float newHeight = frame.size.height + [theEvent deltaY];
    float newWidth  = frame.size.width  + [theEvent deltaX];
    NSSize minSize = [window minSize];
    if (newHeight >= minSize.height) {
        frame.size.height = newHeight;
        frame.origin.y -= [theEvent deltaY];
    }
    if (newWidth >= minSize.width)
        frame.size.width = newWidth;
    [window setFrame:frame display:YES];
}

@end
