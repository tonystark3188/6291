#include "db/version_table.h"
#include "db/db_base.h"
#include "db/nas_db.h"
#include "db/db_backup.h"
#include "db/sqlite3.h"
#include "config.h"
#include "base.h"
#include "file_opr.h"
#include "db/db_config.h"

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


//fill the table with our db update functions
static struct version_update_table g_update_table = 
{
    .update_item_array[0].update_fn   = db_1_0_0_update,
	.update_item_array[0].dv.major    = 1,
	.update_item_array[0].dv.minor    = 0,
	.update_item_array[0].dv.revision = 0,
	.num = 1,
};

static error_t file_table_alter_cmd(sqlite3 *db)
{
    error_t errcode = RET_SUCCESS;
    char sql[1024] = {0};
    sprintf(sql,"ALTER TABLE file_table ADD ENCRYPT_ID  INT DEFAULT 0");
    if((errcode = sqlite3_exec_busy_wait(db, sql, NULL, NULL)) != SQLITE_OK)
    {
        DMCLOG_E("alter file table error");
        return errcode;
    }
    return errcode;
}

static error_t db_1_0_0_update(sqlite3 *db, struct digit_version *dv)
{
    int errcode = RET_SUCCESS;
    if((errcode = file_table_alter_cmd(db)) != RET_SUCCESS)
    {
        DMCLOG_E("file table add encrypt id error");
        return -1;
    }
    dv->revision++;
    DMCLOG_E("db_1_0_0_update");
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

	if(i == g_update_table.num)
	{
		log_warning("not found matched version in update table!");
		return EDB_INVALID_VERSION;
	}

	//using later registered update fn to update
    for( ; i<g_update_table.num; i++)
    {
    		
		start_transaction(db);

		//mark status as updating
		upd_v.state = 1;
		upd_v.update_state = TRUE;
		upd_v.update_version = FALSE;
		if((errcode = version_table_update(db, &upd_v)) != RET_SUCCESS)
		{
			log_warning("update version table error(0x%X)", errcode);
			return errcode;
		}

		//do database update
		errcode = g_update_table.update_item_array[i].update_fn(db, dv_cur);
		if(errcode != RET_SUCCESS)
		{
			log_warning("update from %s.%s.%s error", dv_cur->major, dv_cur->minor, dv_cur->revision);
			rollback_transaction(db);
			return errcode;
		}

        //update database ok, update the version table
		S_SNPRINTF(upd_v.version, MAX_VERSION_SIZE, "%d.%d.%d", dv_cur->major, dv_cur->minor, dv_cur->revision);
		upd_v.state = 0;
		upd_v.update_state = TRUE;
		upd_v.update_version = TRUE;
		if((errcode = version_table_update(db, &upd_v)) != RET_SUCCESS)
		{
			log_warning("update version table error(0x%X)", errcode);
			rollback_transaction(db);
			return errcode;
		}
		
		log_trace("database update to %d.%d.%d", dv_cur->major, dv_cur->minor, dv_cur->revision);
		commit_transaction(db);
	}

    log_trace("db update ok, now the version is %d.%d.%d", dv_cur->major, dv_cur->minor, dv_cur->revision);

	return RET_SUCCESS;
}


error_t db_update(char *database_path)
{
	sqlite3 *database;
	struct digit_version d_cur,d_tar;
	int res;
	error_t errcode;
    
	errcode = sqlite3_open(database_path, &database);
	if(errcode != SQLITE_OK)
	{
		log_warning("cannot open database\n");
		return errcode;
	}
	
    DMCLOG_D("db version = %s",get_sys_db_version());
	version_to_digit(get_sys_db_version(), &d_tar);
	
    version_info_t cur_version;
    memset(&cur_version,0,sizeof(version_info_t));
	if((errcode = version_table_query(database, &cur_version)) 
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
    
    if(!*cur_version.version)
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

	if(cur_version.state == 1)
	{
		sqlite3_close(database);
//        remove_db_file(database_path);
//		cp_file(BACKUP_DB, DATABASE);

//		errcode = sqlite3_open(database_path, &database);
//		if(errcode != SQLITE_OK)
//		{
//			log_warning("cannot open database\n");
//			return errcode;
//		}
	}

	version_to_digit(cur_version.version, &d_cur);

    res = compare_version(&d_cur, &d_tar);
    if(res == 0)//version number equals, no need to update	
    {
        sqlite3_close(database);
		return RET_SUCCESS;
	}
	else if(res > 0)//error condition
	{
		log_warning("current db version(%s) larger than target version(%s)!", 
				cur_version.version, get_sys_db_version());
        sqlite3_close(database);
		return EDB_INVALID_VERSION;
	}

//    // backup for update.
//    if((errcode = db_backup(database)) != RET_SUCCESS)
//    {
//        log_warning("backup database error(0x%X)", errcode);
//        sqlite3_close(database);
//        return errcode;
//    }
    

	//need to update
	if((errcode = do_db_update(database, &d_cur, &d_tar)) != RET_SUCCESS)
	{
		log_warning("do update error(0x%X)", errcode);
        sqlite3_close(database);
		return errcode;
	}

//    //after update ok, we backup the database
//    if((errcode = db_backup(database)) != RET_SUCCESS)
//    {
//        log_warning("backup database error(0x%X)", errcode);
//        sqlite3_close(database);
//        return errcode;
//    }
    
    sqlite3_close(database);
	return RET_SUCCESS;
}
