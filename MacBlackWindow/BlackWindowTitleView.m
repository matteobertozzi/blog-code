//
//  BlackWindowTitleView.m
//  BlackWindow
//
//  Created by Matteo Bertozzi on 10/29/10.
//  Copyright 2010 Matteo Bertozzi. All rights reserved.
//

#import "BlackWindowTitleView.h"
#import "BlackWindow.h"


#define kTitlebarMiddleWidth            (1)
#define kTitlebarCornerWidth            (4)
#define kTitlebarCornerWidth2           (8)
#define kTitlebarHeight                 (23)

@implementation BlackWindowTitleView

- (void)_closeWindow {
    NSWindow *window = [self window];
    id delegate = [window delegate];
    
    // Check if window want to stay up.
    if ([delegate respondsToSelector:@selector(windowShouldClose:)]) {  
        if (![delegate windowShouldClose:window])
            return;
    }
    
    [window close];
}


- (id)initWithFrame:(NSRect)frame {
    if ((self = [super initWithFrame:frame]) != nil) {
        buttonsHover = NO;
        
        closeButton = [[NSImageView alloc] initWithFrame:NSMakeRect(8.0, 3.0, 16.0, 16.0)];
        [closeButton setImage:[NSImage imageNamed:@"close-active-color"]];
        [self addSubview:closeButton];
        
        minimizeButton = [[NSImageView alloc] initWithFrame:NSMakeRect(28.0, 3.0, 16.0, 16.0)];
        [minimizeButton setImage:[NSImage imageNamed:@"minimize-active-color"]];
        [self addSubview:minimizeButton];

        zoomButton = [[NSImageView alloc] initWithFrame:NSMakeRect(48.0, 3.0, 16.0, 16.0)];
        [zoomButton setImage:[NSImage imageNamed:@"zoom-active-color"]];
        [self addSubview:zoomButton];
        
        titleLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, frame.size.width, frame.size.height - 4)];
        [titleLabel setAutoresizingMask:NSViewMinYMargin | NSViewWidthSizable];
        [titleLabel setAlignment:NSCenterTextAlignment];
        [titleLabel setTextColor:[NSColor whiteColor]];
        [titleLabel setDrawsBackground:NO];
        [titleLabel setBordered:NO];
        [titleLabel setEditable:NO];
        [self addSubview:titleLabel];
        
        [self addTrackingRect:NSMakeRect(0.0, 0.0, 64.0, 23.0) owner:self userData:nil assumeInside:NO];
    }
        
    return self;
}

- (void)dealloc {
    [titleLabel release];
    [minimizeButton release];
    [closeButton release];
    [zoomButton release];
    [super dealloc];
}

- (BOOL)isOpaque {
    return(YES);
}


- (void)drawRect:(NSRect)dirtyRect {
    CGFloat fw = [self frame].size.width;
    CGFloat h = dirtyRect.size.height;
    CGFloat w = dirtyRect.size.width;
    CGFloat x = dirtyRect.origin.x;
    CGFloat y = dirtyRect.origin.y;
    
    [[NSColor blackColor] set];
    NSEraseRect(dirtyRect);
    NSRectFill(dirtyRect);
    
    if (x < kTitlebarCornerWidth) {
        NSImage *cornerLeft = [NSImage imageNamed:@"titlebar-corner-left"];
        [cornerLeft drawInRect:NSMakeRect(x, y, kTitlebarCornerWidth - x, h) 
                    fromRect:NSMakeRect(x, y, kTitlebarCornerWidth - x, h) 
                    operation:NSCompositeSourceOver
                    fraction:1.0];
    }

    if ((x + w) > (fw - kTitlebarCornerWidth2)) {
        NSImage *cornerRight = [NSImage imageNamed:@"titlebar-corner-right"];
        [cornerRight drawInRect:NSMakeRect(fw - kTitlebarCornerWidth, y, kTitlebarCornerWidth, h) 
                       fromRect:NSMakeRect(0, y, kTitlebarCornerWidth, h) 
                      operation:NSCompositeSourceOver 
                       fraction:1.0];
    }
    
    NSImage *middle = [NSImage imageNamed:@"titlebar-middle"];
    [middle drawInRect:NSMakeRect(kTitlebarCornerWidth, y, fw - kTitlebarCornerWidth2, h) 
            fromRect:NSMakeRect(0, y, kTitlebarMiddleWidth, h)
            operation:NSCompositeSourceOver
            fraction:1.0];

    // Smooth Edges
    [[NSColor clearColor] set];
    NSRectFill(NSMakeRect(0, 22, 2, 1));
    NSRectFill(NSMakeRect(0, 21, 1, 1));
    NSRectFill(NSMakeRect(fw - 2, 22, 2, 1));
    NSRectFill(NSMakeRect(fw - 1, 21, 1, 1));
}

- (BOOL)mouseDownCanMoveWindow {
    return(NO);
}

- (void)refreshButtons {
    BlackWindow *window = (BlackWindow *)[self window];
    BOOL isKeyWindow = [window isKeyWindow];
    NSString *imageName;
    
    // Close
    if (!window.isClosable) {
        imageName = @"close-inactive-disabled-color";
    } else if (buttonsHover) {
        if ([[self window] isDocumentEdited])
            imageName = @"closedirty-rollover-color";
        else
            imageName = @"close-rollover-color";
    } else if ([window isKeyWindow]) {
        if ([[self window] isDocumentEdited])
            imageName = @"closedirty-active-color";
        else
            imageName = @"close-active-color";
    } else {
        if ([[self window] isDocumentEdited])
            imageName = @"closedirty-activenokey-color";
        else
            imageName = @"close-activenokey-color";
    }
    [closeButton setImage:[NSImage imageNamed:imageName]];

    // Minimize
    if (!window.isMiniaturizable)
        imageName = @"minimize-inactive-disabled-color";
    else if (buttonsHover)
        imageName = @"minimize-rollover-color";
    else if (isKeyWindow)
        imageName = @"minimize-active-color";
    else
        imageName = @"minimize-activenokey-color";
    [minimizeButton setImage:[NSImage imageNamed:imageName]];
    
    // Zoom
    if (!window.isResizable)
        imageName = @"zoom-inactive-disabled-color";
    else if (buttonsHover)
        imageName = @"zoom-rollover-color";
    else if (isKeyWindow)
        imageName = @"zoom-active-color";
    else
        imageName = @"zoom-activenokey-color";
    [zoomButton setImage:[NSImage imageNamed:imageName]];
}

- (void)mouseEntered:(NSEvent *)theEvent {
    buttonsHover = YES;
    [self refreshButtons];
}

- (void)mouseExited:(NSEvent *)theEvent {
    buttonsHover = NO;
    [self refreshButtons];
}

- (void)mouseDragged:(NSEvent *)theEvent {
    NSWindow *window = [self window];
    if ([window isMovable]) {
        NSPoint origin = [window frame].origin;
        [window setFrameOrigin:NSMakePoint(origin.x + [theEvent deltaX], origin.y - [theEvent deltaY])];
    }
}

- (void)mouseUp:(NSEvent *)theEvent {
    BlackWindow *window = (BlackWindow *)[self window];
    
    NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    if (NSPointInRect(location, [closeButton frame]) && window.isClosable)
        [self _closeWindow];
    else if (NSPointInRect(location, [minimizeButton frame]) && window.isMiniaturizable)
        [[self window] miniaturize:nil];
    else if (NSPointInRect(location, [zoomButton frame]) && window.isResizable)
        [[self window] zoom:nil];
}

- (void)windowKeyChanged:(id)object {
    [self refreshButtons];
}

- (void)viewWillMoveToWindow:(NSWindow *)newWindow {
    [newWindow addObserver:self forKeyPath:@"title" options:NSKeyValueObservingOptionNew context:NULL];
    [newWindow addObserver:self forKeyPath:@"isMiniaturizable" options:NSKeyValueObservingOptionNew context:NULL];
    [newWindow addObserver:self forKeyPath:@"isResizable" options:NSKeyValueObservingOptionNew context:NULL];
    [newWindow addObserver:self forKeyPath:@"isClosable" options:NSKeyValueObservingOptionNew context:NULL];
    [newWindow addObserver:self forKeyPath:@"isDocumentEdited" options:NSKeyValueObservingOptionNew context:NULL];
        
    NSNotificationCenter *notificationCenter = [NSNotificationCenter defaultCenter];     
    [notificationCenter addObserver:self selector:@selector(windowKeyChanged:) name:NSWindowDidBecomeKeyNotification object:NULL];
    [notificationCenter addObserver:self selector:@selector(windowKeyChanged:) name:NSWindowDidResignKeyNotification object:NULL];

    [titleLabel setStringValue:[newWindow title]];
    [super viewWillMoveToWindow:newWindow];
}

- (void)observeValueForKeyPath:(NSString *)keyPath 
                      ofObject:(id)object 
                        change:(NSDictionary *)change 
                      context:(void *)context 
{
    id newValue = [change objectForKey:NSKeyValueChangeNewKey];
    
    if ([keyPath isEqual:@"title"]) {
        [titleLabel setStringValue:(newValue != nil) ? newValue : @""];
    } else if ([keyPath isEqual:@"isDocumentEdited"] || 
               [keyPath isEqual:@"isClosable"] ||
               [keyPath isEqual:@"isResizable"] ||
               [keyPath isEqual:@"isMiniaturizable"])
    {
        [self refreshButtons];
    }
}

@end
