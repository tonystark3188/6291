#include "db_prcs.h"


void *db_task_func(void *self)
{
	// init db module
	register_db_table_ops(); 
	if(access(DATABASE, F_OK) != 0) // need to format hdisk firstly!
	{
		int fd = open(DATABASE, O_RDWR | O_CREAT, 0666);
		DMCLOG_D("DATABASE = %s",DATABASE);
		if (fd >= 0) {
			close(fd);
		}
	}
	
	if(db_module_init() != RET_SUCCESS)
    {
        DMCLOG_E("init_db_module failed! Exit Process Now!");
        ASSERT(0);
    }

	if(db_update() != RET_SUCCESS)
	{
		DMCLOG_E("failed to update db");
        ASSERT(0);
	}
    return NULL;
}


int create_prcs_task(void)
{
	int ret = -1;
	BaseTask g_category_task;
	MEMSET(g_category_task);
	strcpy(g_category_task.task_name, "category_task");
	g_category_task.task_func = db_task_func;
	ret = create_base_task(&g_category_task);
	if(ret < 0)
	{
		 DMCLOG_E("create_category_task failed!");
		 return ret;
	}
	return ret;
}

void db_prcs_task(void)
{
	create_sem_queue();
	create_prcs_task();
    return;
}
