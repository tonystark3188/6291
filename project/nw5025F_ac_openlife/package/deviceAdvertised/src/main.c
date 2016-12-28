/*
 *  Shenzhen LongSys Electronics Co.,Ltd All rights reserved.
 *
 * The source code   are owned by  Shenzhen LongSys Electronics Co.,Ltd.
 * Corporation or its suppliers or licensors. Title LongSys or its suppliers and
 * licensors.  The Material contains trade secrets and proprietary
 * and confidential information of LongSys or its suppliers and
 * licensors. 
 *
 */


/*****************
NOTICE:This module just work for upnp device advertise and there is no service.

=========HELP===========
--version : display version number.
--help : display command help.
--ip-address : assign IP address for this application.
--socket-port : set socket communication port.       //only for airdisk3.0 hot tips
	 --ip-address xxx.xxx.xxx.xxx
--uuid : assign uuid for this application.
	 --uuid xxxxxxxxxxxx
--friendly-name : assign friendly name for this application.
	 --friendly-name xxxxxxxxxxxx
--dump-devicedesc : dump device descriptor XML and exit.
=========HELP===========

for example:
     deviceAdvertised --ip-address 192.168.222.254 --socket-port 8000 --uuid xxxxxxxxx

if you want to read the device describle file just execute "deviceAdvertised --dump-devicedesc"

if you want to change the device describle file, please consult function generate_desc int file upnp.c

if you want to change the type of your device, please change the device_type of struct struct device render_device,which is in file upnp_renderer.c

********************/

#include "config.h"


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>

#include <glib.h>

#include <upnp/ithread.h>
#include <upnp/upnp.h>
#include "string.h"
#include "logging.h"
#include "upnp.h"
#include "upnp_device.h"
#include "upnp_renderer.h"

static gboolean show_version = FALSE;
static gboolean show_help = FALSE;
static gboolean show_devicedesc = FALSE;
static gboolean show_connmgr_scpd = FALSE;
static gboolean show_control_scpd = FALSE;
static gboolean show_transport_scpd = FALSE;
static char *ip_address = NULL;
static char *SocketProt = NULL;
static char *uuid = "e6572b54-f3c7-2d91-2fb5-b757f2537e21";
static char *friendly_name = PACKAGE_NAME;
char my_uuid[128];
char my_friendly_name[128];
int g_duration = 300;


static void exit_loop_sighandler(int sig) 
{
                                    //printf("	UpnpFinish start!\n");
    UpnpFinish();        //printf("	UpnpFinish over!\n");
                            //printf("leave %s\n",__FUNCTION__);
    if(ip_address)
       free(ip_address);
    if(SocketProt)
       free(SocketProt);
    exit(0);

}/**/

static void do_show_version(void)
{
	puts( PACKAGE_STRING "\n"
        	"SORRY!This is no a free software.\n "
	);
}
static void do_show_help(void)
{
    printf("Command format : gmediarender [option]\n");
    printf("--version : display version number.\n");
    printf("--help : display command help.\n");
    printf("--ip-address : assign IP address for this application.\n");
    printf("\t --ip-address xxx.xxx.xxx.xxx\n");
    printf("--socket-port : set socket communication port.\n");
    printf("--uuid : assign uuid for this application.\n");
    printf("\t --uuid xxxxxxxxxxxx\n");
    printf("--duration : assign lifecycle for advertised in seconds.\n");
    printf("\t --duration xx\n");
    printf("--friendly-name : assign friendly name for this application.\n");
    printf("\t --friendly-name xxxxxxxxxxxx\n");
    printf("--dump-devicedesc : dump device descriptor XML and exit.\n");
   /* printf("--dump-connmgr-scpd : dump Connection Manager service description XML and exit.\n");
    printf("--dump-control-scpd : dump Rendering Control service description XML and exit.\n");
    printf("--dump-transport-scpd : Dump A/V Transport service description XML and exit.\n");*/
    printf("\n");

}
static int process_cmdline(int argc, char **argv)
{
	int count = argc;
	int i;

    if(count == 1)
        return 0;

    count--;

    for(i=1;i<=count;)
    {
        if(!strcmp("--version", argv[i]))
        {
            show_version = TRUE;
            return 0;
        }
        else if(!strcmp("--help", argv[i]))
        {
            show_help = TRUE;
            return 0;
        }
        else if(!strcmp("--ip-address", argv[i]))
        {
            ip_address = (char *)malloc(strlen(argv[i+1]) + 1);
            if(!ip_address)
                return -1;
            strcpy(ip_address, argv[i+1]);
            i+=2;
        }
        else if(!strcmp("--duration", argv[i]))
        {
            g_duration = atoi(argv[i+1]);
            i+=2;
        }
        else if(!strcmp("--socket-port", argv[i]))
        {
            SocketProt = (char *)malloc(strlen(argv[i+1]) + 1);
            if(!SocketProt)
                return -1;
            strcpy(SocketProt, argv[i+1]);
            i+=2;
        }
        else if(!strcmp("--uuid", argv[i]))
        {
            strcpy(my_uuid, argv[i+1]);
            i+=2;
        }
        else if(!strcmp("--friendly-name", argv[i]))
        {
            strcpy(my_friendly_name, argv[i+1]);
            i+=2;
        }
        else if(!strcmp("--dump-devicedesc", argv[i]))
        {
            show_devicedesc = TRUE;
            return 0;
        }
        else if(!strcmp("--dump-connmgr-scpd", argv[i]))
        {
            show_connmgr_scpd = TRUE;
            return 0;
        }
        else if(!strcmp("--dump-control-scpd", argv[i]))
        {
            show_control_scpd = TRUE;
            return 0;
        }
        else if(!strcmp("--dump-transport-scpd", argv[i]))
        {
            show_transport_scpd = TRUE;
            return 0;
        }
        else
        {
            return -1;
        }
    }
	return 0;
}

int main(int argc, char **argv)
{
	int rc;
	int result = EXIT_FAILURE;
	struct device *upnp_renderer;

	memset(my_uuid, 0x0, sizeof(my_uuid));
    memset(my_friendly_name, 0x0, sizeof(my_friendly_name));
	
	aNewOneIn();

	rc = process_cmdline(argc, argv);
	if (rc != 0) {
		goto out;
	}

	if (show_version) {
		do_show_version();
		exit(EXIT_SUCCESS);
	}
	if (show_help) {
		do_show_help();
		exit(EXIT_SUCCESS);
	}
	/*if (show_connmgr_scpd) {
		upnp_renderer_dump_connmgr_scpd();
		exit(EXIT_SUCCESS);
	}
	if (show_control_scpd) {
		upnp_renderer_dump_control_scpd();
		exit(EXIT_SUCCESS);
	}
	if (show_transport_scpd) {
		upnp_renderer_dump_transport_scpd();
		exit(EXIT_SUCCESS);
	}*/

    if(strlen(my_friendly_name) == 0)
    {
        strcpy(my_friendly_name, friendly_name);
    }
    if(strlen(my_uuid) == 0)
    {
        strcpy(my_uuid, uuid);
    }

	upnp_renderer = upnp_renderer_new(my_friendly_name, my_uuid);
	if (upnp_renderer == NULL) {printf("out 3\n");
		goto out;
	}

	if (show_devicedesc) {
                char *desc = NULL;
                desc = upnp_get_device_desc(upnp_renderer,ip_address,SocketProt);
		fputs(desc, stdout);
                if(desc)
                    free(desc);
		exit(EXIT_SUCCESS);
	}

	rc = upnp_device_init(upnp_renderer, ip_address,SocketProt);
	if (rc != 0) {printf("out 2\n");
		goto out;
	}
	signal(SIGINT, &exit_loop_sighandler);//ctrl+c
	signal(SIGTERM, &exit_loop_sighandler);//kill
	signal(SIGSEGV, &exit_loop_sighandler);//segmentfault
	signal(SIGBUS, &exit_loop_sighandler);//bus error/**/
        result = EXIT_SUCCESS;

        while(1)
        {
           usleep(5000);
        }

out:
        if(ip_address)
           free(ip_address);

	return result;
}



