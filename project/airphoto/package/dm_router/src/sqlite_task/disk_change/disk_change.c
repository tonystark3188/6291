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

/*int set_on_device_list_changed(void *arg)
{
    DiskTaskObj *diskTask = (TaskObj *)arg;
	int enRet = 0;
	int g_unix_socket = -1;
	int client_fd = -1;
	char *recv_buf;
	g_unix_socket = DM_InetServerInit(PF_INET,DISK_MONITER_PORT,SOCK_STREAM,3);
	DMCLOG_D("g_unix_socket = %d",g_unix_socket);
	while(exit_flag == 0)
	{
		client_fd = DM_ServerAcceptClient(g_unix_socket);	
		if(client_fd < 0)
			break;
		DMCLOG_D("client_fd = %d",client_fd);
		enRet = DM_MsgReceive(client_fd, &recv_buf,RECV_TIMEOUT);
		if(enRet != RET_SUCCESS)
        {
            DMCLOG_D("_recv_req_from_client failed!");
            break;
        }
		DMCLOG_D("enRet = %d",enRet);
		enRet = select_change_disk(diskTask);
		if(enRet == 0)
		{
			diskTask->onDMDiskListDel(diskTask);
		}else if(enRet == 1)
		{
			diskTask->onDMDiskListAdd(diskTask);
		}
		safe_close(client_fd);
	}
	DM_DomainServerDeinit(g_unix_socket);
	return 0;
}*/

void set_on_device_list_changed(void *arg)
{
    DiskTaskObj *diskTask = (DiskTaskObj *)arg;
	int enRet = 0;
	while(exit_flag == 0)
	{
		disk_change_flag = 0;
		do{
			sleep(1);
		}while(disk_change_flag == 0);
		enRet = select_change_disk(diskTask);
		DMCLOG_D("enRet = %d",enRet);
		if(enRet == 0)
		{
			diskTask->onDMDiskListDel(diskTask);
		}else if(enRet == 1)
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


