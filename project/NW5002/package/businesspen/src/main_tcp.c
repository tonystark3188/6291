#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "mysignal.h"
#include "common.h"
#include "tcp_server.h"

#define MINFREESPACE (5*1024)//5MB
#define MAXFREETIME 60
#define POWERKEYTIME 3

#define SIGPENKEYUP 60
#define SIGPENKEYDOWN 61

#define PENKEYUPVALUE (1)//01b
#define PENKEYDOWNVALUE (2)//10b
#define PENKEYRECORD (3)//11b
#define BATTERY_OFFSET (7)
#define PENKEYPOWERDOWN (0xf)

#define RECORDPROGRESS "dmicRecord"

#define KEY_SCAN 1
#define SHUTDOWN 2

#define LOG_ERR(fmt, args...) fprintf(stderr, fmt, ##args)
#define LOG_MSG(fmt, args...) fprintf(stdout, fmt, ##args)
#define LOG_DEB(fmt, args...) fprintf(stdout, fmt, ##args)
#ifndef MTAB_FILE
#define MTAB_FILE "/proc/self/mounts"
#endif
#ifndef PATH_PREFIX
#define PATH_PREFIX "/tmp/mnt/"
#endif


#define LED_ON 				0x11
#define LED_OFF				0x10
#define LED_BLINK			0x01


int g_logLevel = 0;
static int g_exit_flag = 0;
static int gkeyfd = 0;
static int g_socketport = 6068;
//char g_sdcartpath[64]={0};

int getExitFlag(void)
{
    return g_exit_flag;
}

void setExitFlag(int value)
{
    g_exit_flag = value;
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
	   /* fseek(fd, 0, SEEK_END);//seek to end
        len = ftell(fd);//get size
        fseek(fd, 0, SEEK_SET);//seek to start*/
	    bufall = malloc(len+1);
		memset(bufall, 0, len+1);
		fread(bufall, 1, len, fd);
        //printf("bufall=%s\n\n len=%d\n",bufall,len);
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
			debug(1,"get path %s\n", diskpath);
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

//df /tmp/mnt/SD-disk-p4/ | grep mmc | awk -F ' ' '{print $4}'

static int getFreeSpace(void)
{	
    FILE *pp=NULL;	
	char buf[16]={0};
	char hotdiskpath[128] = {0};
	char cmdstr[256] = {0};
	int freevalue = 0;

    if(getHotplugDiskPath(hotdiskpath))
		return RETERROR;

    snprintf(cmdstr,sizeof(cmdstr),"df %s | grep mmc | awk -F ' ' '{print $4}'",hotdiskpath);
	if((pp=popen(cmdstr,"r"))!=NULL)
	{
	    if((fgets(buf,16,pp))!=NULL)
		{
		    freevalue = atoi(buf);	    
		}
		pclose(pp);	
	}
	debug(1,"freespace=%d\n",freevalue);
	return freevalue;
}

static void mySignalHandle(int sig)
{
	char cmdstr[256] = {0};

    ClientClose(sig);
	if(0 == g_exit_flag)
	{
	    setExitFlag(1);
	    sleep(1);//wait record
	}
	else
	{
        printf("already come here,so exit direct!\n");
		exit(1);
	}
	printf("\n\n[%s: %d] penserver application crashed by signal %s(%d).\n", __FUNCTION__, __LINE__, signal_str[sig],sig);

	if (sig == SIGSEGV || sig == SIGBUS ||
	    sig == SIGTRAP || sig == SIGABRT) {
		sprintf(cmdstr, "cat /proc/%d/maps", getpid());
		printf("Process maps:\n");
		system(cmdstr);
	}
	
}


static int CheckProgressCount(const char *progress)
{
	FILE *pp;
	char buf[32] = {0};
	char cmd[128] = {0};
	//char *str1;
	int find=0;

	snprintf(cmd,sizeof(cmd),"ps -w |grep %s | grep -v grep | wc -l",progress);
	
	if((pp=popen(cmd,"r"))==NULL)
	{
		perror("popen");
		return -1;
	}
	if(fgets(buf,sizeof(buf)-1,pp)!=NULL)
	    find = atoi(buf);
	
	pclose(pp);

	return find;
}

static void stopRecord(void)
{
    char cmdstr[64] = {0};
    if(CheckProgressCount(RECORDPROGRESS))
    {
        sprintf(cmdstr,"killall %s",RECORDPROGRESS);
        system(cmdstr);
		sleep(3);
	}
	if(CheckProgressCount(RECORDPROGRESS))
	{
	    printf("WARINING!!!stop dmicRecord failed try again\n");
	    system(cmdstr);
	}
}

static void startRecord(void)
{
    system("recordrestart &");
}

static void usage(void)
{    
    printf("\n");
    printf("Options:\n");
    printf("    --help          show this help\n");    
    printf("    --log     show debug log\n");	
    printf("    --port PORT     set socket port\n");
    //printf("    --log2     show high level debug log\n");
}
static int process_cmdline(int argc, char **argv)
{    
    int count = argc;    
    int i = 0;
	
    if(count == 1)    
	    return 0;    
    count--;    

	for(i=1;i<=count;)    
	{        
	    if(!strcmp("--port", argv[i]))      
		{           
		    g_socketport = atoi(argv[i+1]);    
		    i+=2;            
			continue;  
		}
	    else if(!strcmp("--log", argv[i])) 
		{     
		    g_logLevel++;
			i++;
		    continue;
		}
	    else if(!strcmp("--help", argv[i])) 
		{     
		    usage();        
		    exit(0);       
		}
    }    
    return 0;
}

static void wifiLedBrinkThree(int flag)
{
    int fd = 0;
    if ( (fd = open("/proc/jz_gpio_wifi", 0)) < 0) {
        LOG_DEB("thread open jz_gpio_wifi file error:%s\n", strerror(errno));
        return ;
    }
	if(flag == LED_ON)
	{
		ioctl(fd, LED_ON);
        ioctl(fd, LED_ON);
	}
	else if(flag == LED_BLINK)
	{
        ioctl(fd, LED_OFF);
	    usleep(300000);
        ioctl(fd, LED_ON);
	    usleep(300000);
        ioctl(fd, LED_OFF);
	    usleep(300000);
        ioctl(fd, LED_ON);
	    usleep(300000);
        ioctl(fd, LED_OFF);
	    usleep(300000);
        ioctl(fd, LED_ON);
	}
	else
	{
	    ioctl(fd, LED_OFF);
        ioctl(fd, LED_OFF);
	}
	close(fd);
}/**/
int main(int argc,char *argv[])
{
    int canRecordFlag = 0;
	int isRecording = 0;
	int powerkeyvalue = 0;
    struct timeval CurrentTime = {0,0};    
	int CurrentTime_seconds = 0;
	int StartTime_seconds = 0; 
	int powerStartTime_seconds = 0;
	int powertimeDuration = 0;
	int timeDuration = 0;
    int keyvalue = 0;
	pthread_t socket_thread;
	int socketret = 0;
	int battery_value = 0;

    process_cmdline(argc,argv);
	
	if(getFreeSpace() < MINFREESPACE)
		canRecordFlag = 0;
	else
		canRecordFlag = 1;
		
    signal_setup(mySignalHandle);
	if(g_socketport != 6068)	
	{	    
	    PRINTFWARNING("g_socketport != 6068");		
		printf("port=%d\n",g_socketport);        	
	}    

	socketret = pthread_create(&socket_thread, NULL, startSocketServer, &g_socketport);
	if (socketret != 0) {
		perror("ERROR: create socket thread failed.");
		exit(1);
	}/**/

    if ( (gkeyfd = open("/proc/penkey_ioctl", 0)) < 0) {
        LOG_DEB("thread open ioctl file error:%s\n", strerror(errno));
        return -1;
    }

	gettimeofday(&CurrentTime,NULL);    
	StartTime_seconds = CurrentTime_seconds = CurrentTime.tv_sec;
    while(1)
	{
	    if(getExitFlag())
		{
		    printf(">>>>>>>>>>>>>exit the pregress!!!!!!!!!!\n");
		    break;
		}
	    gettimeofday(&CurrentTime,NULL);
		CurrentTime_seconds = CurrentTime.tv_sec;
		timeDuration = CurrentTime_seconds - StartTime_seconds;

        if(powerkeyvalue)//get power key press time duration
			powertimeDuration = CurrentTime_seconds - powerStartTime_seconds;
		else
		{
		    powertimeDuration = 0;
		    powerStartTime_seconds = CurrentTime_seconds;
		}

        if(timeDuration > MAXFREETIME)
		{
			StartTime_seconds = CurrentTime_seconds;
			if(getFreeSpace() < MINFREESPACE)//check free space
			{
			    canRecordFlag = 0;
				stopRecord();
				sendActiontoClient(ACT_DEVICEBUSY);
			}
			else
			{
			    if(0 == CheckProgressCount(RECORDPROGRESS))
			        canRecordFlag = 1;
			}
		}

		if(powertimeDuration >= POWERKEYTIME)
		{
            printf("\n****************power down the device!****************\n\n");
			stopRecord();
			wifiLedBrinkThree(LED_OFF);
            ioctl(gkeyfd, SHUTDOWN);
			sleep(1);
			ioctl(gkeyfd, SHUTDOWN);
		}

/*****************test******************
sleep(15);
sendActiontoClient(ACT_DOWN);
sleep(10);
sendActiontoClient(ACT_UP);
sleep(10);
sendActiontoClient(ACT_DEVICEBUSY);

/**********************************/
        keyvalue = ioctl(gkeyfd, KEY_SCAN);
        if (keyvalue !=0) 
        {
            LOG_DEB("pressed key:%d\n", keyvalue);
        }
		if(PENKEYPOWERDOWN != keyvalue)
		{
		    battery_value = keyvalue&(0x1<<BATTERY_OFFSET);
			if(battery_value)
			{
                printf("Battery low!!!\n");
			}
            keyvalue = keyvalue&(0x7f);
			powerkeyvalue = 0;//reset power value
		}
        switch(keyvalue) {
            case PENKEYUPVALUE:
                debug(1,"PENKEY UP pressed\n");
                sendActiontoClient(ACT_UP);
                usleep(100000);
                break;

            case PENKEYDOWNVALUE:
                debug(1,"PEN KEY DOWN pressed\n");
                sendActiontoClient(ACT_DOWN);
				usleep(100000);
                break;
			case PENKEYRECORD:
				debug(1,"RECORD key pressed\n");
				if(canRecordFlag)
				{
				    wifiLedBrinkThree(LED_BLINK);
                    startRecord();
                    canRecordFlag = 0;
					//sleep(2);
					//wifiLedBrinkThree(fd_led);
				}
				else
				{
				    //ioctl(fd_led, LED_OFF);
	                stopRecord();
					canRecordFlag = 1;
					sleep(2);
				}

				break;
			case PENKEYPOWERDOWN:
				debug(1,"POWER key pressed\n");
				powerkeyvalue = 1;
        }
		usleep(100000);
    }
	ClientClose(0);
    if(gkeyfd)
        close(gkeyfd);
	if (socketret == 0) 
	{
		pthread_join(socket_thread, NULL);
	}

}

