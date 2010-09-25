//
//  MotionViewerViewController.m
//  MotionViewer
//
//  Created by Matteo Bertozzi on 9/25/10.
//  Copyright 2010 Matteo Bertozzi. All rights reserved.
//

#import "MotionViewerViewController.h"

@implementation MotionViewerViewController


- (void)updateMotionData:(NSTimer *)timer {
    // Gyro Rotational Rate
    labelRotationalRateX.text = [NSString stringWithFormat:@"X: %2.2f", motionManager.deviceMotion.rotationRate.x];
    labelRotationalRateY.text = [NSString stringWithFormat:@"Y: %2.2f", motionManager.deviceMotion.rotationRate.y];
    labelRotationalRateZ.text = [NSString stringWithFormat:@"Z: %2.2f", motionManager.deviceMotion.rotationRate.z];
    
    statusRotationalRateX.progress = motionManager.deviceMotion.rotationRate.x / (2.0f * M_PI);
    statusRotationalRateY.progress = motionManager.deviceMotion.rotationRate.y / (2.0f * M_PI);
    statusRotationalRateZ.progress = motionManager.deviceMotion.rotationRate.z / (2.0f * M_PI);
    
    // User Acceleration
    labelAccelerationX.text = [NSString stringWithFormat:@"X: %2.2f", motionManager.deviceMotion.userAcceleration.x];
    labelAccelerationY.text = [NSString stringWithFormat:@"Y: %2.2f", motionManager.deviceMotion.userAcceleration.y];
    labelAccelerationZ.text = [NSString stringWithFormat:@"Z: %2.2f", motionManager.deviceMotion.userAcceleration.z];

    statusAccelerationX.progress = (motionManager.deviceMotion.userAcceleration.x + 1.0f) * 0.5f;
    statusAccelerationY.progress = (motionManager.deviceMotion.userAcceleration.y + 1.0f) * 0.5f;
    statusAccelerationZ.progress = (motionManager.deviceMotion.userAcceleration.z + 1.0f) * 0.5f;

    // Gravity
    labelGravityX.text = [NSString stringWithFormat:@"X: %2.2f", motionManager.deviceMotion.gravity.x];
    labelGravityY.text = [NSString stringWithFormat:@"Y: %2.2f", motionManager.deviceMotion.gravity.y];
    labelGravityZ.text = [NSString stringWithFormat:@"Z: %2.2f", motionManager.deviceMotion.gravity.z];

    statusGravityX.progress = (motionManager.deviceMotion.gravity.x + 1.0f) * 0.5f;
    statusGravityY.progress = (motionManager.deviceMotion.gravity.y + 1.0f) * 0.5f;
    statusGravityZ.progress = (motionManager.deviceMotion.gravity.z + 1.0f) * 0.5f;
    
    // Attitude
    labelAttitudeRoll.text = [NSString stringWithFormat:@"%2.2f", motionManager.deviceMotion.attitude.roll];
    labelAttitudeYaw.text = [NSString stringWithFormat:@"%2.2f", motionManager.deviceMotion.attitude.yaw];
    labelAttitudePitch.text = [NSString stringWithFormat:@"%2.2f", motionManager.deviceMotion.attitude.pitch];
}

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
    
    [[UIAccelerometer sharedAccelerometer] setUpdateInterval:0.2f];
    [[UIAccelerometer sharedAccelerometer] setDelegate:self];
    
    motionManager = [[CMMotionManager alloc] init];
    motionManager.accelerometerUpdateInterval = 0.01;
    motionManager.deviceMotionUpdateInterval = 0.01;
    [motionManager startDeviceMotionUpdates];
    
    timer = [NSTimer scheduledTimerWithTimeInterval:0.2f 
                                             target:self 
                                           selector:@selector(updateMotionData:) 
                                           userInfo:nil 
                                            repeats:YES];
}


// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return (UIInterfaceOrientationIsLandscape(interfaceOrientation));
}

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
    [timer invalidate];
    
    [motionManager stopDeviceMotionUpdates];
    [motionManager release];
    
    [[UIAccelerometer sharedAccelerometer] setDelegate:nil];
}

- (void)accelerometer:(UIAccelerometer *)accelerometer 
        didAccelerate:(UIAcceleration *)acceleration
{
    labelAccelerometerX.text = [NSString stringWithFormat:@"X: %2.2f", acceleration.x];
    labelAccelerometerY.text = [NSString stringWithFormat:@"Y: %2.2f", acceleration.y];
    labelAccelerometerZ.text = [NSString stringWithFormat:@"Z: %2.2f", acceleration.z];
    
    statusAccelerometerX.progress = (acceleration.x + 1.0f) * 0.5f;
    statusAccelerometerY.progress = (acceleration.y + 1.0f) * 0.5f;
    statusAccelerometerZ.progress = (acceleration.z + 1.0f) * 0.5f;
}

- (void)dealloc {
    [super dealloc];
}

@end
