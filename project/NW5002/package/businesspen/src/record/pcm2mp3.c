#include "headers.h"
#include <lame/lame.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "common.h"
#include "record_list.h"
#include "pcm2mp3.h"


#define MP3_SIZE (DEFAULT_BUFF_SIZE/2)
#define PRINTTIME 0


static lame_t g_lame;
static pthread_t g_encode_thread;
static int g_exit_encode = 0;

int initLameEncode(int speed,int channels)
{
    g_lame = lame_init();
    lame_set_in_samplerate(g_lame, speed);
    lame_set_VBR(g_lame, vbr_default);
	lame_set_quality(g_lame,7);
	lame_set_num_channels(g_lame,channels);
	lame_set_brate(g_lame,32);
    lame_init_params(g_lame);

    id3tag_init(g_lame);
    id3tag_add_v2(g_lame);
    id3tag_space_v1(g_lame);
    id3tag_pad_v2(g_lame);
    id3tag_set_artist(g_lame,"DAMAI");
    id3tag_set_album(g_lame,"nw5002");
    id3tag_set_title(g_lame,"DM-5002");
    id3tag_set_track(g_lame,"0");
    id3tag_set_year(g_lame,"unknow");
    id3tag_set_comment(g_lame,"Senzhen Damai Technology Co.,Ltd");
    id3tag_set_genre(g_lame,"Other");

	
	return RETSUCCESS;
}

#if HAVEENCODETHREAD
void *encode2Mp3(void *arg)
{
	struct MP3param *mp3param = (struct MP3param *)arg;
	int mp3fd = 0;
	int mp3_size = 0;
	unsigned char mp3_buffer[MP3_SIZE] = {0};
	int writen = 0;
	int ret = 0;
	RECORD_LIST *record_node = NULL;

    mp3fd = mp3param->fd;
    mp3_size = MP3_SIZE/(mp3param->channel);
		
    printf("mp3fd=%d   channel=%d\n",mp3fd,mp3param->channel);
	while(1)
	{
        if(g_exit_encode)
			break;
		record_node = getEncordNode();
		if(record_node)
		{
			if (0 == (record_node->datalen))
				writen = lame_encode_flush(g_lame, mp3_buffer, mp3_size);
			else
			{
			    if(2 == mp3param->channel)
			        writen = lame_encode_buffer_interleaved(g_lame, (short *)(record_node->buff), record_node->datalen, mp3_buffer, mp3_size);
			    else
					writen = lame_encode_buffer(g_lame, (short *)(record_node->buff), (short *)(record_node->buff), record_node->datalen, mp3_buffer, mp3_size);
			}
			record_node->datalen = 0;
			ret = write(mp3fd,mp3_buffer, writen);// printf("writen=%d\n",writen);
			if(ret < 0)
			{
			    perror(">>>>>>>>>>>>REC: write sound data file failed %d\n");
				exit(1);
			}
			else
				setRecordFlag(1);
		}
		else
			usleep(5000);		
	}
	pthread_join(g_encode_thread, NULL);
	
}
#else
int encode2Mp3(int mp3fd, short *pcm_buffer,int len)
{
	unsigned char mp3_buffer[MP3_SIZE] = {0};
	int writen = 0;
#if PRINTTIME
	struct timeval  tstart, tend;
	long diffsec=0, diffusec=0; 
#endif

#if PRINTTIME
    gettimeofday( &tstart, 0 );
#endif
  //  if (len > 1152)
    //    len = 1152;

	if (len == 0)
		writen = lame_encode_flush(g_lame, mp3_buffer, MP3_SIZE);
	else
	{
	    if(2 == ENCODECHANNEL)
	        writen = lame_encode_buffer_interleaved(g_lame, pcm_buffer, len, mp3_buffer, MP3_SIZE);
		else
			writen = lame_encode_buffer(g_lame, pcm_buffer, pcm_buffer, len, mp3_buffer, MP3_SIZE);
	}
    writen = write(mp3fd,mp3_buffer, writen);
#if PRINTTIME
	gettimeofday( &tend, 0	);
	diffusec = tend.tv_usec - tstart.tv_usec ;
	diffsec =  tend.tv_sec - tstart.tv_sec ;
	if ( diffusec <  0 )
	{
		diffsec -=1; 
		diffusec +=1000000;
	}
	printf( "%dS %06lduS  len=%d\n",diffsec,diffusec,len);
#endif
	return writen;
}
#endif

int createEncodeThread(struct MP3param mp3_param)
{
    int ret = 0;
	
	ret = pthread_create(&g_encode_thread, NULL, encode2Mp3, &mp3_param);
	if (ret != 0) {
		perror("create Encode Thread failed.\n");
		return RETERROR;
	}
    return RETSUCCESS;
}

int deInitLameEncode(int mp3fd)
{
    char Buf[7200]={0};
    int mp3bytes = lame_encode_flush(g_lame,Buf,7200);

	g_exit_encode = 1;
	usleep(500000);
	if (mp3bytes>0)
	{
	  write(mp3fd,Buf, mp3bytes);
	}
    lame_close(g_lame);
#if HAVEENCODETHREAD
	freeRecordList();
#endif
    printf("deInitLame over!\n");
}


