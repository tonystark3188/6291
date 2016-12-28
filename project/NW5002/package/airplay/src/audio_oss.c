/*
 * oss output driver. This file is part of Shairport.
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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/soundcard.h>

#include "common.h"
//#include "volume_interface.h"

#define DEVICE_NAME	"shairport oss"

#if 0
#define DL(x,y...)	{fprintf(stderr, "%s debug: "x"\n", DEVICE_NAME, ##y);}
#else
#define DL(x,y...)
#endif
#define EL(x,y...)	{fprintf(stderr, "%s: "x"\n", DEVICE_NAME, ##y);}

#define OSS_WITH_NONBLOCK
#define DSP_LOW_DELAY
#define VOL_MAX 100

/* default 20 millisecond buffer */
#define AO_OSS_BUFFER_TIME 20000

typedef struct ao_sample_format {
	int	channels;	/* number of audio channels */
	int	bits;		/* bits per sample */
	int	format;		/* oss format */
	int	rate;		/* samples per second (in a single channel) */
} ao_sample_format;

typedef struct ao_oss_internal {
	char		*dev;
	int		id;
	int		fd;
	int		blocking;
	unsigned int	buffer_time;
	int		outburst;
	int		buffersize;
} ao_oss_internal;

struct ao_oss_internal oss_internal;
static void stop(void);

static int bits2format(int bits)
{
    switch(bits) {
    case 8: return AFMT_S8;
    case 16: return AFMT_S16_LE;
    }

    return -1;
}

/*
 * open either the devfs device or the traditional device and return a
 * file handle.  Also strdup() path to the selected device into
 * *dev_path.  Assumes that *dev_path does not need to be free()'ed
 * initially.
 *
 * If OSS_WITH_NONBLOCK is defined, this opens the device in non-blocking
 * mode at first in order to prevent deadlock caused by ALSA's OSS
 * emulation and some OSS drivers if the device is already in use.  If
 * blocking is non-zero, we remove the blocking flag if possible so
 * that the device can be used for actual output.
 *
 * Copied from libao-1.2.0 OSS Plugin
 */
static int open_default_oss_device(char **dev_path, int id, int blocking)
{
    int fd;
    char buf[80];

    /* default: first try the devfs path */
    if (id > 0) {
        sprintf(buf,"/dev/sound/dsp%d",id);
        if (!(*dev_path = strdup(buf)))
            return -1;
    } else {
        if (!(*dev_path = strdup("/dev/sound/dsp")))
            return -1;
    }

#ifdef OSS_WITH_NONBLOCK
    fd = open(*dev_path, O_WRONLY | O_NONBLOCK);
#else
    fd = open(*dev_path, O_WRONLY);
#endif /* OSS_WITH_NONBLOCK */

    /* then try the original dsp path */
    if (fd < 0) {
        /* no? then try the traditional path */
        free(*dev_path);
        if (id > 0) {
            sprintf(buf,"/dev/dsp%d",id);
            if (!(*dev_path = strdup(buf)))
                return -1;
        } else {
            if (!(*dev_path = strdup("/dev/dsp")))
                return -1;
        }
#ifdef OSS_WITH_NONBLOCK
        fd = open(*dev_path, O_WRONLY | O_NONBLOCK);
#else
        fd = open(*dev_path, O_WRONLY);
#endif /* OSS_WITH_NONBLOCK */
    }

#ifdef OSS_WITH_NONBLOCK
    /* Now have to remove the O_NONBLOCK flag if so instructed. */
    if (fd >= 0 && blocking) {
        if (fcntl(fd, F_SETFL, 0) < 0) { /* Remove O_NONBLOCK */
            /* If we can't go to blocking mode, we can't use
               this device */
	    close(fd);
            fd = -1;
        }
    }
#endif /* OSS_WITH_NONBLOCK */

    /* Deal with error cases */
    if (fd < 0) {
        /* could not open either default device */
        free(*dev_path);
        *dev_path = NULL;
    }

    return fd;
}

#ifdef DSP_LOW_DELAY
static int ilog(long x)
{
    int ret = -1;

    while (x > 0) {
        x >>= 1;
        ret++;
    }

    return ret;
}
#endif

/**
 * oss_device_open - 
 * @ao_oss:
 * @fmt:
 */
static int oss_device_open(ao_oss_internal *ao_oss, ao_sample_format *fmt)
{
    static audio_buf_info zz;
    int oss_rate;
#ifdef DSP_LOW_DELAY
    long bytesperframe;
    int fraglog, fragment, fragcheck;
#endif

    if (ao_oss->dev) {
        if (ao_oss->blocking)
            ao_oss->fd = open(ao_oss->dev, O_WRONLY);
        else
#ifdef OSS_WITH_NONBLOCK
            ao_oss->fd = open(ao_oss->dev, O_WRONLY | O_NONBLOCK);
#else
            ao_oss->fd = open(ao_oss->dev, O_WRONLY);
#endif

        if (ao_oss->fd < 0) {
            EL("Can not open \'%s\'", ao_oss->dev);
            goto err_open;
        }
    } else {
        ao_oss->fd = open_default_oss_device(&ao_oss->dev, ao_oss->id, ao_oss->blocking);
        if (ao_oss->fd < 0) {
            EL("Can not open oss device");
            goto err_open;
        }
    }

#ifdef DSP_LOW_DELAY
    /* try to lower the DSP delay; this ioctl may fail gracefully */
    bytesperframe = (fmt->bits + 7) / 8 *
                    fmt->channels *
                    fmt->rate * (ao_oss->buffer_time / 1000000.);
    fraglog = ilog(bytesperframe);
    fragment = 0x00040000 | fraglog;
    fragcheck=fragment;

    if (ioctl(ao_oss->fd, SNDCTL_DSP_SETFRAGMENT, &fragment) ||
        fragcheck != fragment) {
        EL("Could not set DSP fragment size; continuing.");
    }
#endif

    /* Now set all of the parameters */

    /* We only use SNDCTL_DSP_CHANNELS for >2 channels,
     * in case some drivers don't have it.
     */
    if (fmt->channels > 2) {
	int channels = fmt->channels;
        if (ioctl(ao_oss->fd, SNDCTL_DSP_CHANNELS, &channels) < 0 ||
            channels != fmt->channels) {
            EL("Cannot set channels to %d", fmt->channels);
            goto err_out;
        }
    } else {
	int channels = fmt->channels - 1;
	if (ioctl(ao_oss->fd, SNDCTL_DSP_STEREO, &channels) < 0 ||
	    channels + 1 != fmt->channels) {
            EL("Cannot set channels to %d", fmt->channels);
            goto err_out;
	}
    }

    if (ioctl(ao_oss->fd, SNDCTL_DSP_SETFMT, &fmt->format) < 0) {
        EL("Cannot set format to %d", fmt->bits);
	goto err_out;
    }

    /* Some cards aren't too accurate with their clocks and set to the
     * exact data rate, but something close.  Fail only if completely out
     * of whack.
     */
    oss_rate = fmt->rate;
    if (ioctl(ao_oss->fd, SNDCTL_DSP_SPEED, &oss_rate) < 0 ||
        oss_rate > 1.02 * fmt->rate ||
        oss_rate < 0.98 * fmt->rate) {
        EL("Cannot set rate to %d, get result %d", fmt->rate, oss_rate);
        goto err_out;
    }

    /* Update OSS buffer size */
    if (ioctl(ao_oss->fd, SNDCTL_DSP_GETOSPACE, &zz) < 0) {
        EL("Cannot get oss buffer info");
        ao_oss->buffersize = 0;
        goto err_out;
    }
    ao_oss->buffersize = zz.fragstotal * zz.fragsize;

    /* this calculates and sets the fragment size */
    ao_oss->outburst = -1;
    if ((ioctl(ao_oss->fd, SNDCTL_DSP_GETBLKSIZE, &(ao_oss->outburst)) < 0) ||
        ao_oss->outburst <= 0) {
        /* Some versions of the *BSD OSS drivers use a subtly
         * different SNDCTL_DSP_GETBLKSIZE ioctl implementation
         * which is, oddly, incompatible with the shipped
         * declaration in soundcard.h.  This ioctl isn't necessary
         * anyway, it's just tuning.  Soldier on without,
         */
        EL("Cannot get buffer size for device; using a default of 1024kB");
        ao_oss->outburst = 1024;
    }

    return 1; /* Open successful */

err_out:
    if (ao_oss->fd >= 0) {
        close(ao_oss->fd);
        ao_oss->fd = -1;
    }

    if (ao_oss->dev) {
        free(ao_oss->dev);
        ao_oss->dev = NULL;
    }

err_open:
    return 0; /* Failed to open device */
}

/**
 * init -
 * 
 * NOTE:
 */
static int init(int argc, char **argv)
{
    oss_internal.dev		= NULL;
    oss_internal.id		= -1;
    oss_internal.fd		= -1;
    oss_internal.blocking	= 1;
    oss_internal.buffer_time	= AO_OSS_BUFFER_TIME;

    return 1;
}

static void deinit(void)
{
    ao_oss_internal *ao_oss = &oss_internal;

    if (ao_oss->fd >= 0)
        stop();

    if (ao_oss->dev) {
        free(ao_oss->dev);
        ao_oss->dev = NULL;
    }
}

static void start(int sample_rate)
{
    ao_oss_internal *ao_oss = &oss_internal;

    if (sample_rate != 44100)
        die("unexpected sample rate!\n");

    if (ao_oss->fd < 0) {
        struct ao_sample_format fmt;
        memset(&fmt, 0, sizeof(fmt));

        fmt.bits	= 16;
        fmt.format	= bits2format(fmt.bits);
        fmt.rate	= 44100;
        fmt.channels	= 2;

        oss_device_open(ao_oss, &fmt);
    }
}

static void play(short buf[], int samples)
{
    ao_oss_internal *ao_oss = &oss_internal;
    static audio_buf_info zz;
    char *out_buffer = (char *)buf;
    int num_bytes = samples * 4;
    int send;
    int retval;

    if (ao_oss->fd < 0)
            return;

    while (num_bytes > 0) {
        if (ioctl(ao_oss->fd, SNDCTL_DSP_GETOSPACE, &zz) == -1) {
            EL("%s: Can not get availade space.", __func__);
            return;
        }

        if (zz.bytes < ao_oss->outburst) {
            usleep(100);
            continue;
        }

        send = num_bytes > ao_oss->outburst ?
               ao_oss->outburst : num_bytes;
        retval = write(ao_oss->fd, out_buffer, send);
        if (retval < 0){
            if(errno == -EINTR) continue;
            return;
        }

        num_bytes -= retval;
        out_buffer += retval;
    }
}

static void stop(void)
{
    ao_oss_internal *ao_oss = &oss_internal;

    if (ao_oss->fd >= 0) {
        /* to get the buffer played */
        ioctl(ao_oss->fd, SNDCTL_DSP_SYNC, NULL);

        close(ao_oss->fd);
        ao_oss->fd = -1;
    }
}

static void volume(double vol)
{
    unsigned int volume = vol * VOL_MAX;
    DL("vol = %lf", vol);
    DL("vol = %u", volume);
    //mozart_volume_set(volume, MUSIC_VOLUME);
}

static void dev_try(int *is_try)
{
    ao_oss_internal *ao_oss = &oss_internal;
    char *oss_dev = ao_oss->dev ? ao_oss->dev : "/dev/dsp";
    int try_fd;

    if (ao_oss->fd >= 0)
        return;

    *is_try = 1;
    while (*is_try) {
        try_fd = open(oss_dev, O_WRONLY);
        if (try_fd >= 0) {
            close(try_fd);
            break;
        }

        /* sleep 100ms */
        usleep(100 * 1000);
    }
    *is_try = 0;
}

static void help(void)
{
    EL("There are no options for oss audio.\n");
}

audio_output audio_oss = {
    .name = "oss",
    .help = &help,
    .init = &init,
    .deinit = &deinit,
    .start = &start,
    .stop = &stop,
    .play = &play,
    .volume = &volume,
    .dev_try = &dev_try,
};
