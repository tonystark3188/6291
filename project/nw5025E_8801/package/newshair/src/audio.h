#ifndef _AUDIO_H
#define _AUDIO_H

typedef struct {
    void (*help)(void);
    char *name;

    // start of program
    int (*init)(int argc, char **argv);
    // at end of program
    void (*deinit)(void);

    void (*start)(int sample_rate);
    // block of samples
    void (*play)(short buf[], int samples);
    void (*stop)(void);

    // may be NULL, in which case soft volume is applied
    void (*volume)(int vol);
} audio_output;

audio_output *audio_get_output(char *name);
void audio_ls_outputs(void);
int alsa_init(int argc, char **argv);
void alsa_deinit(void);
void alsa_play(short buf[], int samples);
void alsa_start(int sample_rate,int  channels,int formats,int vol);
void alsa_volume(int vol);

#endif //_AUDIO_H
