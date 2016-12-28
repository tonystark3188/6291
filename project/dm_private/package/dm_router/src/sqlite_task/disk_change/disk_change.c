#include "base.h"
//#include "db_server_prcs.h"
#include "task/task_base.h"
#include "msg_server.h"
#include "disk_manage.h"
#include "router_inotify.h"
#include "dm_encrypt.h"
#include "task/category_task.h"



extern struct disk_node *disk_dn;
extern int exit_flag;
int disk_change_flag;
static int select_change_disk(void *self)
{
	ENTER_FUNC();
	all_disk_t mAll_disk_t;
	int i = 0,j = 0,res = 0;
	int selectedFlag = 0;
	int disk_dn_count = 0;
	struct disk_node *dn = disk_dn;
	DiskTaskObj *disk_task = (DiskTaskObj *)self;
	memset(&mAll_disk_t,0,sizeof(all_disk_t));
	res = dm_get_storage(&mAll_disk_t);
	if(res != SQLITE_OK)
	{
		return -1;
	}
	disk_dn_count = get_disk_dnode_count(dn);
	if(disk_dn_count > mAll_disk_t.count)
	{
		//Del device
		for(i = 0;/*count - detected via !dn below*/;i++)
		{
			if(!dn)
				break;
			DMCLOG_D("i = %d,dn->uuid = %s",i,dn->uuid);
			selectedFlag = 0;
			for(j = 0;j < mAll_disk_t.count;j++)
			{
				DMCLOG_D("uuid = %s",mAll_disk_t.disk[j].uuid);
				if(!strcmp(dn->uuid,mAll_disk_t.disk[j].uuid))
				{
					selectedFlag = 1;
					break;
				}
			}
			if(selectedFlag == 0)
			{
				DMCLOG_D("dn->uuid = %s",dn->uuid);
				strcpy(disk_task->disk_info.uuid,dn->uuid);
				strcpy(disk_task->disk_info.path,dn->path);
				EXIT_FUNC();
				return 0;
			}
			dn = dn->dn_next;
		}
	}else{
		//Add device
		for(j = 0;j < mAll_disk_t.count;j++)
		{
		    
			selectedFlag = 0;
			for(i = 0;/*count - detected via !dn below*/;i++)
			{
				if(!dn)
					break;
				if(!strcmp(dn->uuid,mAll_disk_t.disk[j].uuid))
				{
					selectedFlag = 1;
					continue;
				}
				dn = dn->dn_next;
			}
			if(selectedFlag == 0)
			{
				res = get_disk_mark_info(mAll_disk_t.disk[j].path);
				if(res == 0)
				{
					//有标记文件
					DMCLOG_D("uuid file is exist");
					read_mark_file(mAll_disk_t.disk[j].path,mAll_disk_t.disk[j].uuid);
				}else{
					//没有标记文件
					DMCLOG_D("uuid file is not exist");
					_dm_gen_uuid(mAll_disk_t.disk[j].uuid,mAll_disk_t.disk[j].pid,mAll_disk_t.disk[j].vid, mAll_disk_t.disk[j].total_size, mAll_disk_t.disk[j].free_size);
					DMCLOG_D("uuid = %s",mAll_disk_t.disk[j].uuid);	
					create_mark_file(mAll_disk_t.disk[j].path,mAll_disk_t.disk[j].uuid);//在磁盘创建uuid文件，标记磁盘的唯一性
				}
				strcpy(disk_task->disk_info.uuid,mAll_disk_t.disk[j].uuid);
				strcpy(disk_task->disk_info.path,mAll_disk_t.disk[j].path);
				DMCLOG_D("path = %s",disk_task->disk_info.path);
				return 1;
			}
		}
	}
	EXIT_FUNC();
	return -1;
}
/*return 0:drive on pc
		 1:drive down pc
		 -1:error*/
int is_drive_on_pc()
{
	int storage_dir;
	int enRet = dm_get_storage_dir(&storage_dir);
	if(enRet == 0)
		return storage_dir;
	else
		return enRet;
	
}

void set_on_device_list_changed(void *arg)
{
    DiskTaskObj *diskTask = (DiskTaskObj *)arg;
	int enRet = 0;
	if(diskTask == NULL)
		return;
	
	while(exit_flag == 0)
	{
		disk_change_flag = 0;
		do{
			sleep(1);
		}while(disk_change_flag == 0);
		//enRet = select_change_disk(diskTask);
		enRet = is_drive_on_pc();
		DMCLOG_D("enRet = %d",enRet);
		if(enRet == 0)//indicate drive on pc
		{
			if(diskTask->onDMDiskListDel != NULL)
				diskTask->onDMDiskListDel(diskTask);
			else{
				DMCLOG_D("onDMDiskListDel NULL");
				return;
			}
		}else if(enRet == 1)//indicate drive on board
		{
			if(diskTask->onDMDiskListAdd != NULL)
			{
				diskTask->onDMDiskListAdd(diskTask);
			}else{
				DMCLOG_D("onDMDiskListAdd NULL");
				return;
			}
		}
	}
	return ;
}



