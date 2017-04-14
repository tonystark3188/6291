#ifndef _NOTIFY_SERVER_H_
#define _NOTIFY_SERVER_H_

#ifdef __cplusplus
extern "C"{
#endif

enum releaseDiskFlag{
	UNRELEASE_DISK,
	RELEASE_DISK
};

typedef enum{
	DB_STATUS_WAITTING,
	DB_STATUS_SCANNING
}dbStatus;

int notify_server_release_disk(int release_flag,int *p_status);

int udisk_extract_notify_server_release_disk(int release_flag, char *action_node, int *p_status);

int GetDbWorkStatus(int *p_status);

#ifdef __cplusplus
}
#endif

#endif

