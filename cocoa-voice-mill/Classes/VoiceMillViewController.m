//
//  WindMillViewController.m
//  WindMill
//
//  Created by Matteo Bertozzi on 8/29/09.
//  Copyright Matteo Bertozzi 2009. All rights reserved.
//

#import "VoiceMillViewController.h"

@implementation VoiceMillViewController



/*
// The designated initializer. Override to perform setup that is required before the view is loaded.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if (self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) {
        // Custom initialization
    }
    return self;
}
*/

/*
// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView {
}
*/


// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
	
	NSURL *url = [NSURL fileURLWithPath:@"/dev/null"];
	NSDictionary *settings = [NSDictionary dictionaryWithObjectsAndKeys:
		[NSNumber numberWithFloat:44100.0], AVSampleRateKey,
		[NSNumber numberWithInt:kAudioFormatAppleLossless], AVFormatIDKey,
		[NSNumber numberWithInt:1], AVNumberOfChannelsKey,
		[NSNumber numberWithInt:AVAudioQualityLow], AVEncoderAudioQualityKey,
		nil];

	NSError *error;
	recorder = [[AVAudioRecorder alloc] initWithURL:url settings:settings error:&error];
	if (recorder) {
		[recorder prepareToRecord];
		recorder.meteringEnabled = YES;
		[recorder record];

		levelTimer = [NSTimer scheduledTimerWithTimeInterval:0.03f 
					  target:self selector:@selector(levelTimerHandler:) 
					  userInfo:nil repeats:YES];
	} else {
		statusLabel.text = [error description];
	}
}


// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return (YES);
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation {
	[windMill updateAngle:windMill.angle];
}

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	[levelTimer release];
	[recorder release];
}

- (void)levelTimerHandler:(NSTimer *)timer {
	[recorder updateMeters];
	
	float peakPowerForChannel = pow(10, (0.05f * [recorder peakPowerForChannel:0]));
	lowPassResults = 0.05 * peakPowerForChannel + (1.0 - 0.05) * lowPassResults;
	
	if (lowPassResults > 0.15f)
		[windMill updateAngle:windMill.angle + (1.9f * lowPassResults)];
	statusLabel.text = [NSString stringWithFormat:@"%f", lowPassResults];
}

- (void)dealloc {
    [super dealloc];
}

@end
