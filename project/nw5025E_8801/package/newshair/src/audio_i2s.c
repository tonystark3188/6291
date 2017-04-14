/*
 * i2s output driver. This file is part of Shairport.
 * Copyright (c) James Laird 2013
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <openssl/aes.h>
#include <math.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "audio.h"


static void stop(void);
//int Fs;
//long long starttime, samples_played;
static int audio=0;
static char *audev = "/dev/i2s";

#define I2S_BUF 1024


#define I2S_VOLUME      _IOW('N', 0x20, int)
#define I2S_FREQ        _IOW('N', 0x21, int)
#define I2S_DSIZE       _IOW('N', 0x22, int)
#define I2S_MODE        _IOW('N', 0x23, int)
#define I2S_FINE        _IOW('N', 0x24, int)
#define I2S_COUNT       _IOWR('N', 0x25, int)
#define I2S_PAUSE       _IOWR('N', 0x26, int)
#define I2S_RESUME      _IOWR('N', 0x27, int)
#define I2S_MCLK        _IOW('N', 0x28, int)
#define I2S_MUTE      _IOW('N', 0x29, int)
#define I2S_BUFFER      _IOW('N', 0x2a, int)
int __UsbSoundCard = 0;
static int hairwrite(int fd, char *buf, int len)
{
	int num_bytes=len;
	int send;
	int ret;
	char *output_samples;

	output_samples = buf;
	
	if(__UsbSoundCard)
	    alsa_play(buf,num_bytes);
	while(num_bytes > 0) {
          send = num_bytes>I2S_BUF? I2S_BUF:num_bytes;
           
          ret = write(fd, output_samples, send);

          if (ret < 0)
          {
          	usleep(50000);
			printf("write error!\n"); 
			ret = write(fd, output_samples, send);
			return ret;
          }
          num_bytes-=ret;
          output_samples+=ret;
	}
}


static int init(int argc, char **argv) {

    argc = argc<8000?44100:argc;
    	
	if(audio == 0)
	{
		audio = open (audev, O_WRONLY);
		__UsbSoundCard = 1;
	    alsa_init(NULL,NULL);
		if (audio < 0) 
		{	
			printf("open device failed,try again:\n");
			sleep(4);//try again
			audio = open (audev, O_WRONLY);
			 if (audio < 0) 
			 {
			 	  die("Device opening failed\n");
			    return 1;
			 }
		}
		
		
		if (ioctl(audio, I2S_DSIZE, 16) < 0) 
		{		
			die("I2S_DSIZE");
			return 1;
		}	


		if (ioctl(audio, I2S_FREQ, argc) < 0) 
		{		
			die("I2S_FREQ");	
			return 1;
		}


		if (ioctl(audio, I2S_MODE, 2) < 0) 
		{		
			die("I2S_MODE");	
			return 1;
		}
		
		if (ioctl(audio, I2S_MUTE, 0) < 0) 
		{
			die("I2S_MUTE err");
			return 1;
		}
		if (ioctl(audio, I2S_VOLUME, 80) < 0)//set max volume
		{
			die("set vol err\n");
			return 1;
		}
        if(__UsbSoundCard)
            alsa_start(argc,2,16,100);
	    return 0;
	}
	else
		return 1;
}

static void deinit(void) {
   stop();
}

static void start(int sample_rate) {
   /* Fs = sample_rate;
    starttime = 0;
    samples_played = 0;*/
    printf("i2s audio output started at Fs=%d Hz\n", sample_rate);
}

static void play(short buf[], int samples) {
	#if 0
    struct timeval tv;

    // this is all a bit expensive but it's long-term stable.
    gettimeofday(&tv, NULL);

    long long nowtime = tv.tv_usec + 1e6*tv.tv_sec;

    if (!starttime)
        starttime = nowtime;

    samples_played += samples;

    long long finishtime = starttime + samples_played * 1e6 / Fs;

    usleep(finishtime - nowtime);
	#endif

	if(audio)
		hairwrite(audio,buf, samples*4);
}

static void stop(void) {
	int ret = 0;
	if(audio != 0)
	{
				
		if (ioctl(audio, I2S_MUTE, 1) < 0) 
		{
			die("I2S_MUTE err 111");
			//return -1;
		}
		ret = close(audio);
		audio = 0;
	}
	if(__UsbSoundCard)
		alsa_deinit();
    printf("i2s audio stopped\n");
}

static void help(void) {
    printf("    There are no options for i2s audio.\n");
}
static int havemute = 0;
static void setvolume(int vol)
{
     int volume = vol;
    if(audio)
    {
         if(volume > 0 && volume < 80)
			volume = 80 - 5*((80 - volume)/10);
         if(!volume && !havemute)
         {      
                havemute = 1;
             	if (ioctl(audio, I2S_MUTE, 1) < 0) //mute
		{
			die("I2S_MUTE err");
		}   
         }
         else if(havemute)
         {
                havemute = 0;
             	if (ioctl(audio, I2S_MUTE, 0) < 0) //mute
		{
			die("I2S_MUTE err");
		}   
         }
         if (ioctl(audio, I2S_VOLUME, volume) < 0)//set max volume
        {
             die("set vol err\n");
             return 1;
        }    
    }
}

audio_output audio_i2s = {
    .name = "i2s",
    .help = &help,
    .init = &init,
    .deinit = &deinit,
    .start = &start,
    .stop = &stop,
    .play = &play,
    .volume = &setvolume
};
