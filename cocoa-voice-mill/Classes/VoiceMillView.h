//
//  WindMillView.h
//  WindMill
//
//  Created by Matteo Bertozzi on 8/29/09.
//  Copyright 2009 Matteo Bertozzi. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface VoiceMillView : UIView {
	double angle;
}

@property (readonly) double angle;

- (void)updateAngle:(double)value;

@end
