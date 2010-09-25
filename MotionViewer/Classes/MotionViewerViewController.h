//
//  MotionViewerViewController.h
//  MotionViewer
//
//  Created by Matteo Bertozzi on 9/25/10.
//  Copyright 2010 Matteo Bertozzi. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <CoreMotion/CoreMotion.h>

@interface MotionViewerViewController : UIViewController<UIAccelerometerDelegate> {
    IBOutlet UIProgressView *statusAccelerometerX;
    IBOutlet UIProgressView *statusAccelerometerY;
    IBOutlet UIProgressView *statusAccelerometerZ;
    
    IBOutlet UILabel *labelAccelerometerX;
    IBOutlet UILabel *labelAccelerometerY;
    IBOutlet UILabel *labelAccelerometerZ;

    IBOutlet UIProgressView *statusRotationalRateX;
    IBOutlet UIProgressView *statusRotationalRateY;
    IBOutlet UIProgressView *statusRotationalRateZ;

    IBOutlet UILabel *labelRotationalRateX;
    IBOutlet UILabel *labelRotationalRateY;
    IBOutlet UILabel *labelRotationalRateZ;

    IBOutlet UIProgressView *statusAccelerationX;
    IBOutlet UIProgressView *statusAccelerationY;
    IBOutlet UIProgressView *statusAccelerationZ;

    IBOutlet UILabel *labelAccelerationX;
    IBOutlet UILabel *labelAccelerationY;
    IBOutlet UILabel *labelAccelerationZ;

    IBOutlet UIProgressView *statusGravityX;
    IBOutlet UIProgressView *statusGravityY;
    IBOutlet UIProgressView *statusGravityZ;
    
    IBOutlet UILabel *labelGravityX;
    IBOutlet UILabel *labelGravityY;
    IBOutlet UILabel *labelGravityZ;

    IBOutlet UILabel *labelAttitudeRoll;
    IBOutlet UILabel *labelAttitudePitch;
    IBOutlet UILabel *labelAttitudeYaw;  
    
    CMMotionManager *motionManager;  
    NSTimer *timer;
}

@end

