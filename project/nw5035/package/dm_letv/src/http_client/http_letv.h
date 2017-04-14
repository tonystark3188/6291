#ifndef _HTTP_LETV_
#define _HTTP_LETV_

#include <netinet/in.h>
#include <sys/socket.h>
#include "my_debug.h"
#include "my_json.h"
#include "msg.h"
#include "defs.h"
#include "http_client.h"


#define TEST_LETV_SERVER_PORT	5540
#define TEST_LETV_SERVER_IP 		"115.182.94.152"

#define LETV_SERVER_PORT	80
//#define LETV_SERVER_PORT	5540
#define LETV_SERVER_IP 		"lebao.leinlife.com"
//#define LETV_SERVER_IP 		"115.182.94.152"
#define LETV_SERVER_DOWNLOAD_PAGE	"/letv_bao/services/contents/download"
#define LETV_SERVER_QUERY_PAGE	"/letv_bao/services/contents/query1"
#define LETV_SERVER_QUERY_ALBUM_INFO	"/letv_bao/services/contents/info"
#define LETV_SERVER_CHECK_FIRMWARE	"/letv_bao/services/contents/version"
#define LETV_SERVER_REPORT_PATH 	"/letv_bao/services/device/notice"
#define LETV_AUTHORIZECODE	"y2c2j4c2e4j4i4z2a3i3"
#define LETV_DEV_TYPE		"zhuijushengqi"
#define LETV_SIGN_TERM				"5"		//1: mobile phone;2:TV;3:spark;4:longsys 5:longsys 1s 6:hack spark


#define STR_AUTHORIZE_CODE 		"authorizeCode"
#define STR_DEV_TYPE	 		"devType"
#define STR_VTYPE				"vtype"
#define STR_VID	 				"vid"
#define STR_PID			 		"pid"
#define STR_EXT	 				"ext"
#define STR_IP	 				"ip"
#define STR_MAC_ID		 		"macId"
#define STR_SIGNATURE	 		"signature"
#define STR_PARAMS				"params"	
#define STR_VERSION_CODE		"versionCode"
#define STR_VERSION_NAME		"versionName"
#define STR_SIGN_TERM			"signTerm"

/* time out value ,ms*/
#define HTTP_GET_INFO_TIME_OUT	10000	
#define HTTP_DOWNLOAD_TIME_OUT	30000//30s
#define HTTP_REPORT_TIME_OUT	10000//20s
/* local file path */
#define FILE_VERSION_INFO_PATH		"/etc/fw_version.conf"
#define FILE_UPGRADE_INFO_PATH		"/etc/fw_upgrade.conf"
#define FILE_UPGRADE_PATH			"/tmp/mnt/USB-disk-1/ota/update.bin"
#define DIR_UPGRADE_PATH			"/tmp/mnt/USB-disk-1/ota"


#define INFO_HAS_NEW			"hasNew"
#define INFO_NEXT_VERSION_CODE	"nextVersionCode"
#define INFO_NEXT_VERSION_NAME	"nextVersionName"
#define INFO_NEXT_FEATURE		"nextFeature"
#define INFO_NEXT_UPDATE_URL	"nextUpdateUrl"
#define INFO_IS_FORCE			"isForce"

char tmp_mac[32];

typedef struct _letv_http_client{
	char authorize_code[64];
	char dev_type[32];
	char vtype[32];
	char vid[VID_LEN];
	char pid[PID_LEN];
	char ext[256];
	char ip[32];
	char mac_id[32];
	char signature[64];
	char params[512];
	char version_code[16];
	char version_name[32];
	char sign_term[8];
}letv_http_client;

typedef struct _fw_version_info{
	char curVersionCode[16];
	char curVersionName[32];
	char nextVersionCode[16];
	char nextVersionName[32];
	char hasNew[16];
	char nextFeature[512];
	char nextUpdateUrl[1024];
	char isForce[16];
	char errorMsg[128];
}fw_version_info;


int letv_client_get_source(http_tcpclient *pclient,int type);

int letv_client_image_download(http_tcpclient *pclient,int type);

int letv_client_video_download(http_tcpclient *pclient,int type);

int letv_client_get_redirect_url(http_tcpclient *pclient);

int letv_client_check_firmware(http_tcpclient *pclient, fw_version_info *p_version_info);

int letv_client_report_status(http_tcpclient *pclient);

int updateSysVal(const char *para,const char *val);

#endif

