//
//  BlackWindow.h
//  BlackWindow
//
//  Created by Matteo Bertozzi on 10/29/10.
//  Copyright 2010 Matteo Bertozzi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface BlackWindow : NSWindow {
    BOOL isMiniaturizable;
    BOOL isResizable;
    BOOL isClosable;
}

@property BOOL isMiniaturizable;
@property BOOL isResizable;
@property BOOL isClosable;

@end
