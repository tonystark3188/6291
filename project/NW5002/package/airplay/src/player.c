/*
 * Slave-clocked ALAC stream player. This file is part of Shairport.
 * Copyright (c) James Laird 2011, 2013
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
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <pthread.h>
#include <openssl/aes.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <assert.h>
#include <fcntl.h>

#include "common.h"
#include "player.h"
#include "rtp.h"
#include "config.h"

#ifdef FANCY_RESAMPLING
#include <samplerate.h>
#endif

#include "alac.h"

//#define INTERNAL_RESAMPLING

// parameters from the source
static unsigned char *aesiv;
static AES_KEY aes;
static int sampling_rate, frame_size;

#define FRAME_BYTES(frame_size) (4*frame_size)
// maximal resampling shift - conservative
#define OUTFRAME_BYTES(frame_size) (4*(frame_size+3))

static pthread_t player_thread;
static int please_stop;

static alac_file *decoder_info;

#ifdef FANCY_RESAMPLING
static int fancy_resampling = 1;
static SRC_STATE *src;
#endif


// interthread variables
static double volume = 1.0;
static int fix_volume = 0x10000;
static pthread_mutex_t vol_mutex = PTHREAD_MUTEX_INITIALIZER;

// default buffer size
// needs to be a power of 2 because of the way BUFIDX(seqno) works
#define BUFFER_FRAMES	512
#define MAX_PACKET	2048

typedef struct audio_buffer_entry {   // decoded audio packets
    int ready;
    int n_bypass;
    int n_repeat;
    signed short *data;
} abuf_t;
static abuf_t audio_buffer[BUFFER_FRAMES];
#define BUFIDX(seqno) ((seq_t)(seqno) % BUFFER_FRAMES)

#ifdef CONFIG_MUTEBUFFER_ADJUST
// drop or add some mute buffer
static int muteDropGate = 65;
static int muteDrop = 16;
static int muteAdd  = 20;

struct mute_drop {
    int		mute_count;
    int		drop_count;
    int		add_count;
};

#define ABSOLUTE_MUTE	6
static struct mute_drop drop;
#endif /* CONFIG_MUTEBUFFER_ADJUST */

// mutex-protected variables
static seq_t ab_read, ab_write;
static int ab_buffering = 1, ab_synced = 0;
static pthread_mutex_t ab_mutex = PTHREAD_MUTEX_INITIALIZER;

#ifdef INTERNAL_RESAMPLING
static void bf_est_reset(short fill);
#endif

static void ab_resync(void) {
    int i;
    for (i=0; i<BUFFER_FRAMES; i++) {
        audio_buffer[i].ready = 0;
        audio_buffer[i].n_bypass = 0;
        audio_buffer[i].n_repeat = 0;
    }
    ab_synced = 0;
    ab_buffering = 1;

#ifdef CONFIG_MUTEBUFFER_ADJUST
    drop.mute_count = 0;
    drop.drop_count = 0;
    drop.add_count = 0;
#endif /* CONFIG_MUTEBUFFER_ADJUST */
}

// the sequence numbers will wrap pretty often.
// this returns true if the second arg is after the first
static inline int seq_order(seq_t a, seq_t b) {
    signed short d = b - a;
    return d > 0;
}

static void alac_decode(short *dest, uint8_t *buf, int len) {
    unsigned char packet[MAX_PACKET];
    assert(len<=MAX_PACKET);

    unsigned char iv[16];
    int aeslen = len & ~0xf;
    memcpy(iv, aesiv, sizeof(iv));
    AES_cbc_encrypt(buf, packet, aeslen, &aes, iv, AES_DECRYPT);
    memcpy(packet+aeslen, buf+aeslen, len-aeslen);

    int outsize;

    alac_decode_frame(decoder_info, packet, dest, &outsize);

    assert(outsize == FRAME_BYTES(frame_size));
}


static int init_decoder(int32_t fmtp[12]) {
    alac_file *alac;

    frame_size = fmtp[1]; // stereo samples
    sampling_rate = fmtp[11];

    int sample_size = fmtp[3];
    if (sample_size != 16)
        die("only 16-bit samples supported!");

    alac = alac_create(sample_size, 2);
    if (!alac)
        return 1;
    decoder_info = alac;

    alac->setinfo_max_samples_per_frame = frame_size;
    alac->setinfo_7a =      fmtp[2];
    alac->setinfo_sample_size = sample_size;
    alac->setinfo_rice_historymult = fmtp[4];
    alac->setinfo_rice_initialhistory = fmtp[5];
    alac->setinfo_rice_kmodifier = fmtp[6];
    alac->setinfo_7f =      fmtp[7];
    alac->setinfo_80 =      fmtp[8];
    alac->setinfo_82 =      fmtp[9];
    alac->setinfo_86 =      fmtp[10];
    alac->setinfo_8a_rate = fmtp[11];
    alac_allocate_buffers(alac);
    return 0;
}

static void free_decoder(void) {
    alac_free(decoder_info);
}

#ifdef FANCY_RESAMPLING
static int init_src(void) {
    int err;
    src = src_new(SRC_SINC_MEDIUM_QUALITY, 2, &err);

    return err;
}
static void free_src(void) {
    src_delete(src);
    src = NULL;
}
#endif

static void init_buffer(void) {
    int i;
    for (i=0; i<BUFFER_FRAMES; i++)
        audio_buffer[i].data = malloc(OUTFRAME_BYTES(frame_size));
    ab_resync();
}

static void free_buffer(void) {
    int i;
    for (i=0; i<BUFFER_FRAMES; i++)
        free(audio_buffer[i].data);
}

#ifdef CONFIG_MUTEBUFFER_ADJUST
/* Get audio data absolute value */
static inline signed short abs_sample(signed short j)
{
    return j < 0 ? (signed short)0 - j : j;
}

/**
 * audio_set_adjust -
 * @mute_add:
 * @mute_drop:
 * @drop_gate:
 *
 * Note:
 *	if value is 0, the value not change.
 */
void audio_set_adjust(int mute_add, int mute_drop, int drop_gate)
{
    if (mute_add)
        muteAdd  = mute_add;

    if (mute_drop)
        muteDrop = mute_drop;

    if (drop_gate)
        muteDropGate = drop_gate;

    fprintf(stdout, "shairport muteAdd:\t%d\n"
                    "\tmuteDrop:\t%d\n"
                    "\tmuteDropGate:\t%d\n",
                    muteAdd, muteDrop, muteDropGate);
}

/* Used for Ingenic */
static int audio_check_mute(abuf_t *abuf, int frames)
{
    signed short *a_data = abuf->data;
    int is_mute = 1;
    int i;

    for (i = 0; i < frames; i++) {
        if (abs_sample(a_data[i * 2]) > ABSOLUTE_MUTE ||
            abs_sample(a_data[i * 2 + 1]) > ABSOLUTE_MUTE) {
            is_mute = 0;
            break;
        }
    }

    return is_mute;
}

static int mp1 = 128;

/**
 * audio_mute_adjust -
 * @abuf:
 * @d:
 *
 * Note:
 *	for Ingenic to adjust mute buffer. add or drop some
 *	mute buffsers to avoid underrun and overrun.
 */
static void audio_mute_adjust(abuf_t *abuf, struct mute_drop *d)
{
    int buf_fill;
    int mute;

    buf_fill = (int)seq_diff(ab_read, ab_write) -
               d->drop_count + d->add_count;

    if (buf_fill < 0)
        buf_fill = 0;

    if (ab_buffering ||
        (buf_fill >= config.buffer_start_fill &&
        buf_fill <= config.buffer_start_fill + muteDropGate)) {
        d->mute_count = 0;
        return;
    }

    /* check buffer is mute */
    mute = audio_check_mute(abuf, frame_size);
    if (mute)
        d->mute_count++;
    else {
        d->mute_count = 0;
        return;
    }

    if (d->mute_count >= 125) {
        if (mp1) {
            printf("%s, in adjust %d\n", __func__, buf_fill);
        }

        if (buf_fill >= config.buffer_start_fill + muteDropGate) {
            audio_buffer[BUFIDX(ab_write - (muteDrop - 1))].n_bypass = muteDrop;
            d->drop_count += muteDrop;
	} else {
            audio_buffer[BUFIDX(ab_write)].n_repeat = muteAdd;
            d->add_count += muteAdd;
	}

        if (mp1) {
            printf("%s, result %d\n", __func__,
                   seq_diff(ab_read, ab_write) - d->drop_count + d->add_count);
            mp1--;
        }

        d->mute_count = 0;
    }
}
#endif /* CONFIG_MUTEBUFFER_ADJUST */

void player_put_packet(seq_t seqno, uint8_t *data, int len) {
    abuf_t *abuf = 0;
    int16_t buf_fill;

    pthread_mutex_lock(&ab_mutex);
    if (!ab_synced) {
        debug(2, "syncing to first seqno %04X\n", seqno);
        ab_write = seqno-1;
        ab_read = seqno;
        ab_synced = 1;
    }
    if (seq_diff(ab_write, seqno) == 1) {       // expected packet
        abuf = audio_buffer + BUFIDX(seqno);
        ab_write = seqno;
    } else if (seq_order(ab_write, seqno)) {    // newer than expected
        /* Only lost buffers NOT more than 1/3 seconds,
         * send RTP resend request.
         */
        if (seq_diff(ab_write + 1, seqno) <= 125 / 3)
            rtp_request_resend(ab_write+1, seqno-1);

        abuf = audio_buffer + BUFIDX(seqno);
        ab_write = seqno;
    } else if (seq_order(ab_read, seqno)) {     // late but not yet played
        abuf = audio_buffer + BUFIDX(seqno);
    } else {    // too late.
        debug(1, "late packet %04X (%04X:%04X)\n", seqno, ab_read, ab_write);
    }
    pthread_mutex_unlock(&ab_mutex);

    if (abuf) {
        alac_decode(abuf->data, data, len);
        abuf->ready = 1;
    }

    pthread_mutex_lock(&ab_mutex);
#ifdef CONFIG_MUTEBUFFER_ADJUST
    if (ab_write == seqno)
        audio_mute_adjust(abuf, &drop);
#endif /* CONFIG_MUTEBUFFER_ADJUST */

    buf_fill = seq_diff(ab_read, ab_write);
    if (ab_buffering && buf_fill >= config.buffer_start_fill) {
        debug(1, "buffering over. starting play\n");
        ab_buffering = 0;
#ifdef INTERNAL_RESAMPLING
        bf_est_reset(buf_fill);
#endif
    }
    pthread_mutex_unlock(&ab_mutex);
}


static short lcg_rand(void) {
	static unsigned long lcg_prev = 12345;
	lcg_prev = lcg_prev * 69069 + 3;
	return lcg_prev & 0xffff;
}

static inline short dithered_vol(short sample) {
    static short rand_a, rand_b;
    long out;

    out = (long)sample * fix_volume;
    if (fix_volume < 0x10000) {
        rand_b = rand_a;
        rand_a = lcg_rand();
        out += rand_a;
        out -= rand_b;
    }
    return out>>16;
}

#ifdef INTERNAL_RESAMPLING
typedef struct {
    double hist[2];
    double a[2];
    double b[3];
} biquad_t;

static void biquad_init(biquad_t *bq, double a[], double b[]) {
    bq->hist[0] = bq->hist[1] = 0.0;
    memcpy(bq->a, a, 2*sizeof(double));
    memcpy(bq->b, b, 3*sizeof(double));
}

static void biquad_lpf(biquad_t *bq, double freq, double Q) {
    double w0 = 2.0 * M_PI * freq * frame_size / (double)sampling_rate;
    double alpha = sin(w0)/(2.0*Q);

    double a_0 = 1.0 + alpha;
    double b[3], a[2];
    b[0] = (1.0-cos(w0))/(2.0*a_0);
    b[1] = (1.0-cos(w0))/a_0;
    b[2] = b[0];
    a[0] = -2.0*cos(w0)/a_0;
    a[1] = (1-alpha)/a_0;

    biquad_init(bq, a, b);
}

static double biquad_filt(biquad_t *bq, double in) {
    double w = in - bq->a[0]*bq->hist[0] - bq->a[1]*bq->hist[1];
    double out = bq->b[1]*bq->hist[0] + bq->b[2]*bq->hist[1] + bq->b[0]*w;
    bq->hist[1] = bq->hist[0];
    bq->hist[0] = w;

    return out;
}

static double bf_playback_rate = 1.0;

static double bf_est_drift = 0.0;   // local clock is slower by
static biquad_t bf_drift_lpf;
static double bf_est_err = 0.0, bf_last_err;
static biquad_t bf_err_lpf, bf_err_deriv_lpf;
static double desired_fill;
static int fill_count;

static void bf_est_reset(short fill) {
    biquad_lpf(&bf_drift_lpf, 1.0/180.0, 0.3);
    biquad_lpf(&bf_err_lpf, 1.0/10.0, 0.25);
    biquad_lpf(&bf_err_deriv_lpf, 1.0/2.0, 0.2);
    fill_count = 0;
    bf_playback_rate = 1.0;
    bf_est_err = bf_last_err = 0;
    desired_fill = fill_count = 0;
}

static void bf_est_update(short fill) {
    // the rate-matching system needs to decide how full to keep the buffer.
    // the initial fill is present when the system starts to output samples,
    // but most output chains will instantly gobble their own buffer's worth of
    // data. we average for a while to decide where to draw the line.
    if (fill_count < 1000) {
        desired_fill += (double)fill/1000.0;
        fill_count++;
        return;
    } else if (fill_count == 1000) {
        // this information could be used to help estimate our effective latency?
        debug(1, "established desired fill of %f frames, "
              "so output chain buffered about %f frames\n", desired_fill,
              config.buffer_start_fill - desired_fill);

        fprintf(stderr, "established desired fill of %f frames, "
              "so output chain buffered about %f frames\n", desired_fill,
              config.buffer_start_fill - desired_fill);

        fill_count++;
    }

#define CONTROL_A   (1e-4)
#define CONTROL_B   (1e-1)

    double buf_delta = fill - desired_fill;
    bf_est_err = biquad_filt(&bf_err_lpf, buf_delta);
    double err_deriv = biquad_filt(&bf_err_deriv_lpf, bf_est_err - bf_last_err);
    double adj_error = CONTROL_A * bf_est_err;

    bf_est_drift = biquad_filt(&bf_drift_lpf, CONTROL_B*(adj_error + err_deriv) + bf_est_drift);

    debug(3, "bf %d err %f drift %f desiring %f ed %f estd %f\n",
          fill, bf_est_err, bf_est_drift, desired_fill, err_deriv, err_deriv + adj_error);
    bf_playback_rate = 1.0 + adj_error + bf_est_drift;

    bf_last_err = bf_est_err;
}
#endif /* INTERNAL_RESAMPLING */

// get the next frame, when available. return 0 if underrun/stream reset.
static short *buffer_get_frame(void) {
    int16_t buf_fill;
    seq_t read;

    if (ab_buffering)
        return 0;

    pthread_mutex_lock(&ab_mutex);
#ifdef CONFIG_MUTEBUFFER_ADJUST
    if (audio_buffer[BUFIDX(ab_read)].ready) {
        /* Pass some buffers, need bypass to avoid overrun */
        if (audio_buffer[BUFIDX(ab_read)].n_bypass) {
            int n_bypass = audio_buffer[BUFIDX(ab_read)].n_bypass;
            audio_buffer[BUFIDX(ab_read)].n_bypass = 0;
            ab_read += n_bypass;
            drop.drop_count = drop.drop_count < n_bypass ?
                              0 : drop.drop_count - n_bypass;
        }

        /* Repeat use some times to avoid underrun */
        if (audio_buffer[BUFIDX(ab_read)].n_repeat) {
            audio_buffer[BUFIDX(ab_read)].n_repeat--;
            ab_read--;
            drop.add_count = drop.add_count < 1 ?
                             0 : drop.add_count - 1;
        }
    }
#endif /* CONFIG_MUTEBUFFER_ADJUST */

    buf_fill = seq_diff(ab_read, ab_write);
    if (buf_fill < 1 || !ab_synced) {
        if (buf_fill < 1)
            warn("underrun.");
        ab_buffering = 1;
        pthread_mutex_unlock(&ab_mutex);
        return 0;
    }
#ifdef FANCY_RESAMPLING
    if (buf_fill >= BUFFER_FRAMES - BUFFER_FRAMES / 5) {
        if (!fancy_resampling) {
            debug(1, "Active fancy resamplerate.\n");
            fprintf(stderr, "shairport. Active fancy resamplerate.\n");
            fancy_resampling = 1;
        }
    } else if (buf_fill <= config.buffer_start_fill) {
        if (fancy_resampling) {
            debug(1, "Deactive fancy resamplerate.\n");
            fprintf(stderr, "shairport. Deactive fancy resamplerate.\n");
            fancy_resampling = 0;
        }
    }
#endif
    if (buf_fill >= BUFFER_FRAMES) {   // overrunning! uh-oh. restart at a sane distance
        warn("overrun.");
        ab_read = ab_write - config.buffer_start_fill;
#ifdef CONFIG_MUTEBUFFER_ADJUST
	drop.drop_count = 0;
	drop.add_count = 0;
#endif /* CONFIG_MUTEBUFFER_ADJUST */
    }

    read = ab_read;
    ab_read++;

#ifdef INTERNAL_RESAMPLING
    buf_fill = seq_diff(ab_read, ab_write);
    bf_est_update(buf_fill);
#endif

    abuf_t *curframe = audio_buffer + BUFIDX(read);
    if (!curframe->ready) {
        debug(1, "missing frame %04X.\n", read);

	curframe->n_bypass	= 0;
	curframe->n_repeat	= 0;
        memset(curframe->data, 0, FRAME_BYTES(frame_size));
    }

    curframe->ready = 0;
    pthread_mutex_unlock(&ab_mutex);

    return curframe->data;
}

#ifdef INTERNAL_RESAMPLING
static int stuff_buffer(double playback_rate, short *inptr, short *outptr) {
    int i;
    int stuffsamp = frame_size;
    int stuff = 0;
    double p_stuff;

    p_stuff = 1.0 - pow(1.0 - fabs(playback_rate-1.0), frame_size);

    if (rand() < p_stuff * RAND_MAX) {
        stuff = playback_rate > 1.0 ? -1 : 1;
        stuffsamp = rand() % (frame_size - 1);
    }

    pthread_mutex_lock(&vol_mutex);
    for (i=0; i<stuffsamp; i++) {   // the whole frame, if no stuffing
        *outptr++ = dithered_vol(*inptr++);
        *outptr++ = dithered_vol(*inptr++);
    };
    if (stuff) {
        if (stuff==1) {
            debug(2, "+++++++++\n");
            // interpolate one sample
            *outptr++ = dithered_vol(((long)inptr[-2] + (long)inptr[0]) >> 1);
            *outptr++ = dithered_vol(((long)inptr[-1] + (long)inptr[1]) >> 1);
        } else if (stuff==-1) {
            debug(2, "---------\n");
            inptr++;
            inptr++;
        }
        for (i=stuffsamp; i<frame_size + stuff; i++) {
            *outptr++ = dithered_vol(*inptr++);
            *outptr++ = dithered_vol(*inptr++);
        }
    }
    pthread_mutex_unlock(&vol_mutex);

    return frame_size + stuff;
}
#else
static int stuff_buffer_simple(short *inptr, short *outptr) {
    int i;

    /* the whole frame, if no stuffing */
    pthread_mutex_lock(&vol_mutex);
    for (i = 0; i < frame_size; i++) {
        *outptr++ = dithered_vol(*inptr++);
        *outptr++ = dithered_vol(*inptr++);
    }
    pthread_mutex_unlock(&vol_mutex);

    return frame_size;
}
#endif

static void *player_thread_func(void *arg) {
    int play_samples;

    signed short *inbuf, *outbuf, *silence;
    outbuf = malloc(OUTFRAME_BYTES(frame_size));
    silence = malloc(OUTFRAME_BYTES(frame_size));
    memset(silence, 0, OUTFRAME_BYTES(frame_size));

#ifdef FANCY_RESAMPLING
    float *frame, *outframe;
    SRC_DATA srcdat;
    frame = malloc(frame_size * sizeof(float) * 2);
    outframe = malloc(2 * frame_size * sizeof(float) * 2);

    srcdat.data_in		= frame;
    srcdat.data_out		= outframe;
    srcdat.input_frames		= frame_size;
    srcdat.output_frames	= 2 * frame_size;
    srcdat.src_ratio		= 1.0;
    srcdat.end_of_input		= 0;

    /* Init resampling at deactive */
    pthread_mutex_lock(&vol_mutex);
    fancy_resampling = 0;
    pthread_mutex_unlock(&vol_mutex);
#endif

    while (!please_stop) {
        inbuf = buffer_get_frame();
#if 0
        if (!inbuf)
            inbuf = silence;
#endif
        if (!inbuf) {
            usleep(1000);
            continue;
        }

#ifdef FANCY_RESAMPLING
        if (fancy_resampling) {
            int i;
            pthread_mutex_lock(&vol_mutex);
            for (i = 0; i < frame_size * 2; i++) {
                frame[i] = (float)(inbuf[i]) / 32768.0;
                frame[i] *= volume;
            }
            pthread_mutex_unlock(&vol_mutex);

            srcdat.src_ratio	= (1.0 - 0.018);
            src_process(src, &srcdat);
            assert(srcdat.input_frames_used == frame_size);
            src_float_to_short_array(outframe, outbuf, srcdat.output_frames_gen * 2);
            play_samples = srcdat.output_frames_gen;
        } else
#endif
#ifdef INTERNAL_RESAMPLING
            play_samples = stuff_buffer(bf_playback_rate, inbuf, outbuf);
#else
            play_samples = stuff_buffer_simple(inbuf, outbuf);
#endif /* INTERNAL_RESAMPLING */

        config.output->play(outbuf, play_samples);
    }

    free(outbuf);
    free(silence);
#ifdef FANCY_RESAMPLING
    free(frame);
    free(outframe);
#endif

    return 0;
}

// takes the volume as specified by the airplay protocol
void player_volume(double f) {
    if (config.output->volume) {
        /* Airplay volume is a float value representing the audio
         * attenuation in dB. A value of -144 means the audio is
         * muted. Then it goes from -30 to 0.
         */
	double vol = f != (-144.0) ? ((-30.0) - f) / (-30.0) : 0;
        config.output->volume(vol);
    } else {
        double linear_volume = pow(10.0, 0.05*f);
        pthread_mutex_lock(&vol_mutex);
        volume = linear_volume;
        fix_volume = 65536.0 * volume;
        pthread_mutex_unlock(&vol_mutex);
    }
}
void player_flush(void) {
    pthread_mutex_lock(&ab_mutex);
    ab_resync();
    pthread_mutex_unlock(&ab_mutex);
}

int player_play(stream_cfg *stream) {
    if (config.buffer_start_fill > BUFFER_FRAMES)
        die("specified buffer starting fill %d > buffer size %d",
            config.buffer_start_fill, BUFFER_FRAMES);

    AES_set_decrypt_key(stream->aeskey, 128, &aes);
    aesiv = stream->aesiv;
    init_decoder(stream->fmtp);
    // must be after decoder init
    init_buffer();
#ifdef FANCY_RESAMPLING
    init_src();
#endif

    please_stop = 0;
    command_start();
    config.output->start(sampling_rate);
    if (pthread_create(&player_thread, NULL, player_thread_func, NULL) != 0)
        die("Create player_thread: %s", strerror(errno));

    return 0;
}

void player_stop(void) {
    please_stop = 1;
    pthread_join(player_thread, NULL);
    config.output->stop();
    command_stop();
    free_buffer();
    free_decoder();
#ifdef FANCY_RESAMPLING
    free_src();
#endif
}

void fade_in_volume(unsigned int fade_msec, unsigned int fade_volume)
{
	double linear_volume = 1.0;
	double step = (double)fade_msec / 100000;

	if (!step) {
		pthread_mutex_lock(&vol_mutex);
		fix_volume = 65536.0 * fade_volume;
		pthread_mutex_unlock(&vol_mutex);
		return;
	}

	for(linear_volume = 1.0; linear_volume >= fade_volume-0.000001; linear_volume -= step){
		pthread_mutex_lock(&vol_mutex);
		fix_volume = 65536.0 * linear_volume;
		pthread_mutex_unlock(&vol_mutex);
		usleep(step * 1000000);
	}
}

void fade_out_volume(unsigned int fade_msec)
{
	double linear_volume = 0.0;
	double step = (double)fade_msec / 100000;

	if (!step) {
		pthread_mutex_lock(&vol_mutex);
		fix_volume = 65536.0 * 1.0;
		pthread_mutex_unlock(&vol_mutex);
		return;
	}

	for(linear_volume = 0.0; linear_volume <= 1.000001; linear_volume += step){
		pthread_mutex_lock(&vol_mutex);
		fix_volume = 65536.0 * linear_volume;
		pthread_mutex_unlock(&vol_mutex);
		usleep(step * 1000000);
	}
}
