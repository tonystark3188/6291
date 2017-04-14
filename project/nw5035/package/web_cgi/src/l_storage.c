#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>


#include <mntent.h>
#include <sys/vfs.h>
#include <dirent.h>

#include <sys/stat.h>
#include <libudev.h>
#include <locale.h>
#include "msg.h"

#include "uci_for_cgi.h"
// /etc/config/system sid
#define SID "sid"

///////////////////////////////////////////////////////
//for storage//////////////////////////////////////////
///////////////////////////////////////////////////////
static const unsigned long long G = 1024*1024*1024ull;
static const unsigned long long M = 1024*1024;
static const unsigned long long K = 1024;

int getStorage(char **retstr)
{
	FILE* mount_table;
	struct mntent *mount_entry;
	struct statfs fs;
	unsigned long blocks_used;
	unsigned blocks_percent_used;
	char tmpi=0;
	char tmpstr[128]="\0";
	 struct udev *udev;
	 struct udev_enumerate *enumerate;
	 struct udev_list_entry *devices, *dev_list_entry;
	 struct udev_device *dev;
	 int i=0;
	 int j=0;
	  typedef struct myudevstruct{
	  char devpath[20];
	 // char * mountpoint;
	  char pid[10];
	  char vid[10];
	 }myudev;
	 myudev myusb[20];
	 myudev *p=myusb;

	 /* Create the udev object */
	 udev = udev_new();
	 if (!udev) {
	  printf("Can't create udev\n");       
	  exit(1);
	 }
	 
	 enumerate = udev_enumerate_new(udev);
	 udev_enumerate_add_match_subsystem(enumerate, "block");
	 udev_enumerate_scan_devices(enumerate);
	 devices = udev_enumerate_get_list_entry(enumerate);
	 udev_list_entry_foreach(dev_list_entry, devices) {
	  const char *path;
	  
	  path = udev_list_entry_get_name(dev_list_entry);
	  dev = udev_device_new_from_syspath(udev, path);

	  strcpy(p[i].devpath,udev_device_get_devnode(dev));
	  dev = udev_device_get_parent_with_subsystem_devtype(
	         dev,
	         "usb",
	         "usb_device");
	  if (!dev) {
	    break;
	  // printf("Unable to find parent usb device.");
	  // exit(1);
	  }
	 // p=myusb malloc(sizeof(myusb));
	 // myusb[i].pid=malloc(8*sizeof(char));
	  strcpy(p[i].vid,udev_device_get_sysattr_value(dev,"idVendor"));
	  strcpy(p[i].pid,udev_device_get_sysattr_value(dev, "idProduct"));
	  //printf("myusb[%d]:dev=%s,vid=%s,pid=%s\n",i, p[i].devpath,p[i].vid,p[i].pid);
	  i++;
	  udev_device_unref(dev);
	 }
	 /* Free the enumerator object */
	 udev_enumerate_unref(enumerate);

	 udev_unref(udev);

	  mount_table = setmntent("/proc/mounts", "r");
	  if (!mount_table)
	  {
	    fprintf(stderr, "set mount entry error\n");
	    return -1;
	  }
	  //printf("<Storage>");
	  strcpy(retstr,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{");
	  
	  while (1) {
	    const  char *device;
	    const char *mount_point;
	    if (mount_table) {
	      mount_entry = getmntent(mount_table);
	      if (!mount_entry) {
	        endmntent(mount_table);
	        break;
	      }
	    } 
	    else
	      break;
	    device = mount_entry->mnt_fsname;
	    mount_point = mount_entry->mnt_dir;
	  
	  //  sprintf(retstr, "mount info:mountpoint=%s\n",mount_point);
	    if(strstr(mount_point,"disk")==NULL) continue;
	     tmpi++;
	    if (statfs(mount_point, &fs) != 0) 
	    {
	      fprintf(stderr, "statfs failed!\n");  
	      break;
	    }
	    if ((fs.f_blocks > 0) || !mount_table ) 
	    {
	      blocks_used = fs.f_blocks - fs.f_bfree;
	      blocks_percent_used = 0;

	      if (strcmp(device, "rootfs") == 0)
	        continue;
	      sprintf(tmpstr,"\"volume\":\"%s\",",mount_point+9);//   /tmp/mnt/
	      //if (printf("\n<Section volume=\"%s\"" + 1, device) > 20)
	      //    printf("%1s", "");
	      //strcat(retstr,tmpstr);
	      memset(tmpstr,0,128);
	      for (j = 0; j < i; j++)
	      {

	        if(!strcmp(myusb[j].devpath,device))
	        {

	        //  printf("j=%d,myusbdevice=%s,device=%s\n",j,p[j].devpath,device);
	          sprintf(tmpstr,"\"vid\":\"%s\",\"pid\":\"%s\",",p[j].vid,p[j].pid);
	        //  printf(" vid=\"%s\" pid=\"%s\"",p[j].vid,p[j].pid);
	        }
	      }
	     // strcat(retstr,tmpstr);
	      memset(tmpstr,0,128);

		//cgi_log(mount_entry->mnt_opts);
		  if(strstr(mount_entry->mnt_opts,"rw")){
			  sprintf(tmpstr,"\"rw\":\"%s\",","rw");
			  //strcat(retstr,tmpstr);
	      	  memset(tmpstr,0,128);
		  }else{
			  sprintf(tmpstr,"\"rw\":\"%s\",","ro");
			  //strcat(retstr,tmpstr);
			  memset(tmpstr,0,128);

		  }
		  
	      char s1[20];
	      char s2[20];
	      char s3[20];
	    //  printf("total blocks=%10d\nblock size=%10d\nfree blocks=%10d\n",fs.f_blocks,fs.f_bsize,fs.f_bfree);
	      //strcpy(s1, kscale(fs.f_blocks, fs.f_bsize));
	      //strcpy(s2, kscale(fs.f_blocks - fs.f_bfree, fs.f_bsize));
	      //strcpy(s3, kscale(fs.f_bavail, fs.f_bsize));
	      sprintf(tmpstr,"\"total\":\"%lld\",\"used\":\"%lld\",\"free\":\"%lld\"}",
			  (unsigned long long)(fs.f_blocks*(unsigned long long)fs.f_bsize),
			  (unsigned long long)((fs.f_blocks - fs.f_bfree)* (unsigned long long)fs.f_bsize),
			  (unsigned long long)(fs.f_bavail*(unsigned long long)fs.f_bsize)
	          );
	      strcat(retstr,tmpstr);
	      memset(tmpstr,0,128);
	    }
	     
	  }
	   strcat(retstr,"}");
	/*
	char repair_state[32];
	memset(repair_state,0,32);
	memset(tmpstr,0,128);
	
	if(0 == access("/tmp/repair_doing", F_OK))
	{
		strcpy(repair_state, "repairing");
	}else if(0 == access("/tmp/repair_fail", F_OK))
		{
			strcpy(repair_state, "fail");	
		}else 
		{
			strcpy(repair_state, "normal");	
		}
	sprintf(tmpstr, "<Repair repair_state=\"%s\"></Repair>", repair_state);
	strcat(retstr, tmpstr);

	if(tmpi==0)
	{
	  strcat(retstr,"</Storage>");
	  //strcat(retstr,"</Storage><Return status=\"false\">Disk has already mount PC !</Return>");
	  //printf("</Storage><Return status=\"false\">Empty disk !</Return></getSysInfo>");
	  return -1;
	}

	strcat(retstr,"</Storage>");
*/
	return 0;

}


void main()
{
		char ret_buf[2048];
		char fw_sid[SID_LEN]="\0";
		char app_sid[SID_LEN]="\0";
		char new_code[CODE_LEN]="\0";		
		char *web_str=NULL;


		char uci_option_str[UCI_BUF_LEN]="\0";
		ctx=uci_alloc_context();

		strcpy(uci_option_str,"system.@system[0].sid");			//name
		uci_get_option_value(uci_option_str,fw_sid);
		memset(uci_option_str,'\0',UCI_BUF_LEN);

		printf("Content-type:text/plain\r\n\r\n");

		if((web_str=GetStringFromWeb())==NULL)
		{
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":4,\"errorMessage\":\"Can not get any parameters.\"}");
			printf("%s",ret_buf);
			fflush(stdout);
			uci_free_context(ctx);
			return ;
		}
		processString(web_str,SID,app_sid);
cgi_log(fw_sid);		
cgi_log(app_sid);		

		//if(!strcmp(fw_sid,app_sid))//是管理员
		if(1)//是管理员
		{
			getStorage(&ret_buf);
			//sprintf(ret_buf,"%s","{\"status\":1,\"data\":{\"isAdmin\":1}}");
			
		}else {//访客
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"Not Admin\"}");
			
		}
		
		printf("%s",ret_buf);
		fflush(stdout);
		free(web_str);
		uci_free_context(ctx);
		return ;
}

