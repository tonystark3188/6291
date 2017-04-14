#include <stdio.h>
#include <stdlib.h>

#include "uci_for_render.h"
#include "metadataconfig.h"

void InitMetadataConfig(void)
{
   /*setProgressConfig(0);
   //setVolumeConfig(100);
   setSongpathConfig("unknow");
   setServicetypeConfig(MST_AIRPLAY);
   setChannelConfig(CH_STERO);
   setLengthConfig(0);
   setSimplerateConfig("44100");
   setBitrateConfig(1411);
   setTitleConfig("unknow");
   setCreatorConfig("unknow");
   setAlbumConfig("unknow");*/
}

int setPlaystateConfig(PLAYACTIONCONTROL value)
{
	/*char uci_option_str[64] = {0};

    if(value < ACTIONSTOP || value > ACTIONPAUSE)
		return RET_ERROR;
	
    ctx=uci_alloc_context();           
    sprintf(uci_option_str,"autoplayconfig.@recordinfo[0].playstate=%d",value);
	uci_set_option_value(uci_option_str);
	if(ctx)
	{
        uci_free_context(ctx);
	    ctx = NULL;
	}
    system("uci commit");*/

    return RET_SUCCESS;

}

int setProgressConfig(int value)
{
	/*char uci_option_str[64] = {0};

    value = (value > 100)?100:value;
	value = (value < 0)?0:value;
	
    ctx=uci_alloc_context();           
    sprintf(uci_option_str,"autoplayconfig.@recordinfo[0].progress=%d",value);
	uci_set_option_value(uci_option_str);
	if(ctx)
	{
        uci_free_context(ctx);
	    ctx = NULL;
	}
    system("uci commit");*/

    return RET_SUCCESS;
}

int setVolumeConfig(int value)
{
	/*char uci_option_str[64] = {0};

    value = (value > 100)?100:value;
	value = (value < 0)?0:value;
	
    ctx=uci_alloc_context();           
    sprintf(uci_option_str,"autoplayconfig.@recordinfo[0].volume=%d",value);
	uci_set_option_value(uci_option_str);
	if(ctx)
	{
        uci_free_context(ctx);
	    ctx = NULL;
	}
    system("uci commit");*/

    return RET_SUCCESS;
}
int setSongpathConfig(const char *value)
{	
	/*char uci_option_str[MAXURLSIZE+64] = {0};
		
	if(NULL == value)
		return RET_ERROR;
		
	ctx=uci_alloc_context();		   
	sprintf(uci_option_str,"autoplayconfig.@recordinfo[0].songpath=%s",value);
	uci_set_option_value(uci_option_str);
	if(ctx)
	{
		uci_free_context(ctx);
		ctx = NULL;
	}
	system("uci commit");*/
		
	return RET_SUCCESS;
}
int setServicetypeConfig(MusicServiceType value)
{
	/*char uci_option_str[64] = {0};

    if((value < MST_SELF) || (value >= MST_COUNT))
	    return RET_ERROR;
	
    ctx=uci_alloc_context();           
    sprintf(uci_option_str,"autoplayconfig.@recordinfo[0].servicetype=%d",value);
	uci_set_option_value(uci_option_str);
	if(ctx)
	{
        uci_free_context(ctx);
	    ctx = NULL;
	}
    system("uci commit");*/

    return RET_SUCCESS;

}

int setChannelConfig(MusicChannel value)
{
	/*char uci_option_str[64] = {0};

    if((value < CH_MONO) || (value >= CH_COUNT))
	    return RET_ERROR;
	
    ctx=uci_alloc_context();           
    sprintf(uci_option_str,"autoplayconfig.@recordinfo[0].channel=%d",value);
	uci_set_option_value(uci_option_str);
	if(ctx)
	{
        uci_free_context(ctx);
	    ctx = NULL;
	}
    system("uci commit");*/

    return RET_SUCCESS;

}
int setLengthConfig(int value)
{
	/*char uci_option_str[64] = {0};

    ctx=uci_alloc_context();           
    sprintf(uci_option_str,"autoplayconfig.@recordinfo[0].length=%d",value);
	uci_set_option_value(uci_option_str);
	if(ctx)
	{
        uci_free_context(ctx);
	    ctx = NULL;
	}
    system("uci commit");*/

    return RET_SUCCESS;
}
int setSimplerateConfig(const char *value)
{
	/*char uci_option_str[64] = {0};

    ctx=uci_alloc_context();           
    sprintf(uci_option_str,"autoplayconfig.@recordinfo[0].simplerate=%s",value);
	uci_set_option_value(uci_option_str);
	if(ctx)
	{
        uci_free_context(ctx);
	    ctx = NULL;
	}
    system("uci commit");*/

    return RET_SUCCESS;
}
int setBitrateConfig(int value)
{
	/*char uci_option_str[64] = {0};

	if(NULL == value)
		return RET_ERROR;

    ctx=uci_alloc_context();           
    sprintf(uci_option_str,"autoplayconfig.@recordinfo[0].bitrate=%d",value);
	uci_set_option_value(uci_option_str);
	if(ctx)
	{
        uci_free_context(ctx);
	    ctx = NULL;
	}
    system("uci commit");*/

    return RET_SUCCESS;
}

int setTitleConfig(const char *value)
{
	/*char uci_option_str[256] = {0};

	if(NULL == value)
		return RET_ERROR;

    ctx=uci_alloc_context();           
    sprintf(uci_option_str,"autoplayconfig.@recordinfo[0].title=%s",value);
	uci_set_option_value(uci_option_str);
	if(ctx)
	{
        uci_free_context(ctx);
	    ctx = NULL;
	}
    system("uci commit");*/

    return RET_SUCCESS;
}

int setCreatorConfig(const char *value)
{
	/*char uci_option_str[256] = {0};

	if(NULL == value)
		return RET_ERROR;

    ctx=uci_alloc_context();           
    sprintf(uci_option_str,"autoplayconfig.@recordinfo[0].creator=%s",value);
	uci_set_option_value(uci_option_str);
	if(ctx)
	{
        uci_free_context(ctx);
	    ctx = NULL;
	}
    system("uci commit");*/

    return RET_SUCCESS;

}

int setAlbumConfig(const char *value)
{
	/*char uci_option_str[256] = {0};

	if(NULL == value)
		return RET_ERROR;

    ctx=uci_alloc_context();           
    sprintf(uci_option_str,"autoplayconfig.@recordinfo[0].album=%s",value);
	uci_set_option_value(uci_option_str);
	if(ctx)
	{
        uci_free_context(ctx);
	    ctx = NULL;
	}
    system("uci commit");*/

    return RET_SUCCESS;

}

/*///////////////////////////////////////////////////
option 'songpath' 'unknow'	
option 'playtype' '0'	
option 'channel' '2'	
option 'length' '0' 
option 'simplerate' '0' 
option 'bitrate' '0'	
option 'title' 'unknow' 
option 'creator' 'unknow'	
option 'album' 'unknow'
///////////////////////////////////////////////////*/

/*PLAYACTIONCONTROL getPlaystateConfig(void)
{
	char uci_option_str[64] = {0};
    char ucivalue[8] = {0};
     
    ctx=uci_alloc_context();           
    strcpy(uci_option_str,"autoplayconfig.@recordinfo[0].playstate");
	uci_get_option_value(uci_option_str,ucivalue);
	if(ctx)
	{
        uci_free_context(ctx);
	    ctx = NULL;
	}

    return atoi(ucivalue);
}



char *getSongpathConfig(void)
{
	char uci_option_str[64] = {0};
	char *songpath = NULL;
    char ucivalue[MAXURLSIZE] = {0};
     
    ctx=uci_alloc_context();           
    strcpy(uci_option_str,"autoplayconfig.@recordinfo[0].songpath");
	uci_get_option_value(uci_option_str,ucivalue);
	if(ctx)
	{
        uci_free_context(ctx);
	    ctx = NULL;
	}
	songpath = (char*)calloc(MAXURLSIZE,sizeof(char));
	if(NULL == songpath)
		return NULL;
	strncpy(songpath,ucivalue,MAXURLSIZE);

    return songpath;

}

MusicServiceType getServicetypeConfig(void)
{
	char uci_option_str[64] = {0};
    char ucivalue[8] = {0};
     
    ctx=uci_alloc_context();           
    strcpy(uci_option_str,"autoplayconfig.@recordinfo[0].servicetype");
	uci_get_option_value(uci_option_str,ucivalue);
	if(ctx)
	{
        uci_free_context(ctx);
	    ctx = NULL;
	}

    return atoi(ucivalue);

}
MusicChannel getChannelConfig(void)
{
	char uci_option_str[64] = {0};
    char ucivalue[8] = {0};
     
    ctx=uci_alloc_context();           
    strcpy(uci_option_str,"autoplayconfig.@recordinfo[0].channel");
	uci_get_option_value(uci_option_str,ucivalue);
	if(ctx)
	{
        uci_free_context(ctx);
	    ctx = NULL;
	}

    return atoi(ucivalue);

}
int getLengthConfig(void)
{
	char uci_option_str[64] = {0};
    char ucivalue[8] = {0};
     
    ctx=uci_alloc_context();           
    strcpy(uci_option_str,"autoplayconfig.@recordinfo[0].length");
	uci_get_option_value(uci_option_str,ucivalue);
	if(ctx)
	{
        uci_free_context(ctx);
	    ctx = NULL;
	}

    return atoi(ucivalue);

}
char *getSimplerateConfig(void)
{
	char uci_option_str[64] = {0};
	char *simplerate = NULL;
    char ucivalue[32] = {0};
     
    ctx=uci_alloc_context();           
    strcpy(uci_option_str,"autoplayconfig.@recordinfo[0].simplerate");
	uci_get_option_value(uci_option_str,ucivalue);
	if(ctx)
	{
        uci_free_context(ctx);
	    ctx = NULL;
	}
	simplerate = (char*)calloc(32,sizeof(char));
	if(NULL == simplerate)
		return NULL;
	strncpy(simplerate,ucivalue,32);

    return simplerate;

}
char *getBitrateConfig(void) 
{
	char uci_option_str[64] = {0};
   // char ucivalue[8] = {0};
   char *ucivalue = NULL;

	ucivalue = (char*)calloc(16,sizeof(char));
    ctx=uci_alloc_context();           
    strcpy(uci_option_str,"autoplayconfig.@recordinfo[0].bitrate");
	uci_get_option_value(uci_option_str,ucivalue);
	if(ctx)
	{
        uci_free_context(ctx);
	    ctx = NULL;
	}

    return ucivalue;//atoi(ucivalue);

}

char *getTitleConfig(void)
{
	char uci_option_str[64] = {0};
	char *title = NULL;
    char ucivalue[64] = {0};
     
    ctx=uci_alloc_context();           
    strcpy(uci_option_str,"autoplayconfig.@recordinfo[0].title");
	uci_get_option_value(uci_option_str,ucivalue);
	if(ctx)
	{
        uci_free_context(ctx);
	    ctx = NULL;
	}
	title = (char*)calloc(64,sizeof(char));
	if(NULL == title)
		return NULL;
	strncpy(title,ucivalue,64);

    return title;

}

char *getCreatorConfig(void)
{
	char uci_option_str[64] = {0};
	char *creator = NULL;
    char ucivalue[64] = {0};
     
    ctx=uci_alloc_context();           
    strcpy(uci_option_str,"autoplayconfig.@recordinfo[0].creator");
	uci_get_option_value(uci_option_str,ucivalue);
	if(ctx)
	{
        uci_free_context(ctx);
	    ctx = NULL;
	}
	creator = (char*)calloc(64,sizeof(char));
	if(NULL == creator)
		return NULL;
	strncpy(creator,ucivalue,64);

    return creator;

}

char *getAlbumConfig(void)
{
	char uci_option_str[64] = {0};
	char *album = NULL;
    char ucivalue[64] = {0};
     
    ctx=uci_alloc_context();           
    strcpy(uci_option_str,"autoplayconfig.@recordinfo[0].album");
	uci_get_option_value(uci_option_str,ucivalue);
	if(ctx)
	{
        uci_free_context(ctx);
	    ctx = NULL;
	}
	album = (char*)calloc(64,sizeof(char));
	if(NULL == album)
		return NULL;
	strncpy(album,ucivalue,64);

    return album;

}


int getVolumeConfig(void)
{
	char uci_option_str[64] = {0};
    char ucivalue[8] = {0};
     
    ctx=uci_alloc_context();           
    strcpy(uci_option_str,"autoplayconfig.@recordinfo[0].volume");
	uci_get_option_value(uci_option_str,ucivalue);
	if(ctx)
	{
        uci_free_context(ctx);
	    ctx = NULL;
	}

    return atoi(ucivalue);
}


int getProgressConfig(void)
{
	char uci_option_str[64] = {0};
    char ucivalue[8] = {0};
     
    ctx=uci_alloc_context();           
    strcpy(uci_option_str,"autoplayconfig.@recordinfo[0].progress");
	uci_get_option_value(uci_option_str,ucivalue);
	if(ctx)
	{
        uci_free_context(ctx);
	    ctx = NULL;
	}

    return atoi(ucivalue);
}*/


