//
//  BlackWindowTitleView.h
//  BlackWindow
//
//  Created by Matteo Bertozzi on 10/29/10.
//  Copyright 2010 Matteo Bertozzi. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface BlackWindowTitleView : NSView {
    NSImageView *minimizeButton;
    NSImageView *closeButton;
    NSImageView *zoomButton;
    NSTextField *titleLabel;
    BOOL buttonsHover;
}

@end
