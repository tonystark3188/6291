#include "db/nas_db.h"
#include "db/user_table.h"
#include "db/db_table.h"


#define USER_TABLE_NAME "user_table"


static uint32_t g_user_id;

db_table_t g_user_table;

static int get_user_id_callback(void *data, int nFields, char **FieldValue, char **FieldName);
static int get_user_callback(void *data, int nFields, char **FieldValue, char **FieldName);


//allocate new user id.
static uint32_t alloc_user_id(void)
{
    uint32_t id;

	id = g_user_id;
	g_user_id++;
	if(g_user_id < id)
	{
        log_warning("user id overflow!\n");
	}
	
	return g_user_id;
}


/*
*description:combine user information to string for inserting user record
*input param:puser-->buffer storing user information
*output param:sql-->buffer storing sqlite cmd string
*return: RET_SUCCESS if ok
*/
#if 0
#define SQLITE_CREATE_USER_TABLE \
		"create table IF NOT EXISTS user_table(SESSION CHAR PRIMARY KEY NOT NULL,"\
		"USER_NAME	  CHAR  NOT NULL,"\
		"PASSWORD	  CHAR NOT NULL,"\
		"IP 	   CHAR NOT NULL,"\
		"DEVICE_UUID CHAR NOT NULL,"\
		"DEVICE_NAME CHAR);"
#endif
static error_t load_user_insert_cmd(char *sql, user_info_t *puser)
{
	int n;

	if(sql == NULL || puser == NULL)
	{
		return ENULL_POINT;
	}

	n = snprintf(sql, SQL_CMD_WRITE_BUF_SIZE, "INSERT INTO %s(SESSION,USER_NAME,PASSWORD,IP,DEVICE_UUID,"\
		"DEVICE_NAME) "\
		"VALUES('%s','%s','%s','%s','%s','%s')",USER_TABLE_NAME,
		puser->session, puser->user_name, puser->password, puser->ip, 
		puser->device_uuid, puser->device_name_escape);

	if(n >= SQL_CMD_WRITE_BUF_SIZE)
	{
		return EOUTBOUND;
	}

	return RET_SUCCESS;
}


struct user_id_list
{
	uint32_t id_buf[MAX_USER_QUANTITY];
	uint32_t cnt;
};

static int get_valid_user_for_share_list_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	struct user_id_list *p = (struct user_id_list *)data;

	if(p->cnt < MAX_USER_QUANTITY)
	{
		p->id_buf[p->cnt++] = strtoul(FieldValue[0], NULL, 10);
	}
	else
	{
		log_warning("too many users in db");
		return 1;
	}

	return 0;
	
}



error_t  get_valid_user_for_share_list(sqlite3 *database, char *sql, char *buf, ssize_t buf_size)
{
	char id_str[10];
	int len=0,i;
	struct user_id_list id_list;
	error_t errcode;

	id_list.cnt = 0;
	
	sprintf(sql,"SELECT * FROM %s WHERE VALID=1", USER_TABLE_NAME);
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_valid_user_for_share_list_callback, &id_list))
		 	!= RET_SUCCESS)
	{
		log_warning("get valid user list error");
		return errcode;
	}

    if(id_list.cnt == 0)
    {
        return EDB_EMPTY_LIST;
    }

	if(id_list.cnt == 1)
	{
		snprintf(buf, buf_size, "USER_INFO_INDEX = %u AND", id_list.id_buf[0]);
		return RET_SUCCESS;
	}

	len = snprintf(buf, buf_size, "USER_INFO_INDEX IN (");
	buf_size -= len;
	if(buf_size < 1)
	{
		return EOUTBOUND;
	}

    len = snprintf(id_str, sizeof(id_str), "%u", id_list.id_buf[0]);
	if((buf_size -= len) < 1)
	{
		return EOUTBOUND;
	}
	strcat(buf, id_str);
	
	for(i=1; i<id_list.cnt; i++)
	{
		len = snprintf(id_str, sizeof(id_str), ",%u", id_list.id_buf[i]);
		if((buf_size -= len) < 1)
		{
			return EOUTBOUND;
		}
		strcat(buf, id_str);
	}

	if(buf_size < sizeof(") AND"))
	{
		return EOUTBOUND;
	}
	strcat(buf, ") AND");

	return RET_SUCCESS;
}


/*
*description: query user information by user name.
*input param:user_name->buffer storing user name
return: RET_SUCC*output param:pui->buffer to store user information
*ESS if ok
*/
static error_t user_table_query_by_name(sqlite3 *database, char *user_name, user_info_t *pui)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;

	pui->id = INVALID_USER_ID;

	sprintf(sql,"SELECT * FROM %s WHERE USER_NAME='%s'", USER_TABLE_NAME, user_name);
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_user_callback, pui))
			!= SQLITE_OK)
	{
		return errcode;
	}

	if(pui->id == INVALID_USER_ID)
	{
		return EDB_RECORD_NOT_EXIST;
	}
	
	return errcode;
}
static error_t user_table_query_by_ip(sqlite3 *database, char *ip, user_info_t *pui)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;

	pui->id = INVALID_USER_ID;

	sprintf(sql,"SELECT * FROM %s WHERE IP='%s'", USER_TABLE_NAME, ip);
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_user_callback, pui))
			!= SQLITE_OK)
	{
		return errcode;
	}

	if(!*pui->session)
	{
		return EDB_RECORD_NOT_EXIST;
	}
	
	return errcode;
}


/*
*description: query user information by user name.
*input param:user_name->buffer storing user name
return: RET_SUCC*output param:pui->buffer to store user information
*ESS if ok
*/
static error_t user_table_query_by_session(sqlite3 *database, char *session, user_info_t *pui)
{
	ENTER_FUNC();
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;

	sprintf(sql,"SELECT * FROM %s WHERE SESSION='%s'", USER_TABLE_NAME, session);
	memset(pui->session,0,32);
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_user_callback, pui))
			!= SQLITE_OK)
	{
		return errcode;
	}
	if(pui->session != NULL&&!*pui->session)
	{
		return EDB_RECORD_NOT_EXIST;
	}
	EXIT_FUNC();
	return errcode;
}


/*
*description: mark user as a valid user
*input param:id->target user id
*output param:
*return: RET_SUCCESS if ok
*/

static error_t valid_user_item(sqlite3 *database, uint32_t id)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];

	if(database == NULL)
    {
		return EINVAL_ARG;
	}

	sprintf(sql, "UPDATE %s SET VALID=1 WHERE ID=%u", USER_TABLE_NAME, id);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

/*
*description: insert a new user record
*input param:user_info->buffer storing user information
*output param:
*return: RET_SUCCESS if ok
*/
static error_t user_table_insert(sqlite3 *database, void *user_info)
{
	user_info_t *puser = (user_info_t*)user_info;
	user_info_t ui;
	char *sql = get_db_write_task_sql_buf();
	error_t errcode = RET_SUCCESS;

	if(database == NULL || puser == NULL)
	{
		return ENULL_POINT;
	}

	memset(&ui, 0, sizeof(ui));
	
	errcode = user_table_query_by_session(database, puser->session, &ui);
	if(errcode == EDB_RECORD_NOT_EXIST)//user not exist, go ahead
	{
		sqlite3_str_escape(puser->device_name, puser->device_name_escape, sizeof(puser->device_name_escape));
		if((errcode = load_user_insert_cmd(sql,puser)) != RET_SUCCESS)
		{
			return errcode;
		}

		if((errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL))
				!= SQLITE_OK)
		{
			return errcode;
		}
	}
	return errcode;
}
static int get_user_session_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	if(FieldValue[0] != NULL)
		S_STRNCPY(data, FieldValue[0], 32);
	return 0;
}


/*
*description: check whether user exists
*input param: id->target user id
*output param:
*return: TRUE if user existed, otherwise FALSE
*/
static bool user_exist(sqlite3 *database, char *session)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	char temp_session[32];
	memset(temp_session,0,32);

	sprintf(sql, "SELECT * FROM %s WHERE session='%s'", USER_TABLE_NAME, session);
	if(sqlite3_exec_busy_wait(database, sql, get_user_session_callback, temp_session)
			!= SQLITE_OK)
	{
		return FALSE;
	}

	if(strcmp(session,temp_session))
	{
		return FALSE;
	}

	return TRUE;
}


#if 0
static error_t invalid_user_item(sqlite3 *database, void *target)
{
	uint32_t user_id;
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	
    if(database == NULL || target == NULL)
    {
		return EINVAL_ARG;
	}
	
	user_id = *(uint32_t*)target;
	sprintf(sql,"UPDATE %s SET VALID=0 WHERE ID=%u", USER_TABLE_NAME, user_id);
	return  sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}
#endif



/*
*description:delete user record
*input param: target->buffer storing target id
*output param:
*return: RET_SUCCESS if ok
*/
static error_t user_table_delete(sqlite3 *database, void *target)
{
	char *sql = get_db_write_task_sql_buf();

	if(database == NULL || target == NULL)
	{
		return EINVAL_ARG;
	}
	char *session = (char *)target;
	#if 0
	if(!user_exist(database, session))
	{
		return EDB_RECORD_NOT_EXIST;
	}
	#endif
	sprintf(sql, "DELETE FROM %s WHERE SESSION='%s'", USER_TABLE_NAME, session);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}


//sqlite exec callback to get user id
static int get_user_id_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	uint32_t *pid = (uint32_t *)data;

	if(FieldValue[0] != NULL)
	{
		*pid = strtoul(FieldValue[0], NULL, 10);
	}
	
	return 0;
}
//sqlite exec callback to get user id

#if 0
#define SQLITE_CREATE_USER_TABLE \
		"create table IF NOT EXISTS user_table(SESSION CHAR PRIMARY KEY NOT NULL,"\
		"USER_NAME	  CHAR  NOT NULL,"\
		"PASSWORD	  CHAR NOT NULL,"\
		"IP 	   CHAR NOT NULL,"\
		"DEVICE_UUID CHAR NOT NULL,"\
		"DEVICE_NAME CHAR);"
#endif


//get user information from sqlite query result
static void load_user_info(user_info_t *puser, char **FieldValue)
{
	if(FieldValue[0] != NULL)
		S_STRNCPY(puser->session, FieldValue[0], 32);
	DMCLOG_D("puser->session = %s",puser->session);

	if(FieldValue[3] != NULL)
		S_STRNCPY(puser->ip, FieldValue[3], 32);
	DMCLOG_D("puser->ip = %s",puser->ip);

	if(FieldValue[4] != NULL)
		S_STRNCPY(puser->device_uuid, FieldValue[4], 64);
	DMCLOG_D("puser->device_uuid = %s",puser->device_uuid);

	if(FieldValue[5] != NULL)
	{
		S_STRNCPY(puser->device_name,FieldValue[5],MAX_USER_DEV_NAME_LEN - 1);
		DMCLOG_D("puser->device_name = %s",puser->device_name);
	}
	
}


static int get_user_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	user_info_t *puser = (user_info_t *)data;

	load_user_info(puser,FieldValue);	
	
	return 0;
}



/*
*description: query user information by user name.
*input param:id->target user id
*output param:puser->buffer to store user information
*return: RET_SUCCESS if ok
*/
error_t user_table_query_by_id(sqlite3 *database, uint32_t id, user_info_t *puser)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;
	
	if(database == NULL || puser == NULL)
	{
		return ENULL_POINT;
	}

	puser->id = INVALID_USER_ID;
	puser->user_name[0] = 0;
	puser->password[0] = 0;
	
	sprintf(sql, "SELECT * FROM %s WHERE ID=%u", USER_TABLE_NAME, id);
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_user_callback, puser))
			!= RET_SUCCESS)
	{
		return errcode;
	}

	if(puser->id == INVALID_USER_ID || puser->id != id)
	{
		return EDB_RECORD_NOT_EXIST;
	}

	return errcode;
}




//sqlite exec callback to get user list
static int get_user_list_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	user_list_t *plist = (user_list_t *)data;
	user_info_t *pui = NULL;

	pui = (user_info_t *)malloc(sizeof(user_info_t));
	if(pui == NULL)
	{
		return 1;
	}

	load_user_info(pui, FieldValue);
	dl_list_add_tail(&plist->head, &pui->node);
	plist->result_cnt++;
	
	return 0;
}



/*
*description: query user list in user table
*input param: 
*output param:plist->buffer to store user list
*return: RET_SUCCESS if ok
*/
static error_t user_table_query_list(sqlite3 *database, user_list_t *plist)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];

	if(database == NULL || plist == NULL)
	{
		return ENULL_POINT;
	}

	dl_list_init(&plist->head);
	plist->result_cnt = 0;

	sprintf(sql, "SELECT * FROM %s", USER_TABLE_NAME);
	
	return sqlite3_exec_busy_wait(database, sql, get_user_list_callback, plist);
}



/*
*description: query user information
*input param:query_app-->query approach.. we can query user information by id or by name or query
					       all the existed users
*output param:data-->buffer to store query result.
*return: RET_SUCCESS if ok
*/
static error_t user_table_query(sqlite3 *database, QueryApproach query_app, void *data)
{
	error_t errcode = RET_SUCCESS;
	if(database == NULL || data == NULL)
	{
		return ENULL_POINT;
	}

	if(query_app == QUERY_USER_INFO_BY_ID)
	{
		user_info_t *pui = (user_info_t *)data;
		uint32_t id = pui->id;

		errcode = user_table_query_by_id(database, id, pui);
	}
	else if(query_app == QUERY_USER_INFO_BY_NAME)
	{
		user_info_t *pui = (user_info_t *)data;

		errcode = user_table_query_by_name(database, pui->user_name, pui);
	}
	else if(query_app == QUERY_USER_INFO_BY_IP)
	{
		user_info_t *pui = (user_info_t *)data;

		errcode = user_table_query_by_ip(database, pui->ip, pui);
	}
	else if(query_app == QUERY_USER_INFO_BY_SESSION)
	{
		user_info_t *pui = (user_info_t *)data;
		
		errcode = user_table_query_by_session(database, pui->session, pui);
	}
	else if(query_app == QUERY_USER_LIST)
	{
		user_list_t *puser_list = (user_list_t *)data; 
		
		errcode = user_table_query_list(database, puser_list);
	}
	return errcode;
}

/*
static bool user_table_delete(sqlite3 *database, void *target, delete_callback clean_up_fn)
{
	uint32_t user_id;

	if(database == NULL || target == NULL)
    {
		return FALSE;
	}

	user_id = *(uint16_t *)target;
	sprintf(sql, "SELECT * FROM %s")
	
}*/

#if 0
/*
*description: update sqlite user record 
*input param:puser->buffer storing user information   action-->add, delete,trash or recycle
*output param:
*return: RET_SUCCESS if ok
*/
static error_t update_user_capacity(sqlite3 *database, user_info_t *puser, uint8_t action)
{
	char *sql = get_db_write_task_sql_buf();
	
    if(action == UPDATE_ADD || action == UPDATE_DEL_TRASH || action == UPDATE_DEL_NORM)
    {
		sprintf(sql, "UPDATE %s SET HD_USED_SIZE=%llu,HD_RECYCLE_SIZE=%llu,HD_VIDEO_SIZE=%llu,"\
			"HD_AUDIO_SIZE=%llu,HD_PHOTO_SIZE=%llu WHERE ID=%u", USER_TABLE_NAME,
			puser->hd_used_size, puser->hd_recycle_size, puser->hd_video_size,
			puser->hd_audio_size, puser->hd_photo_size, puser->id);
	}
	else
	{
		sprintf(sql, "UPDATE %s SET HD_RECYCLE_SIZE=%llu WHERE ID=%u", USER_TABLE_NAME,
			puser->hd_recycle_size, puser->id);
	}

	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}
#endif
/*
*description: update user information when user's file state change
*input param:data-->buffer storing update information
*output param:
*return: RET_SUCCESS if ok
*/
static error_t user_table_passive_update(sqlite3 *database, void *data)
{	
	error_t errcode = RET_SUCCESS;
	#if 0
	update_info_t *pui = (update_info_t *)data;
	user_info_t user;
	
	if(database == NULL || data == NULL)
	{
		return EINVAL_ARG;
	}
	
	if((errcode = user_table_query_by_id(database, pui->user_id, &user))
			!= RET_SUCCESS)
	{
		log_warning("user with id=%u not exist\n", pui->user_id);
		return errcode;
	}

	if(pui->action == UPDATE_ADD)
	{
		user.hd_used_size  += pui->total_update_size;
		user.hd_video_size += pui->video_size;
		user.hd_audio_size += pui->audio_size;
		user.hd_photo_size += pui->photo_size;
		
		errcode = update_user_capacity(database, &user, pui->action);
	}
	else if(pui->action == UPDATE_DEL_TRASH)
	{
		user.hd_used_size  -= pui->total_update_size;
		user.hd_recycle_size -= pui->total_update_size;
		user.hd_video_size -= pui->video_size;
		user.hd_audio_size -= pui->audio_size;
		user.hd_photo_size -= pui->photo_size;

		errcode = update_user_capacity(database, &user, pui->action);
	}
	else if(pui->action == UPDATE_DEL_NORM)
	{
		user.hd_used_size  -= pui->total_update_size;
		user.hd_video_size -= pui->video_size;
		user.hd_audio_size -= pui->audio_size;
		user.hd_photo_size -= pui->photo_size;

		errcode = update_user_capacity(database, &user, pui->action);
	}
	else if(pui->action == UPDATE_TRASH)
	{
		user.hd_recycle_size += pui->total_update_size;

		errcode = update_user_capacity(database, &user, pui->action);
	}
	else
	{
		user.hd_recycle_size -= pui->total_update_size;

		errcode = update_user_capacity(database, &user, pui->action);
	}
	#endif
	return errcode;	 
}



static error_t user_table_active_update(sqlite3 *database, void *data)
{
	user_update_t *puu = (user_update_t *)data;
	//char sql[SQL_CMD_WRITE_BUF_SIZE];
	error_t errcode = RET_SUCCESS;
	#if 0
	if(database == NULL || data == NULL)
	{
		return EINVAL_ARG;
	}

	if(puu->user_info.id == INVALID_USER_ID)
	{
		return EDB_INVALID_ID;
	}

	if(puu->cmd == UPDATE_USER_PASSWORD)
	{
		user_update_t *puu = (user_update_t *)data;
		char *sql = get_db_write_task_sql_buf();

		sprintf(sql, "UPDATE %s SET PASSWORD='%s' WHERE ID=%u", USER_TABLE_NAME,
			puu->user_info.password, puu->user_info.id);

		errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL);
	}
	else if(puu->cmd == UPDATE_USER_STATE)
	{
		user_update_t *puu = (user_update_t *)data;
		char *sql = get_db_write_task_sql_buf();

		sprintf(sql, "UPDATE %s SET VALID=%d WHERE ID=%u", USER_TABLE_NAME,
			puu->user_info.is_valid, puu->user_info.id);

		errcode =  sqlite3_exec_busy_wait(database, sql, NULL, NULL);
	}
#endif
	return errcode;
}


//register user table operation functions,these are the entries to user table
void register_user_table_ops(void)
{
	memset(&g_user_table, 0, sizeof(g_user_table));
	
	g_user_table.ops.insert          = user_table_insert;
	g_user_table.ops.passive_update  = user_table_passive_update;
	g_user_table.ops.active_update   = user_table_active_update;
	g_user_table.ops.lazy_delete     = user_table_delete;
	g_user_table.ops.query           = user_table_query;
}


//initialize user table
error_t user_table_init(void)
{
	sqlite3 *database;
	error_t errcode = RET_SUCCESS;

	register_user_table_ops();
#if 0
	errcode = sqlite3_open(DATABASE, &database);
	if(errcode != SQLITE_OK)
	{
		log_warning("cannot open database in user_table_init\n");
		return errcode;
	}

	//get max used user id
	if((errcode = sqlite3_exec_busy_wait(database,"SELECT MAX(ID) FROM user_table",
		     get_user_id_callback, &g_user_id)) != SQLITE_OK)
	{
        sqlite3_close(database);
        return errcode;
	}

    sqlite3_close(database);
#endif
	return errcode;
}
