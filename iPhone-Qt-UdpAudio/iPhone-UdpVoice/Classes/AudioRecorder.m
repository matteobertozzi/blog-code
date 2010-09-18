//
//  AudioRecorder.m
//  UdpVoice
//
//  Created by Matteo Bertozzi on 7/10/10.
//  Copyright 2010 Matteo Bertozzi. All rights reserved.
//

#import "AudioRecorder.h"

static void _handleInputBuffer (void *aqData,
                                AudioQueueRef inAQ,
                                AudioQueueBufferRef inBuffer,
                                const AudioTimeStamp *inStartTime,
                                UInt32 inNumPackets,
                                const AudioStreamPacketDescription *inPacketDesc)
{
    AudioRecorder *recorder = (AudioRecorder *)aqData;

    recorder->mPacketReceivedHandler(inBuffer->mAudioData, 
                                     inBuffer->mAudioDataByteSize,
                                     recorder);
 
    AudioQueueEnqueueBuffer(recorder->mQueue, inBuffer, 0, NULL);
}

static void _initAudioRecorderFormat (AudioStreamBasicDescription *format)
{
    format->mFormatID         = kAudioFormatLinearPCM;
    format->mSampleRate       = 10100;
    format->mChannelsPerFrame = 1;
    format->mBitsPerChannel   = 16;
    format->mFramesPerPacket  = 1;
    format->mBytesPerFrame    = format->mChannelsPerFrame * (format->mBitsPerChannel / 8);
    format->mBytesPerPacket   = format->mFramesPerPacket * format->mBytesPerFrame;
    format->mFormatFlags      = kLinearPCMFormatFlagIsSignedInteger |
                                kLinearPCMFormatFlagIsPacked;
}

OSStatus AudioRecorderInit (AudioRecorder *recorder,
                            AudioRecorderPacketReceived packetReceivedHandler,
                            void *userData)
{
    AudioQueueBufferRef qbuffer;
    OSStatus err = noErr;
    
    /* You need to call AudioRecorderStart() if you want record something */
    recorder->mIsRunning = false;
    
    /* Init Receiver Handler */
    recorder->mPacketReceivedHandler = packetReceivedHandler;
    recorder->mUserData = userData;
    
    /* Init Audio Format */
    _initAudioRecorderFormat(&(recorder->mDataFormat));

    /* Creates the recording audio queue object */
    err = AudioQueueNewInput(&recorder->mDataFormat, 
                             _handleInputBuffer,
                             recorder,
                             NULL,
                             kCFRunLoopCommonModes,
                             0,
                             &recorder->mQueue);
    if (err != noErr) return err;
    
    /* Allocate Audio Queue Buffer and enqueue it */
    for (int i = 0; i < kBufferNumQueues; ++i) {
        err = AudioQueueAllocateBuffer(recorder->mQueue, kBufferSize, &qbuffer);
        if (err != noErr) return err;
    
        err = AudioQueueEnqueueBuffer(recorder->mQueue, qbuffer, 0, NULL);
        if (err) return err;
    }
    return err;
}

OSStatus AudioRecorderDestroy (AudioRecorder *recorder) {
    AudioRecorderStop(recorder);
    return AudioQueueDispose(recorder->mQueue, false);
}

OSStatus AudioRecorderStart (AudioRecorder *recorder) {
    OSStatus err = noErr;
    
    if (!recorder->mIsRunning)
        err = AudioQueueStart(recorder->mQueue, NULL);

    recorder->mIsRunning = (err == noErr);
    return err;
}

OSStatus AudioRecorderStop  (AudioRecorder *recorder) {
    OSStatus err = noErr;
    
    if (recorder->mIsRunning)
        err = AudioQueueStop(recorder->mQueue, false);

    recorder->mIsRunning = (err != noErr);
    return err;
}
