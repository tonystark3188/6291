#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>

#include "common.h"
#include "filemanager.h"

#ifndef MTAB_FILE
#define MTAB_FILE "/proc/self/mounts"
#endif
#ifndef PATH_PREFIX
#define PATH_PREFIX "/tmp/mnt/"
#endif

#define DEFAULTTIMESTR "Raudio2015-12-15-11-50-22"

static int g_num = 1;


static void checkFileExist(char *filepath)
{
    FILE *fp = NULL;
	int i = 0;
	int len = strlen(filepath) - 1;

	if(NULL != OpenfileToRead(fp,filepath))
	{/*already exist*/
	    if(g_num == 1)
		{
		    sprintf(filepath,"%s(%d)",filepath,g_num);
			g_num += 1;
		}
		else
		{
		    while(len > 0)
		    {
                if(*(filepath+len) == '(')
                {
                   *(filepath+len) = '\0';
				   break;
				}
			}
 		    sprintf(filepath,"%s(%d)",filepath,g_num);
			g_num += 1;
		}
		fclose(fp);
		checkFileExist(filepath);
	}
}


static char *getTimeStr(void)
{
    time_t timep;   
    struct tm *p;
    char *timestr = NULL;   
    time(&timep);   
    p =localtime(&timep);

    timestr = (char *)malloc(32);
    if(timestr)
    {
        sprintf(timestr,"Raudio%04d-%02d-%02d-%02d-%02d-%02d",(1900+p->tm_year),(1+p->tm_mon),(p->tm_mday),(p->tm_hour),(p->tm_min),(p->tm_sec));
    }
    else
       printf("malloc timestr==NULL %s\n",__FUNCTION__);     //printf("timestr = %s\n",timestr);
    return timestr;
}

static int getHotplugDiskPath(char *diskpath)
{
	FILE *fd=0;
	int len=2048;
	char *bufall = NULL;
	int i = 0;;
	char *str = NULL;
	
	fd = fopen(MTAB_FILE, "r");   
	if(fd != 0)
	{
	    /*fseek(fd, 0, SEEK_END);//seek to end
        len = ftell(fd);//get size
        fseek(fd, 0, SEEK_SET);//seek to start*/
	
	    bufall = malloc(len+1);
		memset(bufall, 0, len+1);
		fread(bufall, 1, len, fd);
      
		str = strstr(bufall, PATH_PREFIX);
		if(str)
		{
			//get file path
			i = 0;
			for(;str[i]!= ' '; i++)
			{
			    diskpath[i]=str[i];
				if(i > 63)
				{
			        FreeBuf(bufall);
			        fclose(fd);
			        printf("ERROR!It must be a invalid path %s\n", diskpath);
			        return RETERROR;
				}
			}
			diskpath[i] = '/';
			diskpath[i+1] = 0;
			FreeBuf(bufall);
			fclose(fd);
			printf("get path %s\n", diskpath);
			return RETSUCCESS;
		}
		else
		{    
			fflush(fd);
			fclose(fd);
			FreeBuf(bufall);
			printf("ERROR!Didn't find %s in %s\n", PATH_PREFIX,MTAB_FILE);
			return RETERROR;
		}
		
	}
	else
	{
	    printf("ERROR! open %s failed\n",MTAB_FILE);
		FreeBuf(bufall);
		return RETERROR;//no found file
	}
}


int getRecordFilename(char *filepath)
{
    char diskpath[64]={0};
	char *timestr = NULL;

    if(getHotplugDiskPath(diskpath))
    {
        return RETERROR;
	}
	
    timestr = getTimeStr();
	if(NULL == timestr)
	{
        PRINTFWARNING("WARNING! NULL == timestr\n");
		snprintf(filepath,MAXPATHSIZE-16,"%s%s",diskpath,DEFAULTTIMESTR);
	}
	else
	{
	    snprintf(filepath,MAXPATHSIZE-16,"%s%s",diskpath,timestr);
        free(timestr);
	}
	g_num = 1;
	checkFileExist(filepath);
	//strcat(filepath,".amr");
	return RETSUCCESS;
}
