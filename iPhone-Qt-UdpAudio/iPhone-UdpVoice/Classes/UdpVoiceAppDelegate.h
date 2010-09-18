//
//  UdpVoiceAppDelegate.h
//  UdpVoice
//
//  Created by Matteo Bertozzi on 7/10/10.
//  Copyright Matteo Bertozzi 2010. All rights reserved.
//

#import <UIKit/UIKit.h>

@class UdpVoiceViewController;

@interface UdpVoiceAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    UdpVoiceViewController *viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet UdpVoiceViewController *viewController;

@end

