#include "base.h"
#include "hidisk_errno.h"
#include "db/db_base.h"
#include "db/nas_db.h"
#include "db/file_table.h"
#include "util.h"
#include <sys/time.h>
#include <time.h>
#include "config.h"
#include "file_opr.h"

static bool is_db_file_exist(const char *db_path)
{
	int fd;
	if(access(db_path, F_OK) == 0)
	{
		/*fd = open(db_path, O_RDWR | O_CREAT | O_TRUNC, 0666);
		DMCLOG_D("DATABASE = %s",db_path);
		if (fd >= 0) {
			close(fd);
		}*/
		return TRUE;
	}
	
	return FALSE;	
}



static int get_record_cnt_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
    uint32_t *cnt = (uint32_t *)data;

	if(FieldValue[0] != NULL)
	{
		*cnt = strtoul(FieldValue[0], NULL, 0);
	}
	
	return 0;
}

static int get_id_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
    uint32_t *id = (uint32_t *)data;

	*id = strtoul(FieldValue[0], NULL, 0);
	
	return 0;
}

#define DB_CHECK_TOTAL_ITEM_NUMS        500
//#define DB_CHECK_MIN_MATCH_ITEM_NUMS    (DB_CHECK_TOTAL_ITEM_NUMS - (DB_CHECK_TOTAL_ITEM_NUMS >> 2))
#define DB_CHECK_MIN_MATCH_ITEM_NUMS    (DB_CHECK_TOTAL_ITEM_NUMS >> 1)


error_t db_check(void)
{
	error_t errcode = RET_SUCCESS;

    //return RET_SUCCESS;// FOR TEST without check.

	if(!is_db_file_exist(DATABASE))
	{
		DMCLOG_D("database file doesn't exist!\n");
	    return EDB_NOT_EXIST;	
	}
	if(!is_db_file_exist(BACKUP_DB))
	{
		return EDB_BACKUP_DB_NOT_EXIST;	
	}
	return errcode;
}

error_t db_disk_check(char *g_database)
{
	error_t errcode = RET_SUCCESS;

    //return RET_SUCCESS;// FOR TEST without check.

	if(!is_db_file_exist(g_database))
	{
		DMCLOG_D("database file doesn't exist!\n");
	    return EDB_NOT_EXIST;	
	}
	return errcode;
}


