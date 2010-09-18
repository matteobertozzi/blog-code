//
//  UdpVoiceViewController.h
//  UdpVoice
//
//  Created by Matteo Bertozzi on 7/10/10.
//  Copyright Matteo Bertozzi 2010. All rights reserved.
//

#import <AVFoundation/AVFoundation.h>
#import <CoreAudio/CoreAudioTypes.h>
#import <UIKit/UIKit.h>

#include <dispatch/dispatch.h>

#import <netinet/in.h>
#import "AudioRecorder.h"

//#define STREAM_DEBUG
#define STREAM_DEBUG_FILENAME       "/Users/oz/stream.raw"

typedef struct _UdpVoiceData {
    AudioRecorder recorder;

    dispatch_queue_t netqueue;
    struct sockaddr_in saddr;
    int socket;
    
#ifdef STREAM_DEBUG
    int stream;
#endif
} UdpVoiceData;

@interface UdpVoiceViewController : UIViewController {
    UdpVoiceData data;
}

- (void)startRecording;
- (void)stopRecording;

@end

