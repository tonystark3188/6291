#include <stdio.h>
#include <lame/lame.h>
 
int main(void)
{
    int read, write;
 
    FILE *pcm = fopen("record-mono.raw", "rb");
    FILE *mp3 = fopen("record.mp3", "wb");
 
    const int PCM_SIZE = 8192;
    const int MP3_SIZE = 8192;
    int channel = 1;
    short int pcm_buffer[PCM_SIZE*2];
    unsigned char mp3_buffer[MP3_SIZE];
 
    lame_t lame = lame_init();
    lame_set_in_samplerate(lame, 16000);
    lame_set_VBR(lame, vbr_default);
    lame_set_num_channels(lame,channel);  //Ë«ÉùµÀ£¬Õâ¸öOK
  //  lame_set_brate(lame,16);   //16kBps
	lame_set_quality(lame,5);
    lame_init_params(lame);
	
	/*
    int     VBR_mean_bitrate_kbps;
    int     VBR_min_bitrate_kbps;
    int     VBR_max_bitrate_kbps;
	*/
 //printf("-----%d  %d   %d -----\n",lame->samplerate_in,lame->samplerate_out,lame->samplerate_in);
    do {
        read = fread(pcm_buffer, channel*sizeof(short int), PCM_SIZE, pcm);
        if (read == 0)
            write = lame_encode_flush(lame, mp3_buffer, MP3_SIZE);
        else
        {   
            if(2 == channel)
				write = lame_encode_buffer_interleaved(lame, pcm_buffer, read, mp3_buffer, MP3_SIZE);
            else 
				write = lame_encode_buffer(lame, pcm_buffer, pcm_buffer, read, mp3_buffer, MP3_SIZE);
		}
        fwrite(mp3_buffer, write, 1, mp3);
    } while (read != 0);
 
    lame_close(lame);
    fclose(mp3);
    fclose(pcm);
 
    return 0;
}