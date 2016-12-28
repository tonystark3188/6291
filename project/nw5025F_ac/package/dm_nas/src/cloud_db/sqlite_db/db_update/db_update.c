/*
 * =============================================================================
 *
 *       Filename: db_update.h
 *
 *    Description: database update module.
 *
 *        Version:  1.0
 *        Created:  2016/12/10
 *       Revision:  none
 *       Compiler:  gcc
 *
 *          Author:  Oliver <henry.jin@dmsys.com>
 *   Organization:  longsys
 *
 * =============================================================================
 */
#include "version_table.h"
#include "config.h"
#include "db_backup.h"
#include "sqlite_db.h"

//version update function typedef
typedef error_t (*db_update_fn)(sqlite3 *db, struct digit_version *dv);


struct db_update_item
{
	db_update_fn update_fn;//update fn
	struct digit_version dv;//target version for updating
};

struct version_update_table
{
	struct db_update_item update_item_array[32];
	int num;
};


static error_t db_1_0_0_update(sqlite3 *db, struct digit_version *dv);
static error_t db_1_0_1_update(sqlite3 *db, struct digit_version *dv);



//fill the table with our db update functions
static struct version_update_table g_update_table = 
{
    .update_item_array[0].update_fn   = db_1_0_0_update,
	.update_item_array[0].dv.major    = 1,
	.update_item_array[0].dv.minor    = 0,
	.update_item_array[0].dv.revision = 0,
	.update_item_array[1].update_fn   = db_1_0_1_update,
	.update_item_array[1].dv.major    = 1,
	.update_item_array[1].dv.minor    = 0,
	.update_item_array[1].dv.revision = 1,
	.num = 2,
};
static error_t v_file_table_alter_time(sqlite3 *db,char *buncket_name)
{
	error_t errcode = RET_SUCCESS;
	char sql[1024] = {0};
	sprintf(sql,"ALTER TABLE %s ADD CREATE_TIME  INT  DEFAULT 0",buncket_name);
	if((errcode = sqlite3_exec_busy_wait(db, sql, NULL, NULL)) != SQLITE_OK)
	{
		DMCLOG_E("alter v_table_table error");
		return errcode;
	}
	sprintf(sql,"ALTER TABLE %s ADD MODIFY_TIME  INT  DEFAULT 0",buncket_name);
	if((errcode = sqlite3_exec_busy_wait(db, sql, NULL, NULL)) != SQLITE_OK)
	{
		DMCLOG_E("alter v_table_table error");
		return errcode;
	}
	sprintf(sql,"ALTER TABLE %s ADD ACCESS_TIME  INT  DEFAULT 0",buncket_name);
	if((errcode = sqlite3_exec_busy_wait(db, sql, NULL, NULL)) != SQLITE_OK)
	{
		DMCLOG_E("alter v_table_table error");
		return errcode;
	}
	sprintf(sql,"ALTER TABLE %s ADD SIZE  INT  DEFAULT 0",buncket_name);
	if((errcode = sqlite3_exec_busy_wait(db, sql, NULL, NULL)) != SQLITE_OK)
	{
		DMCLOG_E("alter v_table_table error");
		return errcode;
	}
	sprintf(sql,"ALTER TABLE %s ADD GB_NAME  CHAR",buncket_name);
	if((errcode = sqlite3_exec_busy_wait(db, sql, NULL, NULL)) != SQLITE_OK)
	{
		DMCLOG_E("alter v_table_table error");
		return errcode;
	}
	sprintf(sql,"ALTER TABLE %s ADD UUID  CHAR",buncket_name);
	if((errcode = sqlite3_exec_busy_wait(db, sql, NULL, NULL)) != SQLITE_OK)
	{
		DMCLOG_E("alter v_table_table error");
		return errcode;
	}
	return errcode;
}

static error_t db_1_0_0_update(sqlite3 *db, struct digit_version *dv)
{
	DMCLOG_E("db_1_0_0_update");
	return RET_SUCCESS;
}

static error_t db_1_0_1_update(sqlite3 *db, struct digit_version *dv)
{
	error_t errcode = RET_SUCCESS;
	char **bucket_list = (char **)handle_bucket_table_list_query();
	if(bucket_list == NULL)
	{
		DMCLOG_D("it is no bucket to update");
		dv->revision++;
		DMCLOG_D("db_1_0_1_update succ");
		return RET_SUCCESS;
	}
	DMCLOG_D("get bucket list succ");	
	int i = 0;
	for(i = 0;bucket_list[i];i++)
	{
		errcode = v_file_table_alter_time(db,bucket_list[i]);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("alter time error");
			break;
		}
	}

	for(i = 0;bucket_list[i];i++)
	{
		safe_free(bucket_list[i]);
	}
	safe_free(bucket_list);
	
	errcode = v_file_table_alter_time(db,PUBLIC_BUCKET_NAME);
	if(errcode != RET_SUCCESS)
	{
		DMCLOG_E("alter time error");
		return errcode;
	}
	if(errcode != RET_SUCCESS)
	{
		DMCLOG_D("db_1_0_1_update error");
		return errcode;
	}
	dv->revision++;
	DMCLOG_D("db_1_0_1_update succ");
	return RET_SUCCESS;
}


//compare two version
static int compare_version(struct digit_version *v1, struct digit_version *v2)
{
    if(v1->major != v2->major)
    {
		return v1->major - v2->major;
	}
	else
	{
		if(v1->minor != v2->minor)
		{
			return v1->minor - v2->minor;
		}
		else
		{
			return v1->revision - v2->revision;
		}
	}
}



static error_t do_db_update(sqlite3 *db, struct digit_version *dv_cur, struct digit_version *dv_tar)
{
    int i;
	update_version_t upd_v;
	error_t errcode;

	for(i=0;i<g_update_table.num;i++)
	{
		if(compare_version(dv_cur, &g_update_table.update_item_array[i].dv) == 0)
		{
			break;
		}
	}
	DMCLOG_D("i = %d,g_update_table.num = %d",i,g_update_table.num);

	if(i == g_update_table.num)
	{
		DMCLOG_E("not found matched version in update table!");
		return EDB_INVALID_VERSION;
	}

	//using later registered update fn to update
    for( ; i<g_update_table.num; i++)
    {	
		start_transaction(db);
		//mark status as updating
		S_SNPRINTF(upd_v.version, MAX_VERSION_SIZE, "%d.%d.%d", dv_cur->major, dv_cur->minor, dv_cur->revision);
		upd_v.state = 1;
		upd_v.update_state = TRUE;
		upd_v.update_version = FALSE;
		
		if((errcode = version_table_update(db, &upd_v)) != RET_SUCCESS)
		{
			DMCLOG_E("update version table error(0x%X)", errcode);
			return errcode;
		}

		//do database update
		errcode = g_update_table.update_item_array[i].update_fn(db, dv_cur);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("update from %d.%d.%d error", dv_cur->major, dv_cur->minor, dv_cur->revision);
			rollback_transaction(db);
			DMCLOG_E("update error");
			return errcode;
		}

        //update database ok, update the version table
		S_SNPRINTF(upd_v.version, MAX_VERSION_SIZE, "%d.%d.%d", dv_cur->major, dv_cur->minor, dv_cur->revision);
		upd_v.state = 0;
		upd_v.update_state = TRUE;
		upd_v.update_version = TRUE;
		if((errcode = version_table_update(db, &upd_v)) != RET_SUCCESS)
		{
			DMCLOG_E("update version table error(0x%X)", errcode);
			rollback_transaction(db);
			return errcode;
		}
		
		DMCLOG_D("database update to %d.%d.%d", dv_cur->major, dv_cur->minor, dv_cur->revision);
		commit_transaction(db);
	}

    DMCLOG_D("db update ok, now the version is %d.%d.%d", dv_cur->major, dv_cur->minor, dv_cur->revision);
	return RET_SUCCESS;
}


error_t db_update(void)
{
	sqlite3 *database;
	version_info_t cur_version;
	struct digit_version d_cur,d_tar;
	int res;
	error_t errcode;
    
	errcode = sqlite3_open(DATABASE, &database);
	if(errcode != SQLITE_OK)
	{
		DMCLOG_E("cannot open database\n");
		return errcode;
	}
	
	version_to_digit(get_sys_db_version(), &d_tar);//获取目标版本数字
	
	if((errcode = version_table_query(database, &cur_version)) //获取当前数据库版本
			!= RET_SUCCESS)
	{
		if((errcode = sqlite3_exec_busy_wait(database, SQLITE_CREATE_VERSION_TABLE, NULL, NULL)) != SQLITE_OK)
		{
			DMCLOG_E("create version_table error");
			return errcode;
		}
		S_STRNCPY(cur_version.version, "1.0.0", MAX_VERSION_SIZE);
		cur_version.state = 0;
		if((errcode = version_table_insert(database, &cur_version)) != RET_SUCCESS)
		{
			DMCLOG_E("insert version info error.");
			return errcode;
		}	
	}

	DMCLOG_D("cur_version.state = %d",cur_version.state);
	if(cur_version.state == 1)
	{
		sqlite3_close(database);
        remove_db_file(DATABASE);
		cp_file(BACKUP_DB, DATABASE);

		errcode = sqlite3_open(DATABASE, &database);
		if(errcode != SQLITE_OK)
		{
			DMCLOG_E("cannot open database\n");
			return errcode;
		}
	}

	version_to_digit(cur_version.version, &d_cur);//获取当前数据库版本数字

    res = compare_version(&d_cur, &d_tar);
    if(res == 0)//version number equals, no need to update	
    {
        sqlite3_close(database);
		return RET_SUCCESS;
	}
	else if(res > 0)//error condition
	{
		DMCLOG_E("current db version(%s) larger than target version(%s)!", 
				cur_version.version, get_sys_db_version());
        sqlite3_close(database);
		return EDB_INVALID_VERSION;
	}

    // backup for update.
    if((errcode = db_backup(database)) != RET_SUCCESS)
    {
        DMCLOG_E("backup database error(0x%X)", errcode);
        sqlite3_close(database);
        return errcode;
    }
    
	//need to update
	if((errcode = do_db_update(database, &d_cur, &d_tar)) != RET_SUCCESS)
	{
		DMCLOG_E("do update error(0x%X)", errcode);
        sqlite3_close(database);
		return errcode;
	}

    //after update ok, we backup the database
    if((errcode = db_backup(database)) != RET_SUCCESS)
    {
        DMCLOG_E("backup database error(0x%X)", errcode);
        sqlite3_close(database);
        return errcode;
    }
    
    sqlite3_close(database);
	return RET_SUCCESS;
}

