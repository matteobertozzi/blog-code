//
//  MotionViewerAppDelegate.h
//  MotionViewer
//
//  Created by Matteo Bertozzi on 9/25/10.
//  Copyright 2010 Matteo Bertozzi. All rights reserved.
//

#import <UIKit/UIKit.h>

@class MotionViewerViewController;

@interface MotionViewerAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    MotionViewerViewController *viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet MotionViewerViewController *viewController;

@end

