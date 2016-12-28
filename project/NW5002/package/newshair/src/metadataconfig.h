#ifndef __METADATACONFIG_H
#define __METADATACONFIG_H

#define RET_INIT 0
#define RET_SUCCESS 1
#define RET_ERROR -1

#ifndef MAXURLSIZE
#define MAXURLSIZE 1024
#endif

typedef enum __MUSICSERVICEType{
    MST_SELF = 0,
    MST_AIRPLAY = 1,
    MST_DLNA=2,
    MST_PC=3,
	
    MST_COUNT,
}MusicServiceType;

typedef enum __MUSICHANNEL{
    CHUNKNOW = 0,
    CH_MONO  = 1,
    CH_STERO =2,
    CH_51 = 6,
    CH_71 = 8,
    CH_COUNT,
}MusicChannel;

typedef enum {
        ACTIONSTOP=0,
        ACTIONPLAY,
        ACTIONPAUSE,
        ACTIONPRE,
        ACTIONNEXT,
        ACTIONCONTROLCOUNT
}PLAYACTIONCONTROL;


void InitMetadataConfig(void);//

int setProgressConfig(int value);

int setPlaystateConfig(PLAYACTIONCONTROL value);//

int setVolumeConfig(int value);//

int setSongpathConfig(const char *value);//

int setServicetypeConfig(MusicServiceType value);//

int setChannelConfig(MusicChannel value);//

int setLengthConfig(int value);

int setSimplerateConfig(const char *value);//

int setBitrateConfig(int value);//

int setTitleConfig(const char *value);//

int setCreatorConfig(const char *value);//

int setAlbumConfig(const char *value);//


/*-------------------------------------------------------------------*/

PLAYACTIONCONTROL getPlaystateConfig(void);

int getVolumeConfig(void);

int getProgressConfig(void);

char *getSongpathConfig(void);

MusicServiceType getServicetypeConfig(void);

MusicChannel getChannelConfig(void);

int getLengthConfig(void);

char *getSimplerateConfig(void);

char *getBitrateConfig(void);

char *getTitleConfig(void);

char *getCreatorConfig(void);

char *getAlbumConfig(void);


#endif
