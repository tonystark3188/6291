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
typedef enum __NOTIFY_ACTION_TYPE
{
	ACTION_FILE_ADDED = 1,// 1 add new file
	ACTION_FILE_REMOVED,// 2 remove file
	ACTION_FILE_MODIFIED,// 3 modified file--ignore?
	ACTION_FILE_OLD_NAME,// 4 rename file--ignore?
	ACTION_FILE_NEW_NAME,// 5 rename file
	ACTION_NOTIFY_COUNT
}NOTIFY_ACTION_TYPE;

int notify_server_release_disk(int release_flag,int *p_status);

int udisk_extract_notify_server_release_disk(int release_flag, char *action_node, int *p_status);

int GetDbWorkStatus(int *p_status);
void notify_server_modified_file(NOTIFY_ACTION_TYPE action, const unsigned char *filepath);
#ifdef __cplusplus
}
#endif

#endif

