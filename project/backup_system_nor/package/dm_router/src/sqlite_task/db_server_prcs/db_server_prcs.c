#include "db_server_prcs.h"
#include "disk_manage.h"
#include "db/disk_change.h"
#include "list.h"
#include "api_process.h"
#ifdef DB_TASK

static int add_disk_to_list(void *self,char *uuid ,char *path)
{
	DiskTaskObj *diskTask = (DiskTaskObj *)self;
	if(diskTask == NULL||uuid == NULL || path == NULL)
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
	DMCLOG_D("path = %s",disk_node->path);
	
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
        return ;
    }
	dl_list_add_tail(&diskTask->head,&disk_node->node);
	pthread_mutex_unlock(&diskTask->mutex);
	if(create_scan_task(disk_node) != RET_SUCCESS)
    {
        DMCLOG_D("start_scan_task failed");
        return -1;
    }
	return 0;
}

void x1000_onDiskListAdd(void *self)
{
	ENTER_FUNC();
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
	
	for(i = 0;i < mAll_disk_t.count;i++)
	{
		res = get_disk_mark_info(mAll_disk_t.disk[i].path);
		if(res == 0)
		{
			//有标记文件
			DMCLOG_D("uuid file is exist");
			res = read_mark_file(mAll_disk_t.disk[i].path,mAll_disk_t.disk[i].uuid);
			if(res < 0)
			{
				DMCLOG_D("uuid file is invalide");
				_dm_gen_uuid(mAll_disk_t.disk[i].uuid,mAll_disk_t.disk[i].pid,mAll_disk_t.disk[i].vid, mAll_disk_t.disk[i].total_size, mAll_disk_t.disk[i].free_size);
				DMCLOG_D("uuid = %s",mAll_disk_t.disk[i].uuid);	
				create_mark_file(mAll_disk_t.disk[i].path,mAll_disk_t.disk[i].uuid);//在磁盘创建uuid文件，标记磁盘的唯一性
			}
		}else{
			//没有标记文件
			DMCLOG_D("uuid file is not exist");
			_dm_gen_uuid(mAll_disk_t.disk[i].uuid,mAll_disk_t.disk[i].pid,mAll_disk_t.disk[i].vid, mAll_disk_t.disk[i].total_size, mAll_disk_t.disk[i].free_size);
			DMCLOG_D("uuid = %s",mAll_disk_t.disk[i].uuid);	
			create_mark_file(mAll_disk_t.disk[i].path,mAll_disk_t.disk[i].uuid);//在磁盘创建uuid文件，标记磁盘的唯一性
		}
		if((res = add_disk_to_list(self,mAll_disk_t.disk[i].uuid,mAll_disk_t.disk[i].path)) != 0)
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
		
	dl_list_for_each(disk_node,&diskTask->head,disk_node_t,node)
	{
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
	pthread_mutex_unlock(&diskTask->mutex);
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
	sleep(2);
	dm_tcp_scan_notify_no_wait(AIRDISK_OFF_PC);//向dm_router发起磁盘扫描的请求
    return;
}
#endif
