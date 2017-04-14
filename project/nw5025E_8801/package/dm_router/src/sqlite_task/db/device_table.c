/*
 * =============================================================================
 *
 *       Filename: device_table.c
 *
 *    Description: device table operation definition.
 *
 *        Version:  1.0
 *        Created:  2014/09/17 
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Tao Sheng (), vanbreaker90@gmail.com
 *   Organization:  
 *
 * =============================================================================
 */



#include "db/nas_db.h"
//#include "db/device_table.h"
#include "db/db_table.h"
//#include "network/net_util.h"


#define DEVICE_TABLE_NAME "device_table"

static uint32_t g_device_id;
db_table_t g_device_table;

//allocate device id
static  uint32_t alloc_device_id()
{
	uint32_t id;
	
	id = g_device_id;
	g_device_id++;
	if(g_device_id < id)
	{
		log_warning("device id overflow!\n");
	}
	
	return g_device_id;
}


//get device information from sql query result
static void load_device_info(device_info_t *pdi, char **FieldValue)
{
	if(FieldValue[0] != NULL)
		pdi->id = strtoul(FieldValue[0], NULL, 10);
	
	if(FieldValue[1] != NULL)
		S_STRNCPY(pdi->device_name, FieldValue[1], MAX_USER_DEV_NAME_LEN - 1);

		
	if(FieldValue[2] != NULL)
		S_STRNCPY(pdi->device_uuid, FieldValue[2], 64);
		
	if(FieldValue[3] != NULL)
		S_STRNCPY(pdi->disk_uuid, FieldValue[3], 16);
	DMCLOG_D("pdi->disk_uuid = %s",pdi->disk_uuid);
}

static void load_device_id(unsigned *pdi, char **FieldValue)
{
	if(FieldValue[0] != NULL)
		*pdi = strtoul(FieldValue[0], NULL, 10);
	DMCLOG_D("id = %u",*pdi);
}



//sqlite exec callback to get device list
static int get_device_list_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	device_list_t *plist = (device_list_t *)data;
	device_info_t *pdi = NULL;

	pdi = (device_info_t *)malloc(sizeof(device_info_t));
	if(pdi == NULL)
	{
        //DMCLOG_D("malloc failed!\n");
		return 1;
	}
        
	load_device_info(pdi, FieldValue);    
	dl_list_add_tail(&plist->head, &pdi->node);
	plist->result_cnt++;
	
	return 0;
}


//sqlite exec callback to get device id
static int get_device_id_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	uint32_t *pid = (uint32_t *)data;

	if(FieldValue[0] != NULL)
	{
		*pid = strtoul(FieldValue[0], NULL, 10);
	}
	
	return 0;
}


//sqlite exec callback to get device information
static int get_device_info_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	device_info_t *pdi = (device_info_t *)data;

	load_device_info(pdi, FieldValue);

	return 0;
}

#if 0
"create table IF NOT EXISTS device_table("\
"ID INT PRIMARY KEY NOT NULL,"\
"DEVICE_NAME CHAR NOT NULL,"\
"DEVICE_UUID CHAR UNIQUE NOT NULL,"\
"DISK_UUID CHAR NOT NULL);"
#endif

/*
*description:combine device information to string for inserting device record
*input param:pdt-->buffer storing device information
*output param:sql-->buffer storing sqlite cmd string
*return: RET_SUCCESS if ok
*/
static error_t load_device_insert_cmd(char *sql, device_info_t *pdt)
{
	int n;
    char dev_str_escape[MAX_USER_DEV_NAME_LEN+0x10];

    sqlite3_str_escape(pdt->device_name, dev_str_escape, sizeof(dev_str_escape));
    
	n = snprintf(sql, SQL_CMD_WRITE_BUF_SIZE, "INSERT INTO %s(ID,DEVICE_NAME,DEVICE_UUID,DISK_UUID) "\
		   "VALUES(%u,'%s','%s','%s')", DEVICE_TABLE_NAME, pdt->id, dev_str_escape,
		   pdt->device_uuid, pdt->disk_uuid);
	
	if(n >= SQL_CMD_WRITE_BUF_SIZE)
	{
		log_warning("sql buffer size is not enough,the cmd is %s",sql);
		return EOUTBOUND;
	}
	
	return RET_SUCCESS;
}

error_t query_device_item_by_uuid(sqlite3 *database,unsigned device_uuid)
{
	unsigned id = 0;
	error_t errcode = RET_SUCCESS;	
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	DMCLOG_D("device_uuid = %s",device_uuid);
	sprintf(sql, "SELECT * FROM %s WHERE DEVICE_UUID='%s'", DEVICE_TABLE_NAME,device_uuid);

	if((errcode = sqlite3_exec_busy_wait(database, sql, get_device_id_callback, &id))
			!= SQLITE_OK)
	{
		return errcode;
	}

	if(id == 0)
	{
		return EDB_RECORD_NOT_EXIST;
	}
}

/*
*description:query device information
*input param:app->query approach
*output param:data->buffer to store query result 
*return: RET_SUCCESS if ok
*/
static error_t query_device_item(sqlite3 *database, QueryApproach app, void *data)
{
	error_t errcode = RET_SUCCESS;	
	char sql[SQL_CMD_QUERY_BUF_SIZE];

	if(database == NULL || data == NULL)
	{
		return ENULL_POINT;
	}

	if(app == QUERY_DEVICE_BY_USER)//just by user
	{
		device_list_t *plist = (device_list_t *)data;

		plist->result_cnt = 0;
		dl_list_init(&plist->head);

		sprintf(sql, "SELECT * FROM %s WHERE DEVICE_UUID='%s'", DEVICE_TABLE_NAME,
			plist->match.device_uuid);

		errcode = sqlite3_exec_busy_wait(database, sql, get_device_list_callback, plist);
	}
	else if(app == QUERY_DEVICE_BY_USER_MAC)//by user and mac addr
	{
		#if 0
		device_list_t *plist = (device_list_t *)data;

		plist->result_cnt = 0;
		dl_list_init(&plist->head);

		sprintf(sql, "SELECT * FROM %s WHERE OWNER_ID=%u AND MAC_ADDR = '%s'", DEVICE_TABLE_NAME,
			plist->match.owner, plist->match.mac_addr);
		
		errcode = sqlite3_exec_busy_wait(database, sql, get_device_list_callback, plist);
		#endif
	}
	else if(app == QUERY_DEVICE_BY_UUID)//by user ,mac addr and device name
	{
		device_info_t *pdi = (device_info_t *)data;

		pdi->id = 0;
		DMCLOG_D("pdi->device_uuid = %s",pdi->device_uuid);
		sprintf(sql, "SELECT * FROM %s WHERE DEVICE_UUID='%s'", DEVICE_TABLE_NAME ,pdi->device_uuid);

		if((errcode = sqlite3_exec_busy_wait(database, sql, get_device_info_callback, pdi))
				!= SQLITE_OK)
		{
			return errcode;
		}

		if(pdi->id == 0)
		{
			return EDB_RECORD_NOT_EXIST;
		}
	}

	return errcode;
}




/*
*description:insert new device record
*input param:data-->buffer storing device information
*output param:
*return: RET_SUCCESS if ok
*/
static error_t insert_device_item(sqlite3 *database, void *data)
{
	device_info_t *pdt = (device_info_t *)data;
	char *sql = get_db_write_task_sql_buf();
	error_t errcode = RET_SUCCESS;

	if(database == NULL || pdt == NULL)
	{
		return ENULL_POINT;
	}
	if((errcode = query_device_item_by_uuid(database,pdt->device_uuid)) == RET_SUCCESS)
	{
		sprintf(sql, "UPDATE %s SET DISK_UUID='%s' WHERE DEVICE_ID='%s'", DEVICE_TABLE_NAME, pdt->device_uuid);

		return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
	}
	pdt->id = alloc_device_id();
	if(pdt->id == 0)
	{
		return EDB_DEVICE_ID_OVERFLOW;
	}

    

	if((errcode = load_device_insert_cmd(sql, pdt)) != RET_SUCCESS)
	{
		return errcode;
	}

	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}


//register device table operation functions ,these are the entries to device table
void register_device_table_ops(void)
{
	memset(&g_device_table, 0, sizeof(g_device_table));
	g_device_table.ops.insert = insert_device_item;
	g_device_table.ops.query  = query_device_item;
}


//initialize device table 
error_t device_table_init(void)
{
	sqlite3 *database;
	error_t errcode = RET_SUCCESS;

	register_device_table_ops();
	
	errcode = sqlite3_open(DATABASE, &database);
	if(errcode != SQLITE_OK)
	{
		log_warning("cannot open database in device_table_init\n");
		return errcode;
	}
	if((errcode = sqlite3_exec_busy_wait(database,"SELECT MAX(ID) FROM device_table",
		     get_device_id_callback, &g_device_id)) != SQLITE_OK)
	{
		g_device_id = 0;
		errcode = RET_SUCCESS;
        sqlite3_close(database);
        return errcode;
	}
    sqlite3_close(database);
	return errcode;	
}
