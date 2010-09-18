//
//  WindMillViewController.h
//  WindMill
//
//  Created by Matteo Bertozzi on 8/29/09.
//  Copyright Matteo Bertozzi 2009. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreAudio/CoreAudioTypes.h>

#import "VoiceMillView.h"

@interface VoiceMillViewController : UIViewController {
	IBOutlet VoiceMillView *windMill;
	IBOutlet UILabel *statusLabel;
	
	AVAudioRecorder *recorder;
	NSTimer *levelTimer;
	double lowPassResults;
}

@end

