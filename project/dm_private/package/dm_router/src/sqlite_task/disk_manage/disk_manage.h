#ifndef __DISK_MANAGE_H__
#define __DISK_MANAGE_H__

#if __cplusplus
extern "C" {
#endif
#include "task/task_base.h"
#include "base.h"




typedef struct disk_node {
	struct dl_list node;
	struct disk_node *dn_next;
	char uuid[16];
	char path[128];
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
}DiskTaskObj;


/*
*	add disk info to static list 
*     return < 0:error,0:sucess
*/
int add_disk_to_list(struct disk_node **dn,struct disk_node *disk_info);
/*
*	delete disk info from static list 
*     return < 0:error,0:sucess
*/
int del_disk_from_list_for_uuid(struct disk_node **head,char *uuid);
int destory_disk_list(struct disk_node *dn);
void display_disk_dnode(struct disk_node *dn);
int get_disk_dnode_count(struct disk_node *dn);
struct disk_node *get_disk_node(char *uuid);



#if __cplusplus
}
#endif

#endif /* __PROCESS_DEV_INFO_H__ */

