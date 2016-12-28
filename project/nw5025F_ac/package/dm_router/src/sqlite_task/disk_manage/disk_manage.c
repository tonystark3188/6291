#include "disk_manage.h"
#include "util.h"
#include "msg.h"

DiskTaskObj diskListenTask;
int fuser_flag = 0;

int get_fuser_flag()
{
	return fuser_flag;
}
int set_fuser_flag(int flag)
{
	if(flag != 0 && flag != 1)
		return -1;
	fuser_flag = flag;
	return 0;
}
struct disk_node *get_disk_node(char *uuid)
{
	if(!*uuid)
	{
		DMCLOG_E("uuid is null");
		return NULL;
	}
	disk_node_t *disk_node,*n;
	DiskTaskObj *diskTask = &diskListenTask;
	pthread_mutex_lock(&diskTask->mutex);
	if(!*uuid||get_fuser_flag() == AIRDISK_ON_PC||&diskTask->head == NULL || dl_list_empty(&diskTask->head))
	{
		DMCLOG_D("get_fuser_flag() = %d",get_fuser_flag());
		pthread_mutex_unlock(&diskTask->mutex);
		return NULL;
	}
	dl_list_for_each(disk_node,&diskTask->head,disk_node_t,node)
	{
		if(!strcmp(disk_node->uuid,uuid))
		{
			DMCLOG_D("disk_node->uuid = %s,uuid = %s",disk_node->uuid,uuid);
			pthread_mutex_unlock(&diskTask->mutex);
			return disk_node;
		}
	}
	pthread_mutex_unlock(&diskTask->mutex);
	return NULL;
}

int update_disk_uuid(char *path,char *uuid)
{
	disk_node_t *disk_node;
	DiskTaskObj *diskTask = &diskListenTask;
	pthread_mutex_lock(&diskTask->mutex);
	if(&diskTask->head == NULL || dl_list_empty(&diskTask->head))
	{
		pthread_mutex_unlock(&diskTask->mutex);
		return NULL;
	}
	pthread_mutex_lock(&diskTask->mutex);
	dl_list_for_each(disk_node,&diskTask->head,disk_node_t,node)
	{
		if(!strcmp(disk_node->path,path))
		{
			strcpy(uuid,disk_node->uuid);
		}
	}
	pthread_mutex_unlock(&diskTask->mutex);
	return 0;
}

DiskTaskObj *disk_task_init(void (*DISK_ADD_CALLBACK)(void *self),void (*DISK_DEL_CALLBACK)(void *self))
{
	DiskTaskObj *diskTask = &diskListenTask;
	dl_list_init(&diskTask->head);
	pthread_mutex_init(&diskTask->mutex,NULL);
	diskTask->onDMDiskListAdd = DISK_ADD_CALLBACK;
	diskTask->onDMDiskListDel = DISK_DEL_CALLBACK;
	return diskTask;
}

