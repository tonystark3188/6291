#ifndef __PCM2MP3_H
#define __PCM2MP3_H
#include "common.h"

struct MP3param{
	int fd;
	int channel;
};

int initLameEncode(int speed,int channels);
int deInitLameEncode(int mp3fd);
int createEncodeThread(struct MP3param mp3_param);

#if HAVEENCODETHREAD
void *encode2Mp3(void *arg);
#else
int encode2Mp3(int mp3fd, short *pcm_buffer,int len);
#endif

void setRecordFlag(int value);

#endif
