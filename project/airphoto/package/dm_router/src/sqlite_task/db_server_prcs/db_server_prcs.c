#include "db_server_prcs.h"
#include "disk_manage.h"
#include "db/disk_change.h"

extern struct disk_node *disk_dn;

void onDiskListAdd(void *self)
{
	int fd ;
	int enRet;
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
        log_error("init_db_module failed! Exit Process Now!");
        log_sync();
        return ;
    }

	/*if((enRet = match_disk_info(disk_info)!= RET_SUCCESS)//∆•≈‰ ß∞‹
	{
		handle_update_disk_table(disk_info;
		DMCLOG_D("handle_update_disk_table succ");
		
	}*/
	if(create_scan_task(disk_info) != RET_SUCCESS)
    {
        DMCLOG_D("start_scan_task failed");
        return ;
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
	DMCLOG_D("exit callback succ");
	DMCLOG_D("diskTask->disk_info.uuid = %s",diskTask->disk_info.uuid);
	enRet = del_disk_from_list_for_uuid(&disk_dn,diskTask->disk_info.uuid);
	if(enRet < 0)
	{
		EXIT_FUNC();
		return ;
	}
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
	extern SystemConfigInfo *g_p_sys_conf_info;
	#ifdef DB_TASK
	g_p_sys_conf_info       = &g_sys_info.sys_conf_info;
	#endif
	create_sem_queue();
	
	PTHREAD_T tid_changed_task;	
	DiskTaskObj *diskListenTask = (DiskTaskObj *)malloc(sizeof(DiskTaskObj));
	
	diskListenTask->onDMDiskListAdd = onDiskListAdd;
	diskListenTask->onDMDiskListDel = onDiskListDel;
	if (0 !=(PTHREAD_CREATE(&tid_changed_task, NULL, (void *)set_on_device_list_changed, diskListenTask)))
	{
		DMCLOG_D("Create changed task failed!");
        return;
	}
	PTHREAD_DETACH(tid_changed_task);
	create_category_task();
	//œÚdm_router∑¢∆¥≈≈Ã…®√Ëµƒ«Î«Û
	sleep(2);
	notify_disk_scanning();
    return;
}

