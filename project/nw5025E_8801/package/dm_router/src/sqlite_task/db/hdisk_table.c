/*
 * =============================================================================
 *
 *       Filename: hdisk_table.c
 *
 *    Description: hdisk table operation definition.
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
#include "db/hdisk_table.h"
#include "db/db_table.h"


#define HDISK_TABLE_NAME "hdisk_table"

db_table_t g_hdisk_table;
uint32_t   g_hdisk_id;


static error_t load_hdisk_insert_cmd(char *sql, hdisk_info_t *phi)
{
	int n;
	
	if(sql == NULL || phi == NULL)
	{
		return ENULL_POINT;
	}

	n = snprintf(sql, SQL_CMD_WRITE_BUF_SIZE, "INSERT INTO %s(ID,UUID,PID,VID,CAPACITY,FREE_CAPACITY,RECYCLE_SIZE,VIDEO_TOTAL_SIZE,"\
		"AUDIO_TOTAL_SIZE,PHOTO_TOTAL_SIZE,ISMOUNTED) VALUES(%d,'%s','%s','%s',%llu,%llu,%llu,%llu,%llu,%llu,%d)",
	    HDISK_TABLE_NAME,phi->id,phi->uuid, phi->pid,phi->vid,phi->total_capacity, phi->free_capacity, phi->recycle_size,
	    phi->video_size, phi->audio_size, phi->photo_size,phi->mount_status);
	DMCLOG_D("phi->total_capacity = %llu",phi->total_capacity);
	if(n >= SQL_CMD_WRITE_BUF_SIZE)
	{
        log_warning("sql buffer size is not enough,the cmd is %s", sql);
		return EOUTBOUND;
	}

	return RET_SUCCESS;
	
}

static int alloc_hdisk_id(void)
{
    uint32_t id;

	id = g_hdisk_id;
	g_hdisk_id++;
	if(g_hdisk_id < id)
	{
        log_warning("hdisk id overflow!\n");
	}
	
	return g_hdisk_id;
}

static error_t hdisk_table_insert(sqlite3 *database, void *hdisk_info)
{
	ENTER_FUNC();
	hdisk_info_t *phi = (hdisk_info_t *)hdisk_info;
	char *sql = get_db_write_task_sql_buf();
	error_t errcode = RET_SUCCESS;

	if(database == NULL || phi == NULL)
	{
		return ENULL_POINT;
	}

	if((errcode = load_hdisk_insert_cmd(sql, phi)) != RET_SUCCESS)
	{
		return errcode;
	}
	EXIT_FUNC();
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

static void load_hdisk_info(char **FieldValue, hdisk_info_t *phi)
{
	ENTER_FUNC();
	if(FieldValue[0] != NULL)
		phi->id  = strtoull(FieldValue[0], NULL, 10);
	if(FieldValue[1] != NULL)
		strcpy(phi->uuid,FieldValue[1]);
	if(FieldValue[2] != NULL)
		strcpy(phi->pid,FieldValue[2]);
	if(FieldValue[3] != NULL)
		strcpy(phi->vid,FieldValue[3]);
	if(FieldValue[4] != NULL)
		phi->total_capacity  = strtoull(FieldValue[4], NULL, 10);
	DMCLOG_D("phi->total_capacity = %llu",phi->total_capacity);
    if(FieldValue[5] != NULL)
	    phi->free_capacity   = strtoull(FieldValue[5], NULL, 10);
	if(FieldValue[6] != NULL)
	    phi->recycle_size   = strtoull(FieldValue[6], NULL, 10);
	if(FieldValue[7] != NULL)
	    phi->video_size   = strtoull(FieldValue[7], NULL, 10);
	if(FieldValue[8] != NULL)
	    phi->audio_size   = strtoull(FieldValue[8], NULL, 10);
	if(FieldValue[9] != NULL)
	    phi->photo_size   = strtoull(FieldValue[9], NULL, 10);
    if(FieldValue[10] != NULL)
	    phi->mount_status     = strtoull(FieldValue[10], NULL, 10);
	EXIT_FUNC();
}

static int get_hdisk_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	if(FieldValue == NULL)
	{
		DMCLOG_D("get hdisk failed");
		return -1;
	}
	load_hdisk_info(FieldValue, data);
	return 0;
}



/*
* description: query hdisk information by id
* input param: id-->target hdisk id
* output param:phd-->buffer to store hdisk information
* return RET_SUCCESS if ok
*/
	
static error_t query_hdisk_by_uuid(sqlite3 *database, char *uuid, hdisk_info_t *phi)
{
	ENTER_FUNC();
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;
	
	if(database == NULL || phi == NULL)
	{
		return ENULL_POINT;
	}
	
	sprintf(sql, "SELECT * FROM %s WHERE UUID='%s'", HDISK_TABLE_NAME, uuid);
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_hdisk_callback, phi))
			!= RET_SUCCESS)
	{
		DMCLOG_D("uuid is not exist");
		return errcode;
	}
	
	EXIT_FUNC();
	return errcode;
}



/*
* description: update hdisk record
* input param:phd->buffer storing hdisk information 
		        action->add,delete,trash,recycle files or delete user.
* output param:
* return RET_SUCCESS if ok
*/
static error_t do_hdisk_update(sqlite3 *database, hdisk_info_t *phd, uint8_t action)
{
	char *sql = get_db_write_task_sql_buf();

    if(action == UPDATE_ADD || action == UPDATE_DEL_TRASH || action == UPDATE_DELETE_USER)
    {
		sprintf(sql, "UPDATE %s SET FREE_CAPACITY=%llu,RECYCLE_SIZE =%llu,VIDEO_TOTAL_SIZE=%llu,"\
			"AUDIO_TOTAL_SIZE=%llu,PHOTO_TOTAL_SIZE=%llu WHERE uuid='%s'",
			HDISK_TABLE_NAME, phd->free_capacity, phd->recycle_size, phd->video_size,
			phd->audio_size, phd->photo_size, phd->uuid);
	}
	else 
	{
		sprintf(sql, "UPDATE %s SET RECYCLE_SIZE=%llu WHERE UUID=%s", HDISK_TABLE_NAME,
			phd->recycle_size, phd->uuid);
	}

	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);

}

error_t hdisk_update_remount(sqlite3 *database, void *mdisk_info)
{
	hdisk_info_t *disk_info = (hdisk_info_t *)mdisk_info;
	char *sql = get_db_write_task_sql_buf();
	sprintf(sql, "UPDATE %s SET FREE_CAPACITY=%llu,CAPACITY=%llu WHERE UUID='%s'", HDISK_TABLE_NAME,
		disk_info->free_capacity,disk_info->total_capacity,disk_info->uuid);

	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);

}

/*
* description: update hdisk information
* input param:update_info-->buffer storing update information
* output param:
* return RET_SUCCESS if ok
*/
static error_t hdisk_table_update(sqlite3 *database, void *update_info)
{
	update_info_t *phd_update_info = (update_info_t *)update_info;
	hdisk_info_t hd_info;
	error_t errcode = RET_SUCCESS;

	if(database == NULL || update_info == NULL)
	{
		return EINVAL_ARG;
	}

	if((errcode = query_hdisk_by_uuid(database, phd_update_info->uuid, &hd_info))
			!= RET_SUCCESS)
	{
		return errcode;
	}

	if(phd_update_info->action == UPDATE_ADD)//add file
	{
		hd_info.free_capacity -= phd_update_info->total_update_size;
		hd_info.video_size    += phd_update_info->video_size;
		hd_info.audio_size    += phd_update_info->audio_size;
		hd_info.photo_size    += phd_update_info->photo_size;
	}
	else if(phd_update_info->action == UPDATE_DEL_TRASH)//delete file
	{
		hd_info.free_capacity += phd_update_info->total_update_size;
		hd_info.recycle_size  -= phd_update_info->total_update_size;
		hd_info.video_size    -= phd_update_info->video_size;
		hd_info.audio_size    -= phd_update_info->audio_size;
		hd_info.photo_size    -= phd_update_info->photo_size;
	}
	else if(phd_update_info->action == UPDATE_DEL_NORM)
	{
		hd_info.free_capacity += phd_update_info->total_update_size;
		hd_info.video_size    -= phd_update_info->video_size;
		hd_info.audio_size    -= phd_update_info->audio_size;
		hd_info.photo_size    -= phd_update_info->photo_size;
	}
	else if(phd_update_info->action == UPDATE_DELETE_USER)//delete user
	{
		hd_info.free_capacity += phd_update_info->total_update_size;
		hd_info.recycle_size  -= phd_update_info->recycle_size;
		hd_info.video_size    -= phd_update_info->video_size;
		hd_info.audio_size    -= phd_update_info->audio_size;
		hd_info.photo_size    -= phd_update_info->photo_size;
	}
	else if(phd_update_info->action == UPDATE_TRASH)//trash file
	{
		hd_info.recycle_size += phd_update_info->total_update_size;
	}
	else//recycle file
	{
		hd_info.recycle_size -= phd_update_info->total_update_size;
	}

	return do_hdisk_update(database, &hd_info, phd_update_info->action);
}


//sqlite exec callback to get hdisk id
static int get_hdisk_id_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	uint32_t *pid = (uint32_t *)data;
	
	if(FieldValue[0] != NULL)
	{
		*pid = strtoul(FieldValue[0], NULL, 10);
	}
	
	return 0;
}




static error_t hdisk_table_query(sqlite3 *database, QueryApproach app, void *data)
{
	ENTER_FUNC();
	error_t errcode = RET_SUCCESS;

	if(database == NULL || data == NULL)
	{
		return ENULL_POINT;
	}

	if(app == QUERY_HDISK_INFO)
	{
		hdisk_info_t *phi = (hdisk_info_t *)data;

		errcode = query_hdisk_by_uuid(database, phi->uuid, phi);
	}
	EXIT_FUNC();
	return errcode;
}


//register hdisk table operation functions,these are the entries to hdisk table.
void register_hdisk_table_ops(void)
{
	memset(&g_hdisk_table, 0, sizeof(g_hdisk_table));

	g_hdisk_table.ops.insert          = hdisk_table_insert;
	g_hdisk_table.ops.passive_update = hdisk_table_update;
	g_hdisk_table.ops.query  		  = hdisk_table_query;
	g_hdisk_table.ops.update   		  = hdisk_update_remount;
}


//initialize hdisk table.
error_t hdisk_table_init(void)
{
	sqlite3 *database;
	error_t errcode = RET_SUCCESS;

	register_hdisk_table_ops();

	errcode = sqlite3_open(DATABASE, &database);
	if(errcode != SQLITE_OK)
	{
		log_warning("cannot open database in hdisk_table_init\n");
		return errcode;
	}

	if((errcode = sqlite3_exec_busy_wait(database,"SELECT MAX(ID) FROM hdisk_table",
		     get_hdisk_id_callback, &g_hdisk_id)) != SQLITE_OK)
	{
		g_hdisk_id = 0;
		errcode = RET_SUCCESS;
        sqlite3_close(database);
        return errcode;
	}

    sqlite3_close(database);
	return errcode;
}

