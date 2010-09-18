//
//  VoiceMillAppDelegate.h
//  WindMill
//
//  Created by Matteo Bertozzi on 8/29/09.
//  Copyright Matteo Bertozzi 2009. All rights reserved.
//

#import <UIKit/UIKit.h>

@class VoiceMillViewController;

@interface VoiceMillAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    VoiceMillViewController *viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet VoiceMillViewController *viewController;

@end

