#include "base.h"
#include "task/task_base.h"
#include "msg_server.h"
#include "disk_manage.h"
#include "router_inotify.h"
#include "task/category_task.h"
#include "defs.h"




extern int exit_flag;


int is_drive_on_pc()
{
	int storage_dir;
	int enRet = dm_get_storage_dir(&storage_dir);
	if(enRet == 0)
		return storage_dir;
	else
		return enRet;
	
}

static void release_inotify_func(void *self)
{
    ENTER_FUNC();
	ReleaseDiskTaskObj *releaseDiskTask = (ReleaseDiskTaskObj *)self;
	if(get_fuser_flag() == AIRDISK_ON_PC)
	{
		//pthread_mutex_lock(&releaseDiskTask->diskTask->mutex);
		if(releaseDiskTask->diskTask->onDMDiskListDel != NULL)
		{
			releaseDiskTask->diskTask->onDMDiskListDel(releaseDiskTask->diskTask);
		}	
		else{
			DMCLOG_E("callback is NULL");
		}
		//pthread_mutex_unlock(&releaseDiskTask->diskTask->mutex);
		do{
			usleep(1000);
			DMCLOG_D("wait nactive_fd_cnt =%d",*native_cnt);
		}while(*native_cnt != 0);
	}else{
		if(is_drive_on_pc() == 1)
		{
			//pthread_mutex_lock(&releaseDiskTask->diskTask->mutex);
			if(releaseDiskTask->diskTask->onDMDiskListAdd != NULL)
			{
				releaseDiskTask->diskTask->onDMDiskListAdd(releaseDiskTask->diskTask);
			}else{
				DMCLOG_E("callback is NULL");
			}
			//pthread_mutex_unlock(&releaseDiskTask->diskTask->mutex);
		}
	}

	char *send_buf = comb_release_disk_json(get_fuser_flag()); 
    DM_MsgSend(releaseDiskTask->sock,send_buf,strlen(send_buf));
	safe_free(send_buf);
	safe_close(releaseDiskTask->sock);
	safe_free(self);
    EXIT_FUNC();
    return ;
}

int create_release_task(DiskTaskObj *diskTask,int sock)
{
    ENTER_FUNC();
    PTHREAD_T tid_release_task;
    int rslt = 0;
	ReleaseDiskTaskObj *releaseDiskTask = (ReleaseDiskTaskObj *)calloc(1,sizeof(ReleaseDiskTaskObj));
	releaseDiskTask->sock = sock;
	releaseDiskTask->diskTask = diskTask;
    if (0 != (rslt = PTHREAD_CREATE(&tid_release_task, NULL, (void *)release_inotify_func,releaseDiskTask)))
    {
        DMCLOG_D("release task failed!");
        rslt = -1;
        return rslt;
    }
    PTHREAD_DETACH(tid_release_task);
    EXIT_FUNC();
    return rslt;
}

void set_on_device_list_changed(void *self)
{
	int enRet = 0;
	int client_fd = 0;
	char *recv_buf = NULL;
	int time_out = 2000;
	int release_flag = 0;
	DiskTaskObj *diskTask = (DiskTaskObj *)self;
	if(diskTask == NULL)
		return;
	
	int listen_fd = DM_InetServerInit(PF_INET,13001,SOCK_STREAM,3);
	while(exit_flag == 0)
	{
		client_fd = DM_ServerAcceptClient(listen_fd);
		assert(client_fd > 0);
		enRet = DM_MsgReceive(client_fd, &recv_buf, &time_out);
		if(enRet <= 0)
		{
			DMCLOG_E("receive date error :%d",errno);
			safe_free(recv_buf);
			safe_close(client_fd);
			break;
		}
		int release_flag = parser_scan_notify_json(recv_buf);
		DMCLOG_D("release_flag = %d",release_flag);
		set_fuser_flag(release_flag);
		
		int ret = create_release_task(diskTask,client_fd);
		if(ret < 0)
		{
			DMCLOG_E("create release task error");
		    return ret;
		}
		safe_free(recv_buf);
	}
	safe_close(listen_fd);
	return ;
}


