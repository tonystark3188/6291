/*
 * =============================================================================
 *
 *       Filename:  hd_route.c
 *
 *    Description:  set and get wifi settings,set and get network settings
 *
 *        Version:  1.0
 *        Created:  2015/04/03 14:33
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver, 
 *   Organization:  
 *
 * =============================================================================*/
#include <sys/time.h>
#include "hd_route.h"
#include "uci_api.h"
#include "router_defs.h"
#include "config.h"
#include "mcu.h"
#include "api_process.h"


extern int exit_flag;
extern int safe_exit_flag;


int check_fw_file()
{
	struct dirent* ent = NULL;
	DIR *pDir;
	char dir[128];
	char fw_path[64];
	char cmd[128];
	struct stat statbuf;
	int hasFW=FALSE;
	FILE *fw_fp;
	
	if( (pDir=opendir(MOUNT_PATH))==NULL )
	{
		fprintf( stderr, "Cannot open directory:%s\n", MOUNT_PATH );
		return FALSE;
	}
	while( (ent=readdir(pDir))!=NULL )
	{
		memset(dir,0,sizeof(dir));
		snprintf( dir, 128 ,"%s/%s", MOUNT_PATH, ent->d_name );
		DMCLOG_D("%s\n",dir);
		lstat(dir, &statbuf);
		if( S_ISDIR(statbuf.st_mode) )  //is a dir
		{
			if(strcmp( ".",ent->d_name) == 0 || strcmp( "..",ent->d_name) == 0) 
				continue;
			memset(fw_path,0,sizeof(fw_path));
			sprintf(fw_path,"%s/$$update$$.bin",dir);
			if( (fw_fp=fopen(fw_path,"rb")) != NULL )
			{
				hasFW=TRUE;
				memset(cmd,0,sizeof(cmd));
				sprintf(cmd,"dd if=%s/'$$update$$.bin' of=/tmp/fwsysupgrade bs=1k conv=notrunc count=7616",dir);
				system(cmd);
				memset(cmd,0,sizeof(cmd));
				sprintf(cmd,"dd if=%s/'$$update$$.bin' of=/tmp/websysupgrade skip=7616 bs=1k conv=notrunc count=256",dir);
				system(cmd);
				memset(cmd,0,sizeof(cmd));
				sprintf(cmd,"ls %s/FactoryDelete.tag || mv %s/'$$update$$.bin' %s/'$$update$$.bin.bak'",dir,dir,dir);
				system(cmd);
				memset(cmd,0,sizeof(cmd));
				sprintf(cmd,"ls %s/FactoryDelete.tag && touch /tmp/dont_reboot",dir);
				system(cmd);
				system("sync");
				fclose(fw_fp);
				break;
			}
		}
	}
	
	closedir(pDir);
	return hasFW;
}


/*******************************************************************************
 * Function:
 * int dm_get_power(power_info_t *m_power_info);
 * Description:
 * get power info 0x0208
 * Parameters:
 *    m_power_info [OUT] power info
 * Returns:
 *    ROUTER_OK:success,ROUTER_ERRORS_MCU_IOCTL:failed
 *******************************************************************************/
int dm_get_power(power_info_t *m_power_info)
{
	if(0 == dm_mcu_get_power(m_power_info))
		return ROUTER_OK;
	else
		return ROUTER_ERRORS_MCU_IOCTL;
}

#if 0
/*******************************************************************************
 * Function:
 * int dm_get_power(power_info_t *m_power_info);
 * Description:
 * get power info 0x0208
 * Parameters:
 *    m_power_info [OUT] power info
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_get_power(power_info_t *m_power_info)
{
	DMCLOG_D("access dm_get_power");
	unsigned char be = 0;//power percent
	unsigned char stat = 0;// 0:正常，1:充电，2:放电，3:低电
	unsigned char tmpStatus = 0;
	int fh = NULL;
	int count = 0;
	fh=open(rtl_encryp_control, O_RDWR);
	if(fh==NULL)
	{
		DMCLOG_D("open MCU proc error");
		return ROUTER_ERRORS_MCU_IOCTL; 
	}
	usleep(100000);
	//get power percent
	while(1)
	{
		usleep(100000);
		if(ioctl(fh, get_power_level_num, &be) < 0)
		{
			//DMCLOG_D("get power percent from MCU error");
			close(fh);
			return ROUTER_ERRORS_MCU_IOCTL; 							
		}
		if(be == 0 || be == 255)
			continue;
		else
			break;
		count++;
		if(count == 5)
			return;
	}
	usleep(100000);
	//get power status
	if(ioctl(fh,get_Firmware_Edition,&tmpStatus)<0)
	{
		//DMCLOG_D("get power status from MUC error");
		close(fh);
		return ROUTER_ERRORS_MCU_IOCTL;
	}
	stat = (tmpStatus&0x0f);
	if(stat==3)
	{
		stat = 1;
	}
	if(be<=10)
	{
		stat = 3;
	}

	m_power_info->power = be;
	m_power_info->power_status = stat;
	
	return ROUTER_OK;
}
#endif

/*******************************************************************************
 * Function:
 * int dm_udisk_upgrade();
 * Description:
 * upgrade from U drive 0x021B
 * Parameters:
 *    NULL
 * Returns:
 *    0:设备根目录无升级文件,
 *    1:升级文件CRC校验失败
 *    2:升级文件正常，开始升级
 *    -1:failed
 *******************************************************************************/
int dm_udisk_upgrade()
{
	DMCLOG_D("access dm_udisk_upgrade");
	FILE *fw_fp=NULL;
	FILE *fp_dev=NULL;
	FILE *web_dev=NULL;
	unsigned char * fw_ptr;
	int f_size=0;
	int hasfw=0;
	int i;
	char uci_option_str[64]="\0";
	char str_sp[64]="\0";
	char updatefw[8]="\0";
	char op_fw_header[32]="\0";
	int err = 0;
	//ctx=uci_alloc_context();
	//sleep(20);
	//memset(uci_option_str,0,sizeof(uci_option_str));
	memset(str_sp,0,sizeof(str_sp));
	memset(updatefw,0,sizeof(updatefw));
	memset(op_fw_header,0,sizeof(op_fw_header));
	
	for(i = 0; i < 1; i++)
	{
		hasfw = check_fw_file();
		if(hasfw == TRUE)
			break;
		//sleep(1);
	}

	if(0==hasfw)
	{
		DMCLOG_D("no fw file or open fw file failed");
		return -1;
	}
	
	system("echo timer > /sys/class/leds/longsys\:wifi\:led/trigger ");
	err = system("sysupgrade /tmp/fwsysupgrade");
	if(err)
	{
		system("echo netdev > /sys/class/leds/longsys\:wifi\:led/trigger");
		system("echo ra0 > /sys/class/leds/longsys\:wifi\:led/device_name");
		system("echo link tx rx > /sys/class/leds/longsys\:wifi\:led/mode");
		DMCLOG_D("sysupgrade failed");
		return -1;
	}
	return 0;
}

/*******************************************************************************
 * Function:
 * int dm_sync_time();
 * Description:
 * sync time 0x021D
 * Parameters:
 *    NULL
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_sync_time(dm_time_info *time_info_t)
{
	DMCLOG_D("access dm_sync_time");
	char value[64] = "\0";
	char zone[64] = "\0";
	char *zone_tmp = NULL;
	char zone_str[32] = "\0";
	char zone_cur[32] = "\0";
	char str_sp[64] = "\0";
	struct tm time_tm;
    struct timeval time_tv;
    time_t timep;
	struct timezone time_tz;
	int hour;
	int minutes;
	int rec;
	char uci_option_str[64]="\0";
#ifdef SUPPORT_OPENWRT_PLATFORM
	//DMCLOG_D("time_info_t->time_value = %s,time_info_t->time_zone = %s",time_info_t->time_value,time_info_t->time_zone);
	memset(zone_str, 0, sizeof(zone_str));
	memset(zone_cur, 0, sizeof(zone_cur));
	
	memset(value, 0, sizeof(value));
	strcpy(value, time_info_t->time_value);
	memset(zone, 0, sizeof(zone));
	strcpy(zone, time_info_t->time_zone);
	
	memset(uci_option_str, '\0', 64);
	strcpy(uci_option_str,"system.@system[0].timezone");  //status
	uci_get_option_value(uci_option_str,zone_cur);
	
	sscanf(value, "%d-%d-%d %d:%d:%d", &time_tm.tm_year, &time_tm.tm_mon, &time_tm.tm_mday, &time_tm.tm_hour, &time_tm.tm_min, &time_tm.tm_sec);
	time_tm.tm_year -= 1900;
	time_tm.tm_mon -= 1;
	time_tm.tm_wday = 0;
	time_tm.tm_yday = 0;
	time_tm.tm_isdst = 0;
		
	zone_tmp=zone;
	memcpy(zone_str,zone_tmp,3);
	zone_tmp+=3;
		
	if(*zone_tmp == '-')
	{
		zone_tmp++;
		strcat(zone_str,zone_tmp);
		sscanf(zone_tmp,"%d:%d",&hour,&minutes);
		time_tz.tz_minuteswest=hour*60+minutes;		
	}
	else
	{
		zone_str[3]='-';
		zone_tmp++;
		strcat(zone_str,zone_tmp);
		sscanf(zone_tmp,"%d:%d",&hour,&minutes);
		time_tz.tz_minuteswest=hour*60+minutes;
		time_tz.tz_minuteswest=-time_tz.tz_minuteswest;
	}	

	if(strcmp(zone_cur,zone_str)!=0)
	{
		DMCLOG_D("set time zone");
		memset(str_sp, '\0', 64);
		sprintf(str_sp,"system.@system[0].timezone=%s",zone_str);
		if(-1 == uci_set_option_value(str_sp))
		{
			return -1;
		}
		memset(str_sp, '\0', 64);
		sprintf(str_sp,"echo \"%s\">/tmp/TZ",zone_str);
		system(str_sp);
	}

	timep = mktime(&time_tm);
	time_tv.tv_sec = timep;
	time_tv.tv_usec = 0;
	time_tz.tz_dsttime=0;
		
	rec = settimeofday(&time_tv, &time_tz);
	if(rec < 0)
	{
		DMCLOG_D("set time failed");
		return -1;
	}
	
#endif
#ifdef SUPPORT_LINUX_PLATFORM
#endif
	return 0;
}

/*******************************************************************************
 * Function:
 * int dm_sync_system();
 * Description:
 * sync time 0x021D
 * Parameters:
 *    NULL
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_sync_system(system_sync_info_t *p_system_sync_info)
{
	DMCLOG_D("access dm_sync_system");
	if(p_system_sync_info->sync){
		sync();
		usleep(50000);
	}
	
	if(p_system_sync_info->clean_cache){ //clean cache
		system("echo 3 > /proc/sys/vm/drop_caches");
		sync();
		usleep(50000);
	}
	return 0;
}

void del_n(char *str)
{
	int i=0;
	while(str[i])
	{
		if('\n' == str[i])
		{
			str[i]=0;
			if(i>0 && str[i-1]=='\r')
				str[i-1]=0;
		}
		i++;
	}
}

int get_cfg_str(char *param,char *ret_str)
{
	char get_str[128]={0};
	char tmp[128]={0};
	FILE *fp;
	sprintf(get_str,"cfg get \'%s\'",param);
	fp=popen(get_str,"r");
	fgets(tmp,128,fp);
	pclose(fp);
	del_n(tmp);
	if(strlen(tmp)<=1)
		return 0;
	else
	{
		strcpy(ret_str,tmp+strlen(param)+1);
		return 1;
	}
}
#if 0
int set_cfg_str(char *param,char *ret_str)
{
	char set_str[128]={0};
	sprintf(set_str,"cfg set %s=%s",param,ret_str);
	system(set_str);
}
#endif
int set_nor_str(char *param,char *ret_str)
{
	char set_str[128]={0};
	sprintf(set_str,"nor set %s=%s",param,ret_str);
	system(set_str);
	return 0;
}

char *get_conf_str(char *var)
{
	FILE *fp=fopen("/etc/nrender.conf","r");
	if(NULL == fp)
	{
		//printf("open /etc/config/nrender.conf failed \n");
		return 0;
	}
	char tmp[128];
	char *ret_str;
	bzero(tmp,128);
	while(fgets(tmp,128,fp)!=NULL)
	{
		if('\n'==tmp[strlen(tmp)-1])
		{
			tmp[strlen(tmp)-1]=0;
		}
		//printf("get string from /etc/config/nrender.conf:%s\n",tmp);
		if(!strncmp(var,tmp,strlen(var)))
		{
			ret_str = malloc(strlen(tmp)-strlen(var));
			if(!ret_str)
			{
				fclose(fp);
				return 0;
			}
			bzero(ret_str,strlen(tmp)-strlen(var));
			strcpy(ret_str,tmp+strlen(var)+1);
			//printf("ret string:%s\n",ret_str);
			fclose(fp);
			return ret_str;
		}
		
	}
	fclose(fp);
	return 0;
}


/*******************************************************************************
 * Function:
 * int dm_get_ota_info();
 * Description:
 * get the status of whether or net connect internet
 * Parameters:
 *    NULL
 * Returns:
 *    0:not connected,1:connected,-1:error
 *******************************************************************************/
int dm_get_ota_info(dm_ota_info *p_ota_info)
{
	int ret =0; 
	FILE *read_fp = NULL;
	char tmp_mac[32]="\0";
	char version_flag[32] = "\0";
	char fw_version[32] = "\0";
	char *version_num = NULL;
	char *p = NULL;

	#if defined(SUPPORT_OPENWRT_PLATFORM)
		#if defined(OPENWRT_X1000)
			if( (read_fp=fopen("/etc/mac.txt", "rb")) != NULL)
			{
				fread(tmp_mac,1,17,read_fp);
				strncpy(p_ota_info->mac,tmp_mac,17);
				fclose(read_fp);
			}
		#else
			read_fp = popen("hexdump -s 4 -n 6 -C /dev/mtd3 | head -n 1 | sed 's/\ \ /:/g' | cut -d: -f 2 | sed 's/\ /:/g' | tr \"[a-z]\" \"[A-Z]\"", "r");
			if(read_fp != NULL)
			{
				memset(tmp_mac, 0, sizeof(tmp_mac));
				fgets(tmp_mac, 32-1, read_fp);
			}
			strncpy(p_ota_info->mac,tmp_mac,17);
			pclose(read_fp);
		#endif
	#endif
	

	memset(fw_version, 0, sizeof(fw_version));
	ret = get_cfg_str("fw_version",fw_version);
	if(ret == 0)
	{
		//return -1;
		strcpy(p_ota_info->customCode, "undefined");
	}
	else
	{
		strcpy(p_ota_info->customCode, fw_version);
	}

	version_num = get_sys_fw_version();

	memset(version_flag, 0, sizeof(version_flag));
	ret = get_cfg_str("version_flag",version_flag);	
	if((ret == 1) && (strcmp(version_flag,"0") == 0) \
		&& (version_num != NULL) && (strlen(version_num) > 1))
	{
		DMCLOG_D("release version");
		p_ota_info->version_flag = 0;
		p = strrchr(version_num, '.');
		if(NULL != p){
			memcpy(p_ota_info->versionCode, version_num, p-version_num);
		}
		else{
			strcpy(p_ota_info->versionCode, version_num);
		}
	}
	else if((version_num != NULL) && (strlen(version_num) > 1))
	{
		DMCLOG_D("test version");
		p_ota_info->version_flag = 1;
		strcpy(p_ota_info->versionCode, version_num);
	}

	#if defined(OPENWRT_X1000)
	p_ota_info->time = 30;
	#else
	p_ota_info->time = 120;
	#endif
	return 0;
}


int dm_get_version_flag(int *version_flag)
{
	int ret;
	char version_flag_tmp[32] = "\0";
	memset(version_flag_tmp, 0, 32);
	ret = get_cfg_str("version_flag",version_flag_tmp);	
	if((ret == 0) || (strcmp(version_flag_tmp,"1") == 0))
	{
		*version_flag = 1;
	}
	else
	{
		*version_flag = 0;
	}
	return 0;
}

int dm_set_version_flag(int version_flag)
{
	if(version_flag)
		system("cfg set version_flag=1");
	else
		system("cfg set version_flag=0");
	return 0;
}

int dm_get_fw_version(char *fw_version)
{
	int ret = 0;
	char project_version_tmp[32] = "\0";
	char version_flag_tmp[32] = "\0";
	char *version_num = NULL;
	char version_num_tmp[32] = "\0";
	char *p = NULL;
	if(fw_version == NULL)
	{
		return -1;
	}

	//get project number
	char *version=get_conf_str("fw_version");
	if(!version)
	{
		strcpy(project_version_tmp, "undefined");
	}
	else
	{
		strncpy(project_version_tmp, version,31);
		free(version);
	}
	#if 0
	memset(project_version_tmp, 0, sizeof(project_version_tmp));
	ret = get_cfg_str("fw_version",project_version_tmp);
	if(ret == 0)
	{
		//return -1;
		strcpy(project_version_tmp, "undefined");
	}
	#endif

	//get version number
	version_num = get_sys_fw_version();
	DMCLOG_D("version_num = %s", version_num);

	//get vsesion flag 1:  test version; 0: release version
	memset(version_flag_tmp, 0, sizeof(version_flag_tmp));
	memset(version_num_tmp, 0, sizeof(version_num_tmp));
	ret = get_cfg_str("version_flag",version_flag_tmp);	
	if((ret == 1) && (strcmp(version_flag_tmp,"0") == 0) \
		&& (version_num != NULL) && (strlen(version_num) > 1))
	{
		//sprintf(fw_version, "%s-%s", FW_2, project_version_tmp);
		DMCLOG_D("release version");
		p = strrchr(version_num, '.');
		if(NULL != p){
			memcpy(version_num_tmp, version_num, p-version_num);
		}
		else{
			strcpy(version_num_tmp, version_num);
		}
	}
	else if((version_num != NULL) && (strlen(version_num) > 1))
	{
		//sprintf(fw_version, "%s-%s", FW_1, project_version_tmp);
		DMCLOG_D("test version");
		strcpy(version_num_tmp, version_num);
	}

	sprintf(fw_version, "%s-%s", version_num_tmp, project_version_tmp);

	return 0;
}


int dm_upgrade_fw()
{
	FILE *fw_fp=NULL;
	char cmd[128];

	#if defined(SUPPORT_OPENWRT_PLATFORM)
		#if defined(OPENWRT_MT7628)
			if((fw_fp = fopen(FW_FILE,"rb")) == NULL)    //read,binary
			{
				return -1;
			}
			fclose(fw_fp);
			system("sync");
			system("echo 3 >/proc/sys/vm/drop_caches");
			sleep(1);
			//system("block umount");
			//usleep(200000);
			//system("stm8_control 6");  //logout disk
			system("sync");
			system("echo timer > /sys/class/leds/longsys\:wifi\:led/trigger ");
			memset(cmd, 0 ,sizeof(cmd));
			sprintf(cmd, "dd if=%s of=/tmp/websysupgrade skip=11712 bs=1k conv=notrunc count=256", FW_FILE);
			system(cmd);
			memset(cmd, 0 ,sizeof(cmd));
			sprintf(cmd, "sysupgrade %s", FW_FILE);
			system(cmd);
		#elif defined(OPENWRT_X1000)
			if((fw_fp = fopen("/tmp/fwupgrade","rb")) == NULL)    //read,binary
			{
				return -1;
			}
			fclose(fw_fp);
			
			//system("/etc/init.d/dm_router stop");

			system("sysupgrade /tmp/fwupgrade &");
		#endif
	#endif
	
	return 0;
}


int dm_get_safe_exit(int *p_safe_exit)
{
	*p_safe_exit = exit_flag;
	return 0;
}


int dm_set_safe_exit(int safe_exit)
{
	DMCLOG_D("access dm_set_safe_exit");
	if(safe_exit == 1){
		DMCLOG_D("exit exit");
		exit_flag = 1;
		safe_exit_flag = 2;
		DMCLOG_D("safe_exit_flag = %d",safe_exit_flag);
	}
	else{
		//safe_exit_flag = 0;
	}
	return 0;
}

int dm_get_device_id(char *device_id)
{
	int ret = 0;
	if(device_id == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	ret = get_cfg_str("ssid",device_id);
	if(ret == 0)
	{
		DMCLOG_E("get device id error");
		return -1;
	}
	return 0;
}


#define ROOT_PAWWORD_PATH "/etc/root_password.txt"


static int create_password_cache(char *path)
{
	if(path == NULL)
	{
		DMCLOG_E("para is null");
		return 0;
	}
	FILE* fp = NULL;
	if( (fp = fopen(path, "w+")) == NULL)
	{
		DMCLOG_E("open error :%d",errno);
		return -1;
	}
	fclose(fp);
	return 0;
}

static int _dm_get_cfg_pwd(char *pwd)
{
	if(pwd == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	int ret = get_cfg_str("root_pwd",pwd);
	if(ret == 0)
	{
		DMCLOG_E("get root password error");
		return -1;
	}
	DMCLOG_D("pwd = %s",pwd);
	return 0;
}
#if 0
static int _dm_set_cfg_pwd(char *pwd)
{
	if(pwd == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	int ret = set_cfg_str("root_pwd",pwd);
	if(ret == 0)
	{
		DMCLOG_E("set root password error");
		return -1;
	}
	return 0;
}
#endif

static int _dm_set_nor_pwd(char *pwd)
{
	if(pwd == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	int ret = set_nor_str("root_pwd",pwd);
	if(ret != 0)
	{
		DMCLOG_E("set root password error");
		return -1;
	}
	return 0;
}

static int _dm_set_nor_disk_st(int disk_st)
{
	char disk_st_str[2];
	memset(disk_st_str, 0, sizeof(disk_st_str));
	sprintf(disk_st_str, "%d", disk_st);
	int ret = set_nor_str("disk_st",disk_st_str);
	if(ret != 0)
	{
		DMCLOG_E("set disk_st error");
		return -1;
	}
	return 0;
}

int dm_set_root_pwd(char* password)
{
	if(password == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	create_password_cache(ROOT_PAWWORD_PATH);
	JObj* root_json = json_object_new_object();
	JSON_ADD_OBJECT(root_json, "password", JSON_NEW_OBJECT(password,string));
	json_object_to_file(ROOT_PAWWORD_PATH, root_json);
	json_object_put(root_json);
	//_dm_set_cfg_pwd(password);
	_dm_set_nor_pwd(password);
	return 0;
}



int dm_get_root_pwd(char *pwd)
{
	JObj* root_json = NULL;
	JObj* password_json = NULL;
	root_json = json_object_from_file(ROOT_PAWWORD_PATH);
	if(root_json == NULL)
	{
		char cfg_pwd[32] = {0};
		if(_dm_get_cfg_pwd(cfg_pwd) < 0)
		{
			DMCLOG_E("get root password error");
			return -1;
		}
		DMCLOG_D("cfg_pwd = %s",cfg_pwd);
		dm_set_root_pwd(cfg_pwd);//如果cfg存在默认密码，则将此密码保存到指定文件
		strcpy(pwd,cfg_pwd);
		return 0;
	}
	
	password_json = JSON_GET_OBJECT(root_json,"password");
	if(password_json != NULL)
	{
		strcpy(pwd, JSON_GET_OBJECT_VALUE(password_json,string));
	}
	
	return 0;
}

/*
* function:
*  the password is or not exist
* para: 
* return:
*  0:succ
*  -1:failed
*/
bool dm_root_pwd_exist()
{
	if(access(ROOT_PAWWORD_PATH,F_OK) != 0)/*  如果不存在则判断cfg是否存在默认密码*/
	{
		char pwd[32] = {0};
		if(_dm_get_cfg_pwd(pwd) < 0)
		{
			DMCLOG_E("get root password error");
			return FALSE;
		}
		DMCLOG_D("pwd = %s",pwd);
		dm_set_root_pwd(pwd);//如果cfg存在默认密码，则将此密码保存到指定文件
		return TRUE;
	}
	return TRUE;
}

#define DISK_ST_PATH "/etc/disk_st.txt"
int dm_get_file_storage(int *p_g_file_storage_flag){
	DMCLOG_D("access dm_get_file_storage");
	int ret = 0;
	JObj* disk_st_json = NULL;
	JObj* pc_disable_json = NULL;
	disk_st_json = json_object_from_file(DISK_ST_PATH);
	if(disk_st_json == NULL){
		//ret = ROUTER_ERROR_FILE_NOT_EXIST;
		//goto EXIT;
		*p_g_file_storage_flag = G_FILE_STORAGE_SET;
	}
	
	pc_disable_json = JSON_GET_OBJECT(disk_st_json,"pc_disable");
	if(pc_disable_json != NULL){
		*p_g_file_storage_flag = JSON_GET_OBJECT_VALUE(pc_disable_json,int);
	}
	
	return ROUTER_OK;		
EXIT:	
	return ret;	
}

int dm_set_disk_direction(int disk_st){
	DMCLOG_D("accsee dm_set_disk_direction");

	#if 0
	if(disk_st == G_FILE_STORAGE_SET){
		system("echo /dev/mmcblk0p4 >/sys/devices/platform/jz-dwc2/dwc2/gadget/gadget-lun0/file");
	}
	else if(disk_st == G_FILE_STORAGE_CLEAR){
		system("echo 0 >/sys/devices/platform/jz-dwc2/dwc2/gadget/gadget-lun0/file");
	}
	else{
		return ROUTER_ERRORS_CMD_DATA;
	}
	#endif
	
	JObj* disk_st_json = json_object_new_object();
	JSON_ADD_OBJECT(disk_st_json, "pc_disable", JSON_NEW_OBJECT(disk_st, int));
	json_object_to_file(DISK_ST_PATH, disk_st_json);
	json_object_put(disk_st_json);

	_dm_set_nor_disk_st(disk_st);
	return ROUTER_OK;
}

