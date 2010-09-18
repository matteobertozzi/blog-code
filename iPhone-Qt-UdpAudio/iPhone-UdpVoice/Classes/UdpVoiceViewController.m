//
//  UdpVoiceViewController.m
//  UdpVoice
//
//  Created by Matteo Bertozzi on 7/10/10.
//  Copyright Matteo Bertozzi 2010. All rights reserved.
//

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#import "UdpVoiceViewController.h"

#define kUdpVoicePort           55455
//#define kUdpVoiceReceiver       "127.0.0.1"
#define kUdpVoiceReceiver       "192.168.3.1"

static int _initUdpSocket (struct sockaddr_in *saddr) {
    int sock;
    
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        return(-1);
    
    memset(saddr, 0, sizeof(struct sockaddr_in));
    saddr->sin_family = AF_INET;
    saddr->sin_port = htons(kUdpVoicePort);
    
    if (!inet_aton(kUdpVoiceReceiver, &saddr->sin_addr)) {
        close(sock);
        return(-1);
    }
    
    return(sock);
}

static void _audioRecorderPacketReceived (const void *audioData,
                                         UInt32 audioDataByteSize,
                                         void *userData)
{
    UdpVoiceData *data = (UdpVoiceData *)userData;

    dispatch_async(data->netqueue, ^{
        socklen_t slen = sizeof(data->saddr);
        socklen_t optlen = sizeof(int);
        int optval = 8192;
        int sended, avail;
        int x;
        
        getsockopt(data->socket, SOL_SOCKET, SO_SNDBUF, (int *)&optval, &optlen);
        
        sended = 0;
        avail = audioDataByteSize;
        while (avail > 0) {
            x = sendto(data->socket, audioData + sended, avail < optval ? avail : optval,
                        0, (struct sockaddr *)&data->saddr, slen);
            if (x < 1) {
                printf("ERROR X < 1: %d\n", x);
                perror("sendto()");
                break;
            }
            sended += x;
            avail -= x;
        }
    });
    
#ifdef STREAM_DEBUG
    write(data->stream, audioData, audioDataByteSize);
#endif
}

@implementation UdpVoiceViewController

-(void)alertViewWithTitle:(NSString *)title andMessage:(NSString *)message {
  UIAlertView *alert = [[UIAlertView alloc] initWithTitle:title message:message delegate:nil cancelButtonTitle:nil otherButtonTitles:@"Ok", nil];
  [alert show];
  [alert release];
}

// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

- (void)startRecording {
    // Setup Udp Network
    if ((data.socket = _initUdpSocket(&data.saddr)) < 0) {
        [self alertViewWithTitle:@"UdpSocket() Failed" andMessage:[NSString stringWithUTF8String:strerror(errno)]];
        return;
    }
    
    data.netqueue = dispatch_queue_create("com.th30z.udpvoice", NULL);
    
    AudioRecorderInit(&data.recorder, _audioRecorderPacketReceived, &data);
    AudioRecorderStart(&data.recorder);
    
#ifdef STREAM_DEBUG
    // Open Debug Stream
    if ((data.stream = open(STREAM_DEBUG_FILENAME, O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0)
        [self alertViewWithTitle:@"Stream Debug Open Failed" andMessage:[NSString stringWithUTF8String:strerror(errno)]];
#endif
}

- (void)stopRecording {
    AudioRecorderStop(&data.recorder);
    AudioRecorderDestroy(&data.recorder);

    dispatch_release(data.netqueue);
    close(data.socket);

#ifdef STREAM_DEBUG
    if (data.stream >= 0)
        close(data.stream);
#endif
}

@end
