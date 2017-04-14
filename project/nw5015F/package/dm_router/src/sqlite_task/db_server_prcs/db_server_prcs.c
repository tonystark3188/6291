#include "db_server_prcs.h"
#include "disk_manage.h"
#include "db/disk_change.h"
#include "list.h"
#include "api_process.h"
#include "router_inotify.h"

#ifdef DB_TASK

int exist_mark_file(char *path)
{
	int res = 0;
	char uuid[32] = {0};
	FILE *fd;
	if(path == NULL)
	{
		return -1;
	}

	DMCLOG_D("uuid_path = %s",path);
	if((fd = fopen(path,"r"))== NULL)//改动：路径
	{
		DMCLOG_D("open file error[errno = %d]",errno);
		return -1;
	}
	res = fread(uuid,sizeof(char),16,fd);
	if(res <= 0)
	{
		DMCLOG_D("enRet = %d,errno = %d",res,errno);
		fclose(fd);
		return -1;
	}
	fclose(fd);
	return 0;
}



//获取当前磁盘信息(磁盘标记文件) 以及判断字符串是否为空
int get_disk_mark_info(char *path)
{
	int res = 0;
	char uuid_path[256];
	memset(uuid_path,0,256);
	sprintf(uuid_path,"%s/%s",path,get_sys_disk_uuid_name());
	res = is_file_exist(uuid_path);
	if(res != 0)
	{
		DMCLOG_D("the uuid file is not exist");
		return -1;
	}
	return exist_mark_file(uuid_path);
}


static int add_disk_to_list(void *self,char *uuid ,char *path, char *dev_node)
{
	ENTER_FUNC();
	DiskTaskObj *diskTask = (DiskTaskObj *)self;
	if(diskTask == NULL||uuid == NULL || path == NULL || dev_node == NULL)
	{
		DMCLOG_E("para error");
		return -1;
	}
	pthread_mutex_lock(&diskTask->mutex);
	
	if(get_fuser_flag() == AIRDISK_ON_PC)
	{
		DMCLOG_D("airdisk is on pc");
		return -1;
	}

	disk_node_t *disk_node = (disk_node_t *)calloc(1,sizeof(disk_node_t));
	if(disk_node == NULL)
		return -1;
	pthread_mutex_init(&disk_node->mutex,NULL);
	strcpy(disk_node->uuid,uuid);
	strcpy(disk_node->path,path);
	strcpy(disk_node->dev_node,dev_node);
	DMCLOG_D("path = %s, dev_node = %s",disk_node->path, disk_node->dev_node);
	register_disk_table_ops(disk_node); 
	if((db_disk_check(disk_node->g_database)) != RET_SUCCESS) // need to format hdisk firstly!
	{
		int fd = open(disk_node->g_database, O_RDWR | O_CREAT | O_TRUNC, 0666);
		DMCLOG_D("DATABASE = %s",disk_node->g_database);
		if (fd >= 0) {
			close(fd);
		}
	}
	if(db_disk_module_init(disk_node) != RET_SUCCESS)
    {
        DMCLOG_E("init_db_module failed! Exit Process Now!");
        return -1;
    }
	dl_list_add_tail(&diskTask->head,&disk_node->node);
	pthread_mutex_unlock(&diskTask->mutex);
	if(create_scan_task(disk_node) != RET_SUCCESS)
    {
        DMCLOG_D("start_scan_task failed");
        return -1;
    }
	EXIT_FUNC();
	return 0;
}

void x1000_onDiskListAdd(void *self)
{
	ENTER_FUNC();
	disk_node_t *disk_node;
	DiskTaskObj *diskTask = (DiskTaskObj *)self;
	int same_flag = 0;
	int res,i;
	if(get_fuser_flag() == AIRDISK_ON_PC)
	{
		DMCLOG_D("disk is mounted on pc");
		return ;
	}
	all_disk_t mAll_disk_t;
	memset(&mAll_disk_t,0,sizeof(all_disk_t));
	res = dm_get_storage(&mAll_disk_t);
	if(res != 0)
	{
		DMCLOG_E("get storage error");
		return;
	}

	DMCLOG_D("mAll_disk_t.count: %d", mAll_disk_t.count);
	for(i = 0;i < mAll_disk_t.count;i++)
	{
		DMCLOG_D("mAll_disk_t.disk[%d].name: %s", i, mAll_disk_t.disk[i].name);
		same_flag = 0;
		res = get_disk_mark_info(mAll_disk_t.disk[i].path);
		if(res == 0)
		{
			//有标记文件
			DMCLOG_D("uuid file is exist");
			res = read_mark_file(mAll_disk_t.disk[i].path,mAll_disk_t.disk[i].uuid);
			if(res < 0)
			{
				DMCLOG_D("uuid file is invalid");
				_dm_gen_uuid(mAll_disk_t.disk[i].uuid,mAll_disk_t.disk[i].pid,mAll_disk_t.disk[i].vid, mAll_disk_t.disk[i].total_size, mAll_disk_t.disk[i].free_size);
				DMCLOG_D("uuid = %s",mAll_disk_t.disk[i].uuid);	
				create_mark_file(mAll_disk_t.disk[i].path,mAll_disk_t.disk[i].uuid);//?????uuid??,????????
			}
		}else{
			//????????????
			DMCLOG_D("uuid file is not exist or uuid is null, total_size: %llu, free_size: %llu", mAll_disk_t.disk[i].total_size, mAll_disk_t.disk[i].free_size);
			_dm_gen_uuid(mAll_disk_t.disk[i].uuid,mAll_disk_t.disk[i].pid,mAll_disk_t.disk[i].vid, mAll_disk_t.disk[i].total_size, mAll_disk_t.disk[i].free_size);
			DMCLOG_D("uuid = %s",mAll_disk_t.disk[i].uuid);	
			create_mark_file(mAll_disk_t.disk[i].path,mAll_disk_t.disk[i].uuid);//在磁盘创建uuid文件，标记磁盘的唯一性
		}
		DMCLOG_D("uuid: %s, path: %s", mAll_disk_t.disk[i].uuid, mAll_disk_t.disk[i].path);
		dl_list_for_each(disk_node,&diskTask->head,disk_node_t,node)
		{
			DMCLOG_D("disk_node path: %s, uuid: %s", disk_node->path,disk_node->uuid);
			if(!strcmp(mAll_disk_t.disk[i].uuid, disk_node->uuid)){
				same_flag = 1;
			}
		}

		if((same_flag == 0) && (res = add_disk_to_list(self,mAll_disk_t.disk[i].uuid,mAll_disk_t.disk[i].path,mAll_disk_t.disk[i].dev)) != 0)
		{
			DMCLOG_D("add disk to list and scan error");
			return;
		}
		
	}
	EXIT_FUNC();
}


void x1000_onDiskListDel(void *self)
{	
	disk_node_t *disk_node,*n;
	DiskTaskObj *diskTask = (DiskTaskObj *)self;
	pthread_mutex_lock(&diskTask->mutex);
	if(&diskTask->head == NULL || dl_list_empty(&diskTask->head))
	{
		EXIT_FUNC();
		pthread_mutex_unlock(&diskTask->mutex);
		return ;
	}

	if(diskTask->event == PC_MOUNT_EVENT){
		dl_list_for_each(disk_node,&diskTask->head,disk_node_t,node)
		{
			DMCLOG_D("path: %s, uuid: %s, dev_node: %s",disk_node->path, disk_node->uuid, disk_node->dev_node);
			pthread_mutex_lock(&disk_node->mutex);
			if(disk_node->g_db_write_task.exit_cb != NULL)
				disk_node->g_db_write_task.exit_cb(&disk_node->g_db_write_task);
			if(disk_node->g_db_query_task.exit_cb != NULL)
				disk_node->g_db_query_task.exit_cb(&disk_node->g_db_query_task);
			pthread_mutex_unlock(&disk_node->mutex);
		    pthread_mutex_destroy(&disk_node->mutex);
		}
		dl_list_for_each_safe(disk_node,n,&diskTask->head,disk_node_t,node)
		{
			dl_list_del(&disk_node->node);
			safe_free(disk_node);
		}
	}
	else if(diskTask->event == UDISK_EXTRACT_EVENT){
		dl_list_for_each(disk_node,&diskTask->head,disk_node_t,node)
		{
			DMCLOG_D("path: %s, uuid: %s",disk_node->path, disk_node->uuid);
			if(strcmp(diskTask->actionNode, disk_node->dev_node)){
				continue;
			}
			pthread_mutex_lock(&disk_node->mutex);
			if(disk_node->g_db_write_task.exit_cb != NULL)
				disk_node->g_db_write_task.exit_cb(&disk_node->g_db_write_task);
			if(disk_node->g_db_query_task.exit_cb != NULL)
				disk_node->g_db_query_task.exit_cb(&disk_node->g_db_query_task);
			pthread_mutex_unlock(&disk_node->mutex);
		    pthread_mutex_destroy(&disk_node->mutex);
		}
		dl_list_for_each_safe(disk_node,n,&diskTask->head,disk_node_t,node)
		{
			if(strcmp(diskTask->actionNode, disk_node->dev_node)){
				continue;
			}
			dl_list_del(&disk_node->node);
			safe_free(disk_node);
		}
	}
	pthread_mutex_unlock(&diskTask->mutex);
	uint16_t sign = get_database_sign();
	sign++;
	set_database_sign(sign);
	dm_cycle_change_inotify(data_base_changed);
	EXIT_FUNC();
}

void *category_task_func(void *self)
{
	// init db module
	int no_db_file = 1;
	int fd ;
	register_db_table_ops(); 
	if((db_check()) != RET_SUCCESS) // need to format hdisk firstly!
	{
		fd = open(DATABASE, O_RDWR | O_CREAT, 0666);
		DMCLOG_D("DATABASE = %s",DATABASE);
		if (fd >= 0) {
			close(fd);
		}
	}
	if(db_module_init(no_db_file) != RET_SUCCESS)
    {
        log_error("init_db_module failed! Exit Process Now!");
        log_sync();
        //EXIT(-1);
        ASSERT(0);
    }
    return NULL;
}


int create_category_task(void)
{
	int ret = -1;
	BaseTask g_category_task;
	MEMSET(g_category_task);
	strcpy(g_category_task.task_name, "category_task");
	g_category_task.task_func = category_task_func;
	ret = create_base_task(&g_category_task);
	if(ret < 0)
	{
		 DMCLOG_D("create_category_task failed!");
		 return ret;
	}
	return ret;
}

void db_server_prcs_task(void)
{
	// start db scanning task
	int res = 0;
	all_disk_t mAll_disk_t;
	PTHREAD_T tid_changed_task;	
	
	create_sem_queue();
	create_category_task();
	DiskTaskObj *diskTask = disk_task_init(x1000_onDiskListAdd,x1000_onDiskListDel);
	
	if (0 !=(PTHREAD_CREATE(&tid_changed_task, NULL, (void *)set_on_device_list_changed, diskTask)))
	{
		DMCLOG_D("Create changed task failed!");
        return;
	}
	
	PTHREAD_DETACH(tid_changed_task);
	do{
		memset(&mAll_disk_t,0,sizeof(all_disk_t));
		res = dm_get_storage(&mAll_disk_t);
		if(res == 0&&mAll_disk_t.count > 0)
		{
			DMCLOG_D("drive is mounted");
			break;
		}
	
 		DMCLOG_D("drive_count = %d",mAll_disk_t.count);
		usleep(100000);
	}while(1);
	sleep(2);
	res = dm_tcp_scan_notify_no_wait(AIRDISK_OFF_PC);//向dm_router发起磁盘扫描的请求
	if(res < 0)
	{
		DMCLOG_E("tcp scan notify error");
	}
    return;
}
#endif
