//
//  AudioRecorder.h
//  UdpVoice
//
//  Created by Matteo Bertozzi on 7/10/10.
//  Copyright 2010 Matteo Bertozzi. All rights reserved.
//

#ifndef _AUDIO_RECORDER_H_
#define _AUDIO_RECORDER_H_

#include <AudioToolbox/AudioToolbox.h>
#include <AudioToolbox/AudioQueue.h>

typedef struct _AudioRecorder AudioRecorder;
typedef void (*AudioRecorderPacketReceived) (const void *audioData,
                                             UInt32 audioDataByteSize,
                                             void *userData);

#define kBufferSize         (16 * 1024)
#define kBufferNumQueues    (2)

struct _AudioRecorder {
    AudioStreamBasicDescription mDataFormat;
    AudioQueueRef               mQueue;
    AudioRecorderPacketReceived mPacketReceivedHandler;
    bool                        mIsRunning;
    void *                      mUserData;
};

OSStatus AudioRecorderInit      (AudioRecorder *recorder,
                                 AudioRecorderPacketReceived packetReceivedHandler,
                                 void *userData);
OSStatus AudioRecorderDestroy   (AudioRecorder *recorder);

OSStatus AudioRecorderStart     (AudioRecorder *recorder);
OSStatus AudioRecorderStop      (AudioRecorder *recorder);

#endif /* !_AUDIO_RECORDER_H_ */
