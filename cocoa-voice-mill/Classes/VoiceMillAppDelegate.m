//
//  WindMillAppDelegate.m
//  WindMill
//
//  Created by Matteo Bertozzi on 8/29/09.
//  Copyright Matteo Bertozzi 2009. All rights reserved.
//

#import "VoiceMillAppDelegate.h"
#import "VoiceMillViewController.h"

@implementation VoiceMillAppDelegate

@synthesize window;
@synthesize viewController;


- (void)applicationDidFinishLaunching:(UIApplication *)application {    
    
    // Override point for customization after app launch    
    [window addSubview:viewController.view];
    [window makeKeyAndVisible];
}


- (void)dealloc {
    [viewController release];
    [window release];
    [super dealloc];
}


@end
