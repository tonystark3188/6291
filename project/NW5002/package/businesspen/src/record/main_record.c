/*
 * Author: hu.jiang <hu.jiang@longsys.com>
 *
 * record pcm and convert to WAV/RAW/MP3
 */
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

#include <lame/lame.h>
#include <speex/speex_preprocess.h>

#include "common.h"



#include "headers.h"
#include "codec_route_name.h"
#include "mysignal.h"
#include "record_list.h"
#include "pcm2mp3.h"
#include "audio_format_convert.h"


/* If you don't want to use the default route in kernel, you can define it to 1 and should refer the route's name defined in kernel. Because different codec has some different routes. */

#define SET_CODEC_ROUTE  1

#define AFMT_S24_LE		0x00000800


#define ARRAY_SIZE(x) (sizeof((x))/sizeof((x)[0]))

/* my own header files */
#include "wav.h"

#define FILE_TYPE_WAV	0
#define FILE_TYPE_RAW	1
#define FILE_TYPE_MP3	2
#define FILE_TYPE_AMR	3
#define FILE_TYPE_WAV_MY 4


#define FMT8BITS	AFMT_U8
#define FMT16BITS	AFMT_S16_LE
#define FMT24BITS	AFMT_S24_LE

#define DEFAULT_SND_DEV		"/dev/dsp"
#define DEFAULT_MIXER_DEV	"/dev/mixer"
#define DEFAULT_SND_DEV1	"/dev/dsp1"
#define DEFAULT_MIXER_DEV1	"/dev/mixer1"
#define DEFAULT_SND_DEV2	"/dev/dsp2"
#define DEFAULT_MIXER_DEV2	"/dev/mixer2"
#define DEFAULT_SND_DEV3	"/dev/dsp3"
#define DEFAULT_MIXER_DEV3	"/dev/mixer3"
#define LINEIN_DEV              "/dev/linein"  

#define DEFAULT_SND_SPD 44100
#define DEFAULT_SND_CHN 2

/* unit: s */
#define DEFAULT_DURATION	10
#define FOREVER_DURATION	9999


#define SND_FMT_S8		0
#define SND_FMT_U8		1
#define SND_FMT_S16_LE		2
#define SND_FMT_S16_BE		3
#define SND_FMT_U16_LE		4
#define SND_FMT_U16_BE		5
#define SND_FMT_S24_3LE		6
#define SND_FMT_S24_3BE		7
#define SND_FMT_U24_3LE		8
#define SND_FMT_U24_3BE		9

int debug_level = 1;

/* now sndkit only support: U8, S16_LE, S24_3LE, other formats hasnot be support */
const char *supported_fmts[] = {
	"S8",
	"U8",
	"S16_LE",
	"S16_BE",
	"U16_LE",
	"U16_BE",
	"S24_3LE",
	"S24_3BE",
	"U24_3LE",
	"U24_3BE",
};

#define LED_ON 				0x11
#define LED_OFF				0x10


static int sign_adjust = 0;
static int endian_adjust = 0;
static int g_fdLed = 0;
FILE *g_fpamr = NULL;

static int fmtstr_to_fmt(char *fmtstr) {
	unsigned int i = 0;
	int fmt;

	for (i = 0; i < ARRAY_SIZE(supported_fmts); i++) {
		if (!strcmp(supported_fmts[i], fmtstr))
			break;
	}

	if (i == ARRAY_SIZE(supported_fmts))
		return -1;

	sign_adjust = 0;
	endian_adjust = 0;

	switch (i) {
	case SND_FMT_U8:
		sign_adjust = 1;
	case SND_FMT_S8:
		fmt = FMT8BITS;
		break;

	case SND_FMT_U16_BE:
		endian_adjust = 1;
	case SND_FMT_U16_LE:
		sign_adjust = 1;
		fmt = FMT16BITS;
		break;

	case SND_FMT_S24_3BE:
		endian_adjust = 1;
	case SND_FMT_S24_3LE:
		fmt = FMT24BITS;
		break;

	case SND_FMT_U24_3BE:
		endian_adjust = 1;
	case SND_FMT_U24_3LE:
		sign_adjust = 1;
		fmt = FMT24BITS;
		break;

	case SND_FMT_S16_BE:
		endian_adjust = 1;
	case SND_FMT_S16_LE:
	default:
		fmt = FMT16BITS;
	}

	return fmt;
}

static int support_rates[] = {
	8000,
	11025,
	12000,
	16000,
	22050,
	24000,
	32000,
	44100,
	48000,
	96000,
	192000,
};

static int rate_is_supported(int rate) {
	unsigned int i = 0;

	for (i = 0; i < ARRAY_SIZE(support_rates); i++)
		if (support_rates[i] == rate)
			return 1;

	return 0;
}

struct replay_params {
	int dev_fd;
	int replay_fd;
	int buffer_size;
	int replay_data_len;
	int format;
	int channels;
	int speed;
	int file_type;
};

struct record_params {
	int dev_fd;
	int record_fd;
	int buffer_size;
	int duration;
	int format;
	int channels;
	int speed;
	int file_type;
};

#define SNDKIT_VERSION	"dmicRecord 1.0, 2015-12-17"

static struct option long_options[] = {
	{ "help", 0, 0, 'h' },
	{ "version", 0, 0, 'V' },
	{ "makewav", 0, 0, 'W' },
	{ "strip", 0, 0, 'T' },
	{ "use-linein", 0, 0, 'L' },

	{ "device", 1, 0, 'D' },
	{ "file-type", 1, 0, 't' },
	{ "channels", 1, 0, 'c' },
	{ "format", 1, 0, 'f' },
	{ "rate", 1, 0, 'r'},
	{ "duration", 1, 0, 'd' },
	{ "buffer-size", 1, 0, 'B' },
	{ "linein-bypass", 0, 0, 'l'},

	{ "replay", 0, 0, 'P' },
	{ "replay-file", 1, 0, 'p' },

	{ "record", 0, 0, 'R' },
	{ "record-file", 1, 0, 'w' },

	{ "volume", 1, 0, 'o' },

	{0, 0, 0, 0}
};

static char optstring[] = "+hVWTLD:t:c:f:r:d:B:lPp:Rw:o:";//:表示后面必须跟参数

static char g_record_file[128]={0};
static int g_record_flag = 0;
static int g_record_exit = 0;
static struct record_params g_record_params;

int mypcm2amr(FILE *fpamr,char *buffer,int ret)
{
    int j=0;
	int cur_point=0;
	char in_data[320] = {0};

	if(ret>0)
	{
		//ret must be multiple of 4
		
		ret = ret/4;
		ret = ret*4;
		//ret = ret/2;//became mono
        //printf("ret == %d\n",ret);
		for(j=0;j<ret;j++)
		{
			in_data[cur_point] = buffer[j];
			cur_point++;
			if(320 == cur_point)
			{
				cur_point=0;
				if(amr_encode_append_data(in_data,320,fpamr)<0)
				{   
				    printf("error when amr_encode_append_data\n");
					amr_encode_close(fpamr);
					exit(1);
				}
			}
		}
	}
 return 0;
}


static void usage(void) {
	unsigned int i = 0;

	printf("Usage: sndkit [OPTION]... [FILE]...\n\n");

	printf("-h --help\t\tprint this message\n");
	printf("-V --version\t\tprint current version\n");
	printf("-W --makewav\t\tconvert raw audio data to wav format\n");
	printf("-T --strip\t\tstrip the wav header(aka, convert wav to raw)\n");
	printf("-L --use-linein\t\tuse linein instead of MIC when record\n");
	printf("-D --device=NAME\tselect OSS snd device(such as: dsp dsp1 dsp2)\n");
	
	printf("-t --file-type=TYPE\tfile type (mp3, wav, raw)\n");
	printf("-c --channels=#\t\tchannels\n");
	printf("-f --format=FORMAT\tsample format (case insensitive)\n");
	printf("-r --rate=#\t\tsample rate(fs)\n");
	printf("-d --duration=#\t\trecord duration default is 10s, or analysis size\n");
	printf("-B --buffer-size=#\tbuffer size\n");
	printf("-l --linein bypass test\tlinein input music bypass to HP or lineout\n");
	
	printf("-P --replay\t\tenable playback\n");
	printf("-p --replay-file=FILE\tthe sound file to play\n");

	printf("-R --record\t\tenable record\n");
	printf("-w --record-file=FILE\tfile to save the recorded data\n");

	printf("-o --volume=0-100\tvolume\n");

	printf("\n");
	printf("Supported speeds are:");
	for (i = 0; i < ARRAY_SIZE(support_rates); i++) {
		printf("%d", support_rates[i]);

		if (i != (ARRAY_SIZE(support_rates) - 1))
			printf(", ");
	}

	printf("\n\n");

	printf("Recognized sample formats are:");
	printf(" U8, S16_LE, S24_3LE");
	printf("\n");
	printf("<example:dmicRecord -R -t wav -o 30 -w record.wav -c 1 -f S16_LE -r 16000 -D dsp3  -d 10>\n\n");
}

static int open_device(char *file, int oflag, int *is_reg_file) {
	struct stat stats;
	int fd = -1;

	*is_reg_file = 0;

	if ((lstat(file, &stats) == 0) &&
	    S_ISREG(stats.st_mode)) {
		sndkit_dbg("NOTE: device file is a regular file.\n");
		*is_reg_file = 1;
	}

	fd = open(file, oflag);
	if (fd < 0) {
		fprintf(stderr, "%s: cannot open file %s: %s\n",
			__func__,
			file,
			strerror(errno));

	}

	return fd;
}

static int open_file(char *file, int oflag) {
	int fd = -1;

	fd = open(file, oflag);
	if (fd < 0) {
		fprintf(stderr, "%s: cannot open file %s: %s\n",
			__func__,
			file,
			strerror(errno));
	}
	return fd;
}

static int config_device(int fd, int format, int channels, int speed)
{
	int ioctl_val;

	/* NOTE: config order is important: format, channel, speed */

	/* set bit format */
	ioctl_val = format;
	if (ioctl(fd, SNDCTL_DSP_SETFMT, &ioctl_val) == -1) {
		fprintf(stderr, "set format failed: %s\n",
			strerror(errno));
		return -1;
	}
	if (ioctl_val != format) {
		fprintf(stderr, "format not supported, changed by driver to 0x%08x\n",
			ioctl_val);
		return -1;
	}

	/*set channel */
	ioctl_val = channels;
	if ((ioctl(fd, SNDCTL_DSP_CHANNELS, &ioctl_val)) == -1) {
		fprintf(stderr, "set channels failed: %s\n",
			strerror(errno));
		return -1;
	}
	if (ioctl_val != channels) {
		fprintf(stderr, "%d channel is not supported, changed to %d by driver\n",
			channels, ioctl_val);
		return -1;
	}

	/*set speed */
	ioctl_val = speed;
	if (ioctl(fd, SNDCTL_DSP_SPEED, &ioctl_val) == -1) {
		fprintf(stderr, "set speed failed: %s\n",
			strerror(errno));
		return -1;
	}
	if (ioctl_val != speed) {
		fprintf(stderr, "speed %d is not supported, changed to %d by driver\n",
			speed, ioctl_val);
		return -1;
	}

	return 0;
}

static struct replay_params replay_params;
static int play(struct replay_params *params) {
	int dev_fd = params->dev_fd;
	int replay_fd = params->replay_fd;
	int buffer_size = params->buffer_size;
	int replay_data_len = params->replay_data_len;
	int format = params->format;
	int channels = params->channels;
	int speed = params->speed;
	int file_type = params->file_type;

	int ret = 0;
	int len = 0;
	int n = 0;
	int pos;
	unsigned char *buff;
	int alloc_size = buffer_size;
	int forever = 0;

	if (format == FMT24BITS) {
		if (buffer_size % 3) {
			alloc_size = buffer_size / 3 + 1;
			alloc_size = alloc_size * 3;
		}
		buff = malloc(alloc_size / 3 * 4);
	} else {
		buff = malloc(alloc_size);
	}

	if (buff == NULL) {
		fprintf(stderr, "PLAY: alloc buffer failed, size = %d\n", buffer_size);
		return -1;
	}

	if (replay_data_len == 0)
		forever = 1;


	while (forever || (replay_data_len > 0)) {
		len = read(replay_fd, buff, buffer_size);
		if (len < 0) {
			perror("PLAY: read sound data file failed");
			ret = -1;
			goto out;
		}

		if (len == 0) {
			sndkit_dbg("sound data play complete!\n");
			goto out;
		}

		if (!forever)
			replay_data_len -= len;

		if ( (file_type == FILE_TYPE_WAV) &&
		     (format == FMT24BITS)) {
			len = (len / 3) * 3;
			wav_expand_24bit_to_32bit(buff, len);
			len = len / 3 * 4;
		}

		pos = 0;
		while (len > 0) {
			if ((n = write(dev_fd, buff + pos, len)) < 0) {
				perror("write sound device file failed");
				ret = -1;
				goto out;
			}

			len -= n;
			pos += n;
		}
	}

	/* wait for all sound data replay out */
	if (ioctl(dev_fd, SNDCTL_DSP_SYNC, &len) < 0) {
		perror("dsp ioctl DSP_SYNC error\n");
		return -1;
	}
 out:
 	if(buff)
	    free(buff);
	sndkit_dbg("PLAY: end.\n");
	return ret;
}

static void *replay_thread(void *arg) {
	play((struct replay_params *)arg);

	return NULL;
}

void setRecordFlag(int value)
{
    g_record_flag = 1;
	//printf("set g_record_flag to %d\n",g_record_flag);
}

static SpeexPreprocessState *st = NULL;
static int g_noiselev = 0;
static int NoiseReduction(int samplerate,short *in,int len)
{
	   int i;
	   int count=0;
	   float f;
	   int sr = 0;
	   int num = 0;
	   int vad=0;
	
	   sr = samplerate;
	   num = sr/50;
	   num = num>len?num:len;
	   if (st == NULL )
		st	= speex_preprocess_state_init(num, sr);
	   i=1;
	   speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_DENOISE, &i);
	   i=1;
	   //speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_AGC, &i);
	   f=8000;
	   //speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_AGC_LEVEL, &f);
	   i=0;
	   speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_DEREVERB, &i);
	   f=.0;
	   speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_DEREVERB_DECAY, &f);
	   f=.0;
	   speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_DEREVERB_LEVEL, &f);
	   {
		  vad = speex_preprocess_run(st, in);
	   }
	   return 0;
	}

#if 0
{
   int i;
   int count=0;
   int sr = 0;
   int num = 0;

   sr = samplerate;
//   num = sr/50;
 //  num = num<len?num:len;
   if (st == NULL )
			st	= speex_preprocess_state_init(len, sr);
   i=1;
   if (g_noiselev < 0) {
	   speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_DENOISE, &i);
		 int noiseSuppress = g_noiselev;    
		 speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &noiseSuppress); //设置噪声的dB 
	 }
   int vad = 1;
   speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_VAD, &vad); //静音检测   
/*   float f;
	 f = 0.30f;
 	 speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_PROB_START, &f);
	 f = 0.07f;
	 speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_PROB_CONTINUE, &f); //Set probability required for the VAD to stay*/
   vad = speex_preprocess_run(st, in);
	// my_denoise(in,len);
	 /*
   i=1;
   //speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_AGC, &i);
   f=samplerate;
   //speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_AGC_LEVEL, &f);
   i=0;
   speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_DEREVERB, &i);
   f=.0;
   speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_DEREVERB_DECAY, &f);
   f=.0;
   speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_DEREVERB_LEVEL, &f);
   */
   return 0;
}
#endif

static int record(struct record_params *params) {
	int dev_fd = params->dev_fd;
	int record_fd = params->record_fd;
	int buffer_size = params->buffer_size;
	int duration = params->duration;
	int format = params->format;
	int channels = params->channels;
	int speed = params->speed;
	int file_type = params->file_type;
    struct MP3param mp3_param;
	int ret = 0;
	int n_read = 0;
	int n_to_write = 0;
	int n_to_write2 = 0;
	int n;
	int pos;
	unsigned char *buff = NULL;
	int format_bits = 16;
	int record_len = 0;
	RECORD_LIST *record_node = NULL;
	int ledvalue = 0;
	int datatotal = 0;

    if ( (g_fdLed= open("/proc/jz_gpio_wifi", 0)) < 0) {
        printf("thread open jz_gpio_wifi file error:%s\n", strerror(errno));
        g_fdLed = 0;
    }


	switch(format) {
	case FMT8BITS:
		format_bits = 8;
		break;
	case FMT16BITS:
		format_bits = 16;
		break;
	case FMT24BITS:
		format_bits = 24;
		break;
	default:
		break;
	}

	sndkit_dbg("REC: format_bits = %d, channels = %d, speed = %d, duration = %d\n", format_bits, channels, speed, duration);
	if(file_type == FILE_TYPE_AMR)
	{
				printf("file_type == FILE_TYPE_AMR buffer_size=%d\n",buffer_size);
#if HAVEENCODETHREAD
				if(RETERROR== createRecordList(MAXRECORDNODE))
				{
					PRINTFERROR("createRecordList failed!\n");
					goto out;
				}
				mp3_param.fd = record_fd;
				mp3_param.channel = channels;
				if(RETSUCCESS != createEncodeThread(mp3_param))
				{
					goto out;
				}
#else
				buff = malloc(buffer_size);
				if (buff == NULL)
				{
					fprintf(stderr, "REC: alloc buffer failed, size = %d\n", buffer_size);
					return -1;
				}
#endif
				while (1) 
				{
					if(g_record_exit)
					{
						PRINTFWARNING("g_record_exit == 1\n");
						break;
					}
					
#if HAVEENCODETHREAD
					record_node = getRecordNode();
					if(NULL == record_node)
					{
						usleep(5000);
						continue;
					}			
					n_read = read(dev_fd, record_node->buff,buffer_size);//DEFAULT_BUFF_SIZE
#else
					n_read = read(dev_fd, buff,buffer_size);//DEFAULT_BUFF_SIZE
#endif
					if (n_read < 0) {
						perror("11 read sound device(file) failed");
						ret = -1;
						goto out;
					}
		
					if (n_read == 0) {
						printf("REC: zero bytes! why?\n");
						memset(record_node->buff,0x0,DEFAULT_BUFF_SIZE);
						continue;
					}
					n_to_write = n_read/(channels);;//char to short & 2 channels
					datatotal+=n_read;
					if(datatotal > (32000))//16*160001/8=1S pcm data
					{
					    if(g_fdLed)
                        {
                            ioctl(g_fdLed, (0x10+ledvalue));
						}
						datatotal = 0;
                        ledvalue = !ledvalue;
					}
#if HAVEENCODETHREAD
					record_node->datalen = n_to_write;
		//do nothing
#else
                    if(g_fpamr)
					    mypcm2amr(g_fpamr,buff,n_to_write);
					setRecordFlag(1);
#endif
				}

	}
    else if(file_type == FILE_TYPE_MP3)//encode to mp3
    {
        printf("file_type == FILE_TYPE_MP3 buffer_size=%d\n",buffer_size);
#if HAVEENCODETHREAD
        if(RETERROR== createRecordList(MAXRECORDNODE))
        {
	        PRINTFERROR("createRecordList failed!\n");
	        goto out;
        }
        mp3_param.fd = record_fd;
		mp3_param.channel = channels;
        if(RETSUCCESS != createEncodeThread(mp3_param))
        {
	        goto out;
        }
#else
        buff = malloc(buffer_size);
        if (buff == NULL)
	    {
	        fprintf(stderr, "REC: alloc buffer failed, size = %d\n", buffer_size);
	        return -1;
        }
#endif
		while (1) 
		{
			if(g_record_exit)
			{
                PRINTFWARNING("g_record_exit == 1\n");
				break;
			}
			
#if HAVEENCODETHREAD
			record_node = getRecordNode();
			if(NULL == record_node)
			{
                usleep(5000);
				continue;
			}			
	    	n_read = read(dev_fd, record_node->buff,buffer_size);//DEFAULT_BUFF_SIZE
#else
            n_read = read(dev_fd, buff,buffer_size);//DEFAULT_BUFF_SIZE
#endif
		    if (n_read < 0) {
				printf("dev_fd=%d record_fd=%d\n",dev_fd,record_fd);
		    	perror("22 read sound device(file) failed");
		    	ret = -1;
		    	goto out;
		    }

		    if (n_read == 0) {
			    printf("REC: zero bytes! why?\n");
				memset(record_node->buff,0x0,DEFAULT_BUFF_SIZE);
		    	continue;
		    }
			//if(n_read != DEFAULT_BUFF_SIZE)
			//	continue;
		    n_to_write = n_read/(2*channels);//char to short & 2 channels
			datatotal+=n_read;
#if ENABLENOISEREDUCE
			NoiseReduction(speed,(short *)buff,n_to_write);
#endif
			if(datatotal > (32000))//16*16000*1/8=1S pcm data
			{
			    if(g_fdLed)
			    {
				    ioctl(g_fdLed, (0x10+ledvalue));
			    }
				datatotal = 0;
				ledvalue = !ledvalue;
			}

#if HAVEENCODETHREAD
		    record_node->datalen = n_to_write;
//do nothing
#else
            pos = 0;
		    //while (n_to_write > 0) 
			{
			    if ((n = encode2Mp3(record_fd,(short *)(buff),n_to_write)) < 0)
				{
				    perror("REC: write sound data file failed");
				    ret = -1;
				    goto out;
			    }
			  //  n_to_write -= n;
		    //	pos += n*(2*channels);
		    }
			setRecordFlag(1);
#endif
	    }
		close(record_fd);
		record_fd = 0;
	}
	else if(file_type == FILE_TYPE_WAV_MY)//encode to wav
	{
		printf("file_type == FILE_TYPE_WAV_MY buffer_size=%d\n",buffer_size);
		buff = malloc(buffer_size);
		if (buff == NULL)
		{
			fprintf(stderr, "REC: alloc buffer failed, size = %d\n", buffer_size);
			return -1;
		}
		while (1) 
		{
			if(g_record_exit)
			{
				PRINTFWARNING("g_record_exit == 1\n");
				break;
			}
			
		    n_read = read(dev_fd, buff,buffer_size);
		    if (n_read < 0) {
		    	perror("313 read sound device(file) failed");
		    	ret = -1;
		    	goto out;
		    }

		    if (n_read == 0) {
			    printf("REC: zero bytes! why?\n");
		    	continue;
		    }

		    if(format_bits == 24)
			{
			    wav_32bit_to_24bit(buff, n_read);
		    	n_read = n_read / 4 * 3;
		    }

			if (n_read < 0) {
				printf("dev_fd=%d record_fd=%d\n",dev_fd,record_fd);
				perror("22 read sound device(file) failed");
				ret = -1;
				goto out;
			}
	
			if (n_read == 0) {
				printf("REC: zero bytes! why?\n");
				memset(record_node->buff,0x0,DEFAULT_BUFF_SIZE);
				continue;
			}
			//if(n_read != DEFAULT_BUFF_SIZE)
			//	continue;
			n_to_write = n_read/(2*channels);//char to short & 2 channels
			datatotal+=n_read;
#if ENABLENOISEREDUCE
				NoiseReduction(speed,(short *)buff,n_to_write);
#endif
			if(datatotal > (64000))//16*16000*2/8=1S pcm data
			{
				if(g_fdLed)
				{
					ioctl(g_fdLed, (0x10+ledvalue));
				}
				datatotal = 0;
				ledvalue = !ledvalue;
			}

			pos = 0;
			while (n_read > 0) {
				if ((n = write(record_fd, buff + pos, n_read)) < 0) {
					perror("REC: write sound data file failed");
					ret = -1;
					goto out;
				}
				n_read -= n;
				pos += n;
			}
			setRecordFlag(1);
		}
		close(record_fd);
		record_fd = 0;
	}
    else
    {   
		buff = malloc(buffer_size);
		
		if (buff == NULL) {
			fprintf(stderr, "REC: alloc buffer failed, size = %d\n", buffer_size);
			return -1;
		}

		record_len = format_bits * channels * speed / 8 * duration;
		sndkit_dbg("record len = %d\n", record_len);

	    while (record_len > 0) {
		    /* There are 32bit one sample when read dsp in 24bit width record */	
		    if (format_bits == 24){
			    n_read = read(dev_fd, buff,
					(buffer_size > (record_len/3*4))
					? (record_len/3*4)
					: buffer_size);
		    }else{
		    	n_read = read(dev_fd, buff,
					(buffer_size > record_len)
					? record_len
					: buffer_size);
		    }
		    if (n_read < 0) {
		    	perror("33 read sound device(file) failed");
		    	ret = -1;
		    	goto out;
		    }

		    if (n_read == 0) {
			    printf("REC: zero bytes! why?\n");
		    	continue;
		    }

		    if ((file_type == FILE_TYPE_WAV) &&
		        (format_bits == 24)) {
			    wav_32bit_to_24bit(buff, n_read);
		    	n_read = n_read / 4 * 3;
		    }

		    if (n_read < record_len)
		    	n_to_write = n_read;
		    else
			    n_to_write = record_len;

		    n_to_write2 = n_to_write;
		    //printf("n_to_write=%d\n",n_to_write);
		    pos = 0;
		    while (n_to_write > 0) {
			    if ((n = write(record_fd, buff + pos, n_to_write)) < 0) {
				    perror("REC: write sound data file failed");
				    ret = -1;
				    goto out;
			    }
			    n_to_write -= n;
		    	pos += n;
		    }
		
	    	record_len -= n_to_write2;
	    }
		setRecordFlag(1);
    }
 out:
	 if(buff)
	    free(buff);
	 if(g_fpamr)
	 	fclose(g_fpamr);
	sndkit_dbg("REC: end.\n");
	return ret;
}

static void *record_thread(void *arg) {
	record((struct record_params *)arg);
	return NULL;
}

#define FILE_BLOCK_LEN	4096

static int makewav(char *wav_file, char *raw_file,
	    int channels, int speed, int format) {
	int wav_fd = -1;
	int file_len = 0;
	int raw_fd = -1;
	struct stat stats;
	int head_len = 0;
	int len = 0;
	unsigned char wav_head[WAV_HEAD_SIZE];
	unsigned char *wav_head2 = wav_head;
	unsigned char *buff = NULL;
	int ret = 0;
	int format_bits;

	if ((wav_file == NULL) || (wav_file[0] == '\0')) {
		fprintf(stderr, "wav file is not specified!\n");
		return -1;
	}

	if ((raw_file == NULL) || (raw_file[0] == '\0')) {
		fprintf(stderr, "raw file is not specified!\n");
		return -1;
	}

	wav_fd = open_file(wav_file, O_WRONLY | O_CREAT);
	if (wav_fd < 0) {
		fprintf(stderr, "open wav file %s failed.\n", wav_file);
		return -1;
	}

	if ((lstat(raw_file, &stats) == 0) &&
	    S_ISREG(stats.st_mode)) {
		file_len = (int)stats.st_size;
		sndkit_dbg("CONVERT: file len is %d\n", file_len);
	} else {
		fprintf(stderr, "CONVERT: can not stat %s\n", raw_file);
		return -1;
	}

	raw_fd = open_file(raw_file, O_RDONLY);
	if (raw_fd < 0) {
		fprintf(stderr, "open raw file %s failed.\n", raw_file);
		return -1;
	}

	switch (format) {
	case FMT8BITS:
		format_bits = 8;
		break;
	case FMT16BITS:
		format_bits = 16;
		break;
	case FMT24BITS:
		format_bits = 24;
		break;
	default:
		format_bits = 16;
	}

	init_wav_header(wav_head, file_len, channels, speed, format_bits);
	head_len = WAV_HEAD_SIZE;
	while (head_len > 0) {
		if ((len = write(wav_fd, wav_head2, head_len)) < 0) {
			perror("write sound data file failed");
			return -1;
		}
		head_len -= len;
		wav_head2 += len;
	}

	buff = malloc(FILE_BLOCK_LEN);
	if (!buff) {
		fprintf(stderr, "CONVERT: alloc buffer failed.\n");
		return -1;
	}

	while (file_len > 0) {
		int wn;
		int pos;

		len = read(raw_fd, buff, FILE_BLOCK_LEN);
		if (len < 0) {
			perror("CONVERT: read raw file failed.\n");
			ret = len;
			goto out;
		}

		if (len == 0)
			goto out;

		file_len -= len;

		wn = len;
		pos = 0;

		while (wn > 0) {
			if ((len = write(wav_fd, buff + pos, wn)) < 0) {
				perror("CONVERT: write wav file failed.\n");
				ret = len;
				goto out;
			}

			wn -= len;
			pos += len;
		}
	}

 out:
 	if(buff)
	    free(buff);
	return ret;
}

static int strip_wav(char *raw_file, char *wav_file) {
	int wav_fd;
	int raw_fd;
	int ret = 0;
	unsigned char *buff = NULL;
	int wav_data_len = 0;
	int len = 0;
	unsigned int snd_chn = 0;
	unsigned int snd_spd = 0;
	unsigned int snd_fmt = 0;
	int head_len;

	if ((wav_file == NULL) || (wav_file[0] == '\0')) {
		fprintf(stderr, "wav file is not specified!\n");
		return -1;
	}

	if ((raw_file == NULL) || (raw_file[0] == '\0')) {
		fprintf(stderr, "raw file is not specified!\n");
		return -1;
	}

	wav_fd = open_file(wav_file, O_RDONLY);
	if (wav_fd < 0) {
		fprintf(stderr, "open wav file %s failed.\n", wav_file);
		return -1;
	}

	raw_fd = open_file(raw_file, O_WRONLY | O_CREAT);
	if (raw_fd < 0) {
		fprintf(stderr, "open raw file %s failed.\n", raw_file);
		return -1;
	}

	buff = malloc(FILE_BLOCK_LEN);
	if (!buff) {
		fprintf(stderr, "STRIP: alloc buffer failed.\n");
		return -1;
	}

	/* the file must be at least MAX_WAV_HEAD_SIZE bytes */
	len = read(wav_fd, buff, MAX_WAV_HEAD_SIZE);
	if (len != MAX_WAV_HEAD_SIZE) {
		fprintf(stderr, "WAV file must larger than %d bytes.\n", MAX_WAV_HEAD_SIZE);
		return -1;
	} else {
		wav_data_len = parse_wav_header(buff, &head_len,
					       &snd_chn, &snd_spd, &snd_fmt);

		if (wav_data_len == 0) {
			fprintf(stderr, "the specified file is not a WAV file.\n");
			return -1;
		}

		lseek(wav_fd, head_len, SEEK_SET);
	}

	while (wav_data_len > 0) {
		int wn;
		int pos;

		len = read(wav_fd, buff, FILE_BLOCK_LEN);
		if (len < 0) {
			perror("CONVERT: read raw file failed.\n");
			goto out;
		}

		if (len == 0)
			goto out;

		wav_data_len -= len;

		wn = len;
		pos = 0;

		while (wn > 0) {
			if ((len = write(raw_fd, buff + pos, wn)) < 0) {
				perror("CONVERT: write wav file failed.\n");
				ret = len;
				goto out;
			}

			wn -= len;
			pos += len;
		}
	}
 out:
 	if(buff)
	    free(buff);
	return ret;
}

static void mySignalHandle(int sig)
{
	char cmdstr[256] = {0};

	if(g_fdLed)
	{
	    ioctl(g_fdLed, LED_ON);
		close(g_fdLed);
	}

	if(0 == g_record_exit)
	{
	    g_record_exit = 1;
	    sleep(1);//wait record
	}
	else
	{
        printf("already come here,so exit direct!\n");
		exit(1);
	}
	printf("\n\n[%s: %d] record application crashed by signal %s(%d).\n", __FUNCTION__, __LINE__, signal_str[sig],sig);

	//if(g_record_params.file_type == FILE_TYPE_MP3)
     //   deInitLameEncode();

	if(!g_record_flag)
	{
        sprintf(cmdstr,"rm %s",g_record_file);
		system(cmdstr);
		printf(">>>>>cmdstr=%s\n",cmdstr);
		memset(cmdstr,0x0,sizeof(cmdstr));
	}

	if (sig == SIGSEGV || sig == SIGBUS ||
	    sig == SIGTRAP || sig == SIGABRT) {
		sprintf(cmdstr, "cat /proc/%d/maps", getpid());
		printf("Process maps:\n");
		system(cmdstr);
	}
	
}

int main(int argc, char *argv[])
{
	char snd_dev[128]={0};
	char mixer_dev[128]={0};
	int dev_fd = -1;
	
    char cmdstr[256]={0};
	int channels = DEFAULT_SND_CHN;
	int channels_user = 0;
	int format = FMT16BITS;
	int format_bits = 16;
	int format_user = 0;
	int speed = DEFAULT_SND_SPD;
	int speed_user = 0;

	unsigned int duration = DEFAULT_DURATION;
	unsigned int buffer_size = DEFAULT_BUFF_SIZE;

	int replay_enable = 0;
	char replay_file[128]={0};
	int replay_fd = -1;
	int replay_data_len = 0;
    int record_fd = 0;
	int record_enable = 0;
	
	pthread_t replay_pid;
	int replaying = 0xdead;
	pthread_t record_pid;
	int recording = 0xdead;
	int use_linein = 0;
	int file_type = FILE_TYPE_WAV;

	int vol = 0;
	int ret = 0;
	int is_dev_reg_file = 0;
	int convert_raw_to_wav = 0;
	int is_strip_wav = 0;
	char c;
	int linein_bypass = 0;

//	strcpy(snd_dev, DEFAULT_SND_DEV);
	strcpy(mixer_dev, DEFAULT_MIXER_DEV);
	
    signal_setup(mySignalHandle);


/**************default init*********************/
//./record -R -t mp3 -o 30 -w record.wav -c 1 -f S16_LE -r 16000 -D dsp3  -d 10
    record_enable = 1;
    file_type = FILE_TYPE_MP3;
	vol = 80;
    if(getRecordFilename(g_record_file))
    {
        printf("ERROR! getRecordFilename failed!\n");
		return 1;
	}
    else
	{   
	    if(file_type == FILE_TYPE_AMR)
			strcat(g_record_file,".amr");
		else if(file_type == FILE_TYPE_WAV_MY)
			strcat(g_record_file,".wav");
		else
			strcat(g_record_file,".mp3");
	    printf("record_file path=%s\n",g_record_file);
	}
	channels = ENCODECHANNEL;
	channels_user = 1;
	format = fmtstr_to_fmt("S16_LE");
	speed = 16000;
	speed_user = 1;
	strcpy(snd_dev, DEFAULT_SND_DEV3);
	strcpy(mixer_dev, DEFAULT_MIXER_DEV3);
	duration = FOREVER_DURATION;
/*******************************************/

	while (1) {
		c = getopt_long(argc, argv, optstring, long_options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			usage();
			return 0;

		case 'V':
			printf(SNDKIT_VERSION);
			return 0;

		case 'W':
			convert_raw_to_wav = 1;
			break;

		case 'T':
			is_strip_wav = 1;
			break;
        case 'L':
            /*use_linein = 1;*/
            break;

		case 'D':
#if 1
			if (!strcmp("dsp", optarg)){
				strcpy(snd_dev, DEFAULT_SND_DEV);
				strcpy(mixer_dev, DEFAULT_MIXER_DEV);
			}else if (!strcmp("dsp1", optarg)){
				strcpy(snd_dev, DEFAULT_SND_DEV1);
				strcpy(mixer_dev, DEFAULT_MIXER_DEV1);
			}else if (!strcmp("dsp2", optarg)){
				strcpy(snd_dev, DEFAULT_SND_DEV2);
				strcpy(mixer_dev, DEFAULT_MIXER_DEV2);
			}else if (!strcmp("dsp3", optarg)){
				strcpy(snd_dev, DEFAULT_SND_DEV3);
				strcpy(mixer_dev, DEFAULT_MIXER_DEV3);
			}else {
				printf("unsupport sound device: %s\n", optarg);
				return 127;
			}
			break;
#else
			if (strlen(optarg) > 127) {
				printf("file name is too long: %s\n", optarg);
				return 127;
			}
			strncpy(snd_dev, optarg, strlen(optarg));
			break;
#endif
		case 't':
			if (!strcmp("wav", optarg))
				file_type = FILE_TYPE_WAV;
			else if (!strcmp("raw", optarg))
				file_type = FILE_TYPE_RAW;
			else if(!strcmp("mp3", optarg)){
				file_type = FILE_TYPE_MP3;
			}
			else
			{
			    printf("unrecognized file type: %s\n", optarg);
				return 127;
			}
			break;

		case 'c':
			channels = atoi(optarg);
			channels_user = 1;
			break;

		case 'f':
			format = fmtstr_to_fmt(optarg);
			if (format < 0) {
				printf("unrecognized format: %s\n", optarg);
				return 127;
			}
			format_user = 1;
			break;

		case 'r':
			speed = atoi(optarg);
			if (!rate_is_supported(speed)) {
				printf("unrecognized sample rate: %d\n", speed);
				return 127;
			}
			speed_user = 1;
			break;

		case 'd':
			duration = atoi(optarg);
			if (duration == 0) {
				printf("duration is 0, what do you want to do?\n");
				return 127;
			}
			break;

		case 'B':
			buffer_size = atoi(optarg);
			break;
	
		case 'l':
			strcpy(snd_dev, LINEIN_DEV);
			linein_bypass = 1;
			break;

		case 'P':
			/*replay_enable = 1;*/
			break;

		case 'p':
			if (strlen(optarg) > 127) {
				printf("file name is too long: %s\n", optarg);
				return 127;
			}
			strncpy(replay_file, optarg, strlen(optarg));
			break;


		case 'R':
			record_enable = 1;
			break;

		case 'w':
			if (strlen(optarg) > 127) {
				printf("file name is too long: %s\n", optarg);
				return 127;
			}
			memset(g_record_file, 0, sizeof(g_record_file));
			strncpy(g_record_file, optarg, strlen(optarg));
			break;

		case 'o':
			vol = atoi(optarg);
			if ((vol < 0) || (vol > 100)) {
				printf("volume must be in 0~100.\n");
				return 127;
			}
			break;

		default:
			/* do nothing */
			printf("Unknown option '%c'\n", c);
			usage();
			return 127;
		}
	}

	if (convert_raw_to_wav)
		return makewav(g_record_file, replay_file, channels, speed, format);

	if (is_strip_wav)
		return strip_wav(g_record_file, replay_file);

	if (snd_dev[0] == '\0') {
		fprintf(stderr, "device is not specified!\n");
		return 127;
	}

	if (replay_enable && (replay_file[0] == '\0')) {
		sndkit_dbg("replay file is not specified! stdin will be used!\n");
		//return 127;
	}

	if (record_enable  && (g_record_file[0] == '\0')) {
		fprintf(stderr, "record file is not specified!\n");
		//return 127;
	}

	/* adjust replay or record volume & route */
	{
		int mixer_fd = -1;
		unsigned int mode = 0;
		int ret = -1;
		if ((mixer_fd = open(mixer_dev, O_RDWR)) < 0) {
			perror("open mixer failed.\n");
			exit(1);
		}
		/* adjust replay volume */
		if(replay_enable){
#if SET_CODEC_ROUTE
			mode = SOUND_MIXER_SPEAKER;
                    	ret = ioctl(mixer_fd, MIXER_WRITE(SOUND_MIXER_OUTSRC), &mode);
                        if (ret < 0) {
                                printf("write outsrc failed\n");
                                return ret;
                        }
#endif
			if (ioctl(mixer_fd, SOUND_MIXER_WRITE_VOLUME, &vol) == -1) {
				perror("set replay volume failed!\n");
				return -1;
			}
		}

		if(record_enable){
#if SET_CODEC_ROUTE
			/* adjust record route */
			if (use_linein) {
				mode = SOUND_MIXER_LINE; //This route is for linein replay
			} else {
				mode = SOUND_MIXER_MIC;  //This route is for analog mic record. There is not analog mic in X1000 canna board. 
			}
			if (ioctl(mixer_fd, SOUND_MIXER_WRITE_RECSRC, &mode) == -1) {
				perror("set record route failed!");
				return -1;
			}
#endif
 			/* adjust record volume */
			if (ioctl(mixer_fd, SOUND_MIXER_WRITE_MIC, &vol) == -1) {
				perror("set record volume failed!\n");
				return -1;
			}
		}
		close(mixer_fd);
	}

#if 0
	/* This is only for m150 linein replay function */
	if (linein_bypass){
		int linein_fd = -1;
		printf("open /dev/linein now\n");
		if ((linein_fd = open(snd_dev, O_RDWR)) < 0) {
                        perror("open /dev/linein failed.\n");
                        exit(1);
                }
		while (1)
			sleep(1);
 		return 0;
	}
#endif

	/* open device */
	if (replay_enable && record_enable) {
		sndkit_dbg("Play & Record\n");
		dev_fd = open_device(snd_dev, O_RDWR, &is_dev_reg_file);
	} else if (replay_enable) {
		sndkit_dbg("Play\n");
		dev_fd = open_device(snd_dev, O_WRONLY, &is_dev_reg_file);
	} else if (record_enable) {
		sndkit_dbg("Record\n");
		dev_fd = open_device(snd_dev, O_RDONLY, &is_dev_reg_file);
	}

	if ( (record_enable || replay_enable) && (dev_fd < 0)) {
		fprintf(stderr, "open device failed.\n");
		return -1;
	}

	/* open replay sound file */
	if (replay_enable &&  (replay_file[0] != '\0')) {
		replay_fd = open_file(replay_file, O_RDONLY);
		if (replay_fd < 0)
			return -1;
	}

	if (replay_enable &&  (replay_file[0] == '\0')) {
		replay_fd = fileno(stdin);
	}

	/* if replay & wav, check if the file is wav file */
	if (replay_enable && (file_type == FILE_TYPE_WAV)) {
		int head_len = 0;
		int len = 0;
		int is_wav_file = 0;
		unsigned int snd_chn = 0;
		unsigned int snd_spd = 0;
		unsigned int snd_fmt = 0;
		unsigned char buff[MAX_WAV_HEAD_SIZE];

		/* the file must be at least MAX_WAV_HEAD_SIZE bytes */
		len = read(replay_fd, buff, MAX_WAV_HEAD_SIZE);
		if (len != MAX_WAV_HEAD_SIZE) {
			fprintf(stderr, "WAV file must larger than %d bytes.\n", MAX_WAV_HEAD_SIZE);
			return -1;
		} else {
			is_wav_file = parse_wav_header(buff, &head_len,
						       &snd_chn, &snd_spd, &snd_fmt);

			if (is_wav_file == 0) {
				fprintf(stderr, "the specified file is not a WAV file.\n");
				return -1;
			}

			if (format_user == 0) {
				switch (snd_fmt) {
				case 8:
					format = FMT8BITS;
					break;
				case 16:
					format = FMT16BITS;
					break;
				case 24:
					format = FMT24BITS;
					break;
				default:
					break;
				}
			}

			if (channels_user == 0)
				channels = snd_chn;

			if (speed_user == 0)
				speed = snd_spd;

			lseek(replay_fd, head_len, SEEK_SET);

			replay_data_len = is_wav_file;
		}
	}

	sndkit_dbg("%s: format = ",
	       (file_type == FILE_TYPE_WAV) ? "WAV" : "RAW"
	       );
	switch (format) {
	case FMT8BITS:
		format_bits = 8;
		break;
	case FMT16BITS:
		format_bits = 16;
		break;
	case FMT24BITS:
		format_bits = 24;
		break;
	default:
		break;
	}
	sndkit_dbg("%d", format_bits);

	sndkit_dbg(" channels = %d, speed = %d g_record_file=%s\n",
	       channels, speed,g_record_file);

	/* open record file */
	if(file_type == FILE_TYPE_AMR)
		g_fpamr = amr_encode_open(g_record_file, 1);
	else if (record_enable) {
		record_fd = open_file(g_record_file, O_WRONLY | O_CREAT);
		if (record_fd < 0)
			return -1;
	}
	if(file_type == FILE_TYPE_MP3)
        initLameEncode(speed,channels);
	/* prepare wav header */
	if (record_enable && (file_type == FILE_TYPE_WAV || file_type == FILE_TYPE_WAV_MY)) {
		int hi;
		int hn;
		unsigned char wav_head[WAV_HEAD_SIZE];
		unsigned char *wav_head2 = wav_head;

		int record_len = format_bits * channels * speed / 8 * duration;

		init_wav_header(wav_head, record_len, channels, speed, format_bits);
		hi = WAV_HEAD_SIZE;
		while(hi > 0) {
			if ((hn = write(record_fd, wav_head2, hi)) < 0) {
				perror("write sound data file failed");
				return -1;
			}
			hi -= hn;
			wav_head2 += hn;
		}
	}

	/*set sound format */
	if ( (record_enable || replay_enable) && (!is_dev_reg_file)) {
		if (config_device(dev_fd, format, channels, speed) < 0)
			return -1;
	}

	/* Now everything is ok, do real stuffs */
	if (replay_enable) {
		replay_params.dev_fd = dev_fd;
		replay_params.replay_fd = replay_fd;
		replay_params.buffer_size = buffer_size;
		replay_params.replay_data_len = replay_data_len;
		replay_params.format = format;
		replay_params.channels = channels;
		replay_params.speed = speed;
		replay_params.file_type = file_type;

		replaying = pthread_create(&replay_pid, NULL, replay_thread, &replay_params);
		if (replaying != 0) {
			perror("PLAY: start replay thread failed.");
		}
	}

	if (record_enable) {
		if (replay_enable)
			sleep(1);

		g_record_params.dev_fd = dev_fd;
		g_record_params.record_fd = record_fd;
		g_record_params.buffer_size = buffer_size;
		g_record_params.duration = duration;
		g_record_params.format = format;
		g_record_params.channels = channels;
		g_record_params.speed = speed;
		g_record_params.file_type = file_type;
        if(file_type == FILE_TYPE_MP3)
        {
            record(&g_record_params);
			recording = 1;
		}
		else
		{
		    recording = pthread_create(&record_pid, NULL, record_thread, &g_record_params);
		    if (recording != 0) {
			    perror("REC: start record thread failed.");
		    }
		}
	}

	if ((file_type != FILE_TYPE_MP3)&&(recording == 0)) {
		pthread_join(record_pid, NULL);
	}

	if (replaying == 0) {
		pthread_join(replay_pid, NULL);
	}

	close(dev_fd);
	if(g_fdLed)
	{
	    ioctl(g_fdLed, LED_ON);
		close(g_fdLed);
	}

	if(file_type == FILE_TYPE_MP3)
        deInitLameEncode(record_fd);

    close(record_fd);
	
	if(!g_record_flag)
	{
        sprintf(cmdstr,"rm %s",g_record_file);
		system(cmdstr);
		memset(cmdstr,0x0,sizeof(cmdstr));
	}

	return ret;
}




