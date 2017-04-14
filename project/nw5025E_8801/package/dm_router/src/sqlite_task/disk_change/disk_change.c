#include "base.h"
#include "task/task_base.h"
#include "msg_server.h"
#include "disk_manage.h"
#include "db/disk_change.h"
#include "router_inotify.h"
#include "defs.h"

extern int exit_flag;
extern int safe_exit_flag;
extern int listen_fd;

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
	//if(get_fuser_flag() == AIRDISK_ON_PC)
	if(releaseDiskTask->diskTask->release_flag == RELEASE_DISK)
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
		}while(*native_cnt != 0&&*native_cnt < 1024);
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
	if(send_buf != NULL){
	    DM_MsgSend(releaseDiskTask->sock,send_buf,strlen(send_buf));
		safe_free(send_buf);
	}
	safe_close(releaseDiskTask->sock);
	safe_free(self);
    EXIT_FUNC();
    return ;
}

int create_release_task(DiskTaskObj *diskTask)
{
    ENTER_FUNC();
    PTHREAD_T tid_release_task;
    int rslt = 0;
	ReleaseDiskTaskObj *releaseDiskTask = (ReleaseDiskTaskObj *)calloc(1,sizeof(ReleaseDiskTaskObj));
	releaseDiskTask->sock = diskTask->clientFd;
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


int GetDbStatusTask(DiskTaskObj *diskTask)
{
	int dbStatus = 0;

	dbStatus = GetDmFileTableScanStatus();

	//DMCLOG_D("dbStatus: %d", dbStatus);
	char *send_buf = CombGetDbStatusJson(dbStatus); 
	if(send_buf == NULL){
		DMCLOG_E("comb json fail");
		return -1;
	}

	DM_MsgSend(diskTask->clientFd, send_buf, strlen(send_buf));
	safe_free(send_buf);
	safe_close(diskTask->clientFd);
    EXIT_FUNC();
    return 0;	
}

int HandleScanNotifyCmd(DiskTaskObj *diskTask)
{
	int ret = 0;
	if(diskTask->cmd == FN_RELEASE_DISK){
		if(diskTask->event == EVENT_PC_MOUNT)
			set_fuser_flag(diskTask->release_flag);
		
		ret = create_release_task(diskTask);
		if(ret < 0){
			DMCLOG_E("create release task error");
		    return ret;
		}
	}
	else if(diskTask->cmd == FN_GET_DB_STA){
		ret = GetDbStatusTask(diskTask);
		if(ret < 0){
			DMCLOG_E("get db status task error");
		    return ret;
		}
	}
	else{
		DMCLOG_E("unknow cmd: %d", diskTask->cmd);
		return -1;
	}
	return 0;
}

void set_on_device_list_changed(void *self)
{
	int enRet = 0;
	int client_fd = 0;
	char *recv_buf = NULL;
	int time_out = 2000;
	//int release_flag = 0;
	DiskTaskObj *diskTask = (DiskTaskObj *)self;
	if(diskTask == NULL)
		return;
	
	listen_fd = DM_InetServerInit(PF_INET,13001,SOCK_STREAM,3);
	if(listen_fd < 0)
	{
		DMCLOG_D("inet server init error");
		return ;
	}
	DMCLOG_D("inet server init succ, listen_fd = %d",listen_fd);
	while(exit_flag == 0)
	{
		client_fd = DM_ServerAcceptClient(listen_fd);
		//assert(client_fd > 0);
		if(client_fd < 0)
		{
			safe_close(listen_fd);
			exit_flag = 1;
			pid_t pid;
			char s[64 + 1];
			
			pid = getppid();
			DMCLOG_D("pid = %d",pid);
			sprintf(s, "kill -9 %d", pid);
			system(s);	
			DMCLOG_D("exit_flag = %d",exit_flag);
			exit(0);
		}

		diskTask->clientFd = client_fd;
		enRet = DM_MsgReceive(client_fd, &recv_buf, &time_out);
		if(enRet <= 0)
		{
			DMCLOG_E("receive date error :%d",errno);
			safe_free(recv_buf);
			safe_close(client_fd);
			continue;//break;
		}
		
		enRet = ParserScanNotifyJson(recv_buf, diskTask);
		if(enRet < 0){
			DMCLOG_E("parser date error :%d",errno);
			safe_free(recv_buf);
			safe_close(client_fd);
			continue;//break;
		}

		enRet = HandleScanNotifyCmd(diskTask);
		if(enRet < 0){
			DMCLOG_E("handle cmd error :%d",errno);
			safe_free(recv_buf);
			safe_close(client_fd);
			continue;//break;
		}
		
		safe_free(recv_buf);
	}
	safe_close(listen_fd);
	return ;
}


