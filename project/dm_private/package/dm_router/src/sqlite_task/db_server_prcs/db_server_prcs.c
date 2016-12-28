#include "db_server_prcs.h"
#include "disk_manage.h"
#include "db/disk_change.h"
#include "list.h"
#ifdef DB_TASK

extern struct disk_node *disk_dn;

void onDiskListAdd(void *self)
{
	int fd ;
	int enRet;
	char *tmp;
    DiskTaskObj *diskTask = (DiskTaskObj *)self;
	enRet = add_disk_to_disk_list(&disk_dn,&diskTask->disk_info);
	if(enRet < 0)
	{
		return ;
	}
	struct disk_node *disk_info = disk_dn;
	register_disk_table_ops(disk_info); 
	if((db_disk_check(disk_info->g_database)) != RET_SUCCESS) // need to format hdisk firstly!
	{
		fd = open(disk_info->g_database, O_RDWR | O_CREAT | O_TRUNC, 0666);
		DMCLOG_D("DATABASE = %s",disk_info->g_database);
		if (fd >= 0) {
			close(fd);
		}
	}
	if(db_disk_module_init(disk_info) != RET_SUCCESS)
    {
        DMCLOG_E("init_db_module failed! Exit Process Now!");
        return ;
    }
	/*if((enRet = match_disk_info(disk_info)!= RET_SUCCESS)//匹配失败
	{
		handle_update_disk_table(disk_info;
		DMCLOG_D("handle_update_disk_table succ");
		
	}*/
    pthread_mutex_init(&disk_info->mutex,NULL);
	if(create_scan_task(disk_info) != RET_SUCCESS)
    {
        DMCLOG_D("start_scan_task failed");
        return ;
    }
}

void x1000_onDiskListAdd(void *self)
{
	int res,i;
	DiskTaskObj *diskTask = (DiskTaskObj *)self;
	all_disk_t mAll_disk_t;
	disk_node_t *disk_node;
	memset(&mAll_disk_t,0,sizeof(all_disk_t));
	res = dm_get_storage(&mAll_disk_t);
	if(res != 0)
		return;
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
		disk_node = (disk_node_t *)calloc(1,sizeof(disk_node_t));
		if(disk_node == NULL)
			return ;
		strcpy(disk_node->uuid,mAll_disk_t.disk[i].uuid);
		strcpy(disk_node->path,mAll_disk_t.disk[i].path);
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
	    pthread_mutex_init(&disk_node->mutex,NULL);
		if(create_scan_task(disk_node) != RET_SUCCESS)
	    {
	        DMCLOG_D("start_scan_task failed");
	        return ;
	    }
		dl_list_add_tail(&diskTask->head,&disk_node->node);
		disk_node->dn_next = disk_dn;
		disk_dn = disk_node;
	}
}

void onDiskListDel(void *self)
{
	ENTER_FUNC();
	int enRet;
    DiskTaskObj *diskTask = (DiskTaskObj *)self;
	DMCLOG_D("diskTask->disk_info.uuid = %s",diskTask->disk_info.uuid);
	struct disk_node  *disk_info = get_disk_node(diskTask->disk_info.uuid);
	if(disk_info == NULL)
	{
		DMCLOG_D("no disk info");
		return ;
	}
	if(disk_info->g_db_write_task.exit_cb != NULL)
		disk_info->g_db_write_task.exit_cb(&disk_info->g_db_write_task);
	if(disk_info->g_db_query_task.exit_cb != NULL)
		disk_info->g_db_query_task.exit_cb(&disk_info->g_db_query_task);
	DMCLOG_D("diskTask->disk_info.uuid = %s",diskTask->disk_info.uuid);
    pthread_mutex_destroy(&disk_info->mutex);
	enRet = del_disk_from_list_for_uuid(&disk_dn,diskTask->disk_info.uuid);
	if(enRet < 0)
	{
		EXIT_FUNC();
		return ;
	}
	EXIT_FUNC();
}

void x1000_onDiskListDel(void *self)
{
	disk_node_t *disk_node,*n;
	DiskTaskObj *diskTask = (DiskTaskObj *)self;
	if(&diskTask->head == NULL || dl_list_empty(&diskTask->head))
		return ;
	dl_list_for_each(disk_node,&diskTask->head,disk_node_t,node)
	{
		if(disk_node->g_db_write_task.exit_cb != NULL)
			disk_node->g_db_write_task.exit_cb(&disk_node->g_db_write_task);
		if(disk_node->g_db_query_task.exit_cb != NULL)
			disk_node->g_db_query_task.exit_cb(&disk_node->g_db_query_task);
	    pthread_mutex_destroy(&disk_node->mutex);
	}
	dl_list_for_each_safe(disk_node,n,&diskTask->head,disk_node_t,node)
	{
		dl_list_del(&disk_node->node);
		safe_free(disk_node);
	}
	
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
	create_sem_queue();
	
	PTHREAD_T tid_changed_task;	
	DiskTaskObj *diskListenTask = (DiskTaskObj *)calloc(1,sizeof(DiskTaskObj));
	dl_list_init(&diskListenTask->head);
	diskListenTask->onDMDiskListAdd = x1000_onDiskListAdd;
	diskListenTask->onDMDiskListDel = x1000_onDiskListDel;
	if (0 !=(PTHREAD_CREATE(&tid_changed_task, NULL, (void *)set_on_device_list_changed, diskListenTask)))
	{
		DMCLOG_D("Create changed task failed!");
        return;
	}
	PTHREAD_DETACH(tid_changed_task);
	create_category_task();
	//向dm_router发起磁盘扫描的请求
	sleep(2);
	notify_disk_scanning();
    return;
}
#endif
