#ifndef __DISK_MANAGE_H__
#define __DISK_MANAGE_H__

#if __cplusplus
extern "C" {
#endif
#include "task/task_base.h"
#include "base.h"

enum release_disk_flag{
	UNRELEASE_DISK,
	RELEASE_DISK
};

enum release_disk_event{
	PC_MOUNT_EVENT,
	UDISK_EXTRACT_EVENT
};

#define AIRDISK_ON_PC 1
#define AIRDISK_OFF_PC 0
typedef struct disk_node {
	struct dl_list node;
	struct disk_node *dn_next;
	char uuid[16];
	char path[128];
	char dev_node[32];	
	unsigned long long total_size; 		/* total size 1KB */
	unsigned long long free_size;  		/* free size 1KB */
	char pid[16];					/* pid */
	char vid[16];					/* vid */
	int is_scanning;
	int mount_status;
	char g_database[128];
	TaskObj g_db_query_task;
    TaskObj g_db_write_task;
	sqlite3 *write_connection;
	sqlite3 *query_connection;
    pthread_mutex_t mutex;
}disk_node_t;

typedef void (*DISK_DEL_CALLBACK)(void *self);
typedef void (*DISK_ADD_CALLBACK)(void *self);


typedef struct _DiskTaskObj
{
	struct dl_list head;
	struct disk_node  disk_info;
	DISK_DEL_CALLBACK onDMDiskListDel;
    DISK_ADD_CALLBACK onDMDiskListAdd;
	int release_flag;
	int event;
	int actionNode[16];
	pthread_mutex_t mutex;
}DiskTaskObj;

typedef struct _ReleaseDiskTaskObj
{
	DiskTaskObj *diskTask;
	int sock;
}ReleaseDiskTaskObj;

struct disk_node *get_disk_node(char *uuid);
int update_disk_uuid(char *path,char *uuid);

int get_fuser_flag();
int set_fuser_flag(int flag);




DiskTaskObj *disk_task_init(void (*DISK_ADD_CALLBACK)(void *self),void (*DISK_DEL_CALLBACK)(void *self));



#if __cplusplus
}
#endif

#endif /* __PROCESS_DEV_INFO_H__ */

