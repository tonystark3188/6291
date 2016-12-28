#include "db_opr.h"
#include "token_manage.h"
#include "bfavfs.h"
#include "base.h"
#include "media_process.h"


static int get_bucket_name(char *path,char *bucket_name)
{
	char *tmp = strchr(path,'/');
	if(tmp != NULL)
	{
		*tmp = 0;
		strcpy(bucket_name,path);
		*tmp = '/';
		DMCLOG_D("bucket_name = %s",bucket_name);
	}else{
		strcpy(bucket_name,path);
	}
	return 0;
}


BucketObject *build_bucket_object(char *path,void *token)
{
	if(token == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	token_dnode_t *token_dnode = (token_dnode_t *)token;
	BucketObject *bObject = (BucketObject *)calloc(1,sizeof(BucketObject));
	assert(bObject != NULL);
	int res = get_bucket_name(path,bObject->bucket_name);
	if(res != 0)
	{
		DMCLOG_E("get bucket name from path error");
		return NULL;
	}

	if(token_dnode->isPublicUser == false)//cur user is nornal user
	{
		if(strcmp(bObject->bucket_name,PUBLIC_PATH))//it is not public path
		{
			S_STRNCPY(bObject->bucket_name,token_dnode->bucket_name,MAX_BUCKET_NAME_LEN);
			if(*path)
			{
				sprintf(bObject->path,"/%s/%s",bObject->bucket_name,path);
			}else{
				sprintf(bObject->path,"/%s",bObject->bucket_name);
			}
			token_dnode->isPublicPath = false;
		}else{
			strcpy(bObject->bucket_name,PUBLIC_BUCKET_NAME);
			if(*(path + strlen(PUBLIC_PATH) + 1))
			{
				sprintf(bObject->path,"/%s/%s",bObject->bucket_name,path + strlen(PUBLIC_PATH) + 1);
			}else{
				sprintf(bObject->path,"/%s",bObject->bucket_name);
			}
			
			DMCLOG_D("bObject->path = %s",bObject->path);
			token_dnode->isPublicPath = true;
		}
	}else{
		S_STRNCPY(bObject->bucket_name,token_dnode->bucket_name,MAX_BUCKET_NAME_LEN);
		if(*path)
		{
			sprintf(bObject->path,"/%s/%s",bObject->bucket_name,path);
		}else{
			sprintf(bObject->path,"/%s",bObject->bucket_name);
		}
		token_dnode->isPublicPath = false;
	}
	return bObject;
}


/**
 * 通过文件路径获取当前目录下的文件列表。
 * param:
 * dirPath 目录路径
 * startIndex 其实文件索引值（从0开始）
 * count 需要从startIndex的文件条数
 * category 获取那些类别文件
 * sortType 排序方式
 */
ERROR_CODE_VFS bfavfs_get_file_list(char* path,v_file_list_t *vlist,void * token)
{
	token_dnode_t *token_dnode = (token_dnode_t *)token;
	if(path != NULL)
	{
		if(*path)
		{
			sprintf(vlist->path,"/%s/%s",token_dnode->bucket_name,path);
		}else{
			sprintf(vlist->path,"/%s",token_dnode->bucket_name);
		}
	}
	
	int res = handle_v_file_table_list_query(token_dnode->bucket_name,vlist);
	if(res != 0)
	{
		DMCLOG_E("query v file list table error");
		return -1;
	}
	return 0;
}

ERROR_CODE_VFS _bfavfs_get_file_list(BucketObject* sObject,v_file_list_t *vlist,void * token)
{
	ENTER_FUNC();
	v_file_query_t v_file_query;
	memset(&v_file_query,0,sizeof(v_file_query_t));
	
	v_file_query.cmd = V_FILE_TABLE_QUERY_LIST;
	if(sObject->file_type == 0)
	{
		S_STRNCPY(vlist->path,sObject->path,MAX_FILE_PATH_LEN);
		S_STRNCPY(vlist->bucket_name,sObject->bucket_name,MAX_BUCKET_NAME_LEN);
		S_STRNCPY(v_file_query.bucket_name,sObject->bucket_name,MAX_BUCKET_NAME_LEN);
	}else{
		token_dnode_t *token_dnode = (token_dnode_t *)token;
		S_STRNCPY(vlist->bucket_name,token_dnode->bucket_name,MAX_BUCKET_NAME_LEN);
		S_STRNCPY(v_file_query.bucket_name,token_dnode->bucket_name,MAX_BUCKET_NAME_LEN);
	}
		
	memcpy(&v_file_query.v_file_list,vlist,sizeof(v_file_list_t));
	int res = _handle_v_file_table_query(&v_file_query);
	if(res != 0)
	{
		DMCLOG_E("query v file list table error");
		EXIT_FUNC();
		return -1;
	}
	vlist->total = v_file_query.v_file_list.total;
	DMCLOG_D("vlist->total = %d",vlist->total);
	EXIT_FUNC();
	return 0;
}



/**
 * open，read，write，seek，close 这几个命令是否需要合并比较好？（如 put或get）
 * 好处，可以统一控制。
 * 坏处，与原有文件读写操作不统一。
 */


/**
 * 以某种方式打开文件
 * param：
 * path：文件路径
 * mode：打开模式
 * “r” 以只读方式打开文件，该文件必须存在。
“r+” 以可读写方式打开文件，该文件必须存在。
”rb+“ 读写打开一个二进制文件，允许读写数据，文件必须存在。
“w” 打开只写文件，若文件存在则文件长度清为0，即该文件内容会消失。若文件不存在则建立该文件。
“w+” 打开可读写文件，若文件存在则文件长度清为零，即该文件内容会消失。若文件不存在则建立该文件。
“a” 以附加的方式打开只写文件。若文件不存在，则会建立该文件，如果文件存在，写入的数据会被加到文件尾，即文件原先的内容会被保留。（EOF符保留）
”a+“ 以附加方式打开可读写的文件。若文件不存在，则会建立该文件，如果文件存在，写入的数据会被加到文件尾后，即文件原先的内容会被保留。 （原来的EOF符不保留）
“wb” 只写打开或新建一个二进制文件；只允许写数据。
“wb+” 读写打开或建立一个二进制文件，允许读和写
“wx” 创建文本文件,只允许写入数据.[C11]
“wbx” 创建一个二进制文件,只允许写入数据.[C11]
”w+x“ 创建一个文本文件,允许读写.[C11]
“wb+x” 创建一个二进制文件,允许读写.[C11]
“w+bx” 和"wb+x"相同[C11]
“rt” 只读打开一个文本文件，只允许读数据
　　“wt” 只写打开或建立一个文本文件，只允许写数据
　　“at” 追加打开一个文本文件，并在文件末尾写数据
　　“rb” 只读打开一个二进制文件，只允许读数据
　　“wb” 只写打开或建立一个二进制文件，只允许写数据
　　“ab” 追加打开一个二进制文件，并在文件末尾写数据
　　“rt+” 读写打开一个文本文件，允许读和写
　　“wt+” 读写打开或建立一个文本文件，允许读写
　　“at+” 读写打开一个文本文件，允许读，或在文件末追加数据
　　“rb+” 读写打开一个二进制文件，允许读和写
　　“ab+” 读写打开一个二进制文件，允许读，或在文件末追加数据
 *
 * token：登录时返回的token
 */
VFILE* bfavfs_fopen(const char * path,const char * mode,void * token)
{
	int res = 0;
	token_dnode_t *token_dnode = (token_dnode_t *)token;
	v_file_info_t v_file_info;
	memset(&v_file_info,0,sizeof(v_file_info_t));
	
	if(path == NULL)
	{
		return NULL;
	}

	char db_path[MAX_FILE_PATH_LEN] = {0};
	if(*path)
	{
		sprintf(db_path,"/%s/%s",token_dnode->bucket_name,path);
	}else{
		sprintf(db_path,"/%s",token_dnode->bucket_name);
	}

	S_STRNCPY(v_file_info.path,db_path,MAX_FILE_PATH_LEN);
	if(strstr(mode,"r"))//download
	{
		res = handle_v_file_table_query(token_dnode->bucket_name,&v_file_info);
		if(res != 0)
		{
			DMCLOG_E("query file table error");
			return NULL;
		}
	}else if(strstr(mode,"w")||strstr(mode,"a"))//upload
	{
		res = handle_v_file_table_query(token_dnode->bucket_name,&v_file_info);
		if(res != 0)
		{
			// 1 :get new path
			char new_path[32] = {0};
			res =  rfsvfs_get_new_file_path(new_path);
			if(res != 0)
			{
				DMCLOG_E("get new file path error");
				return NULL;
			}
			// 2 :insert new path to file table
			DMCLOG_D("real path:%s,bucket_name = %s",new_path,token_dnode->bucket_name);
			S_STRNCPY(v_file_info.path,db_path,MAX_FILE_PATH_LEN);
			S_STRNCPY(v_file_info.real_path,new_path,MAX_REAL_PATH_LEN);
			//v_file_info.type = db_get_mime_type(db_path,strlen(db_path));
			DMCLOG_D("path = %s,real_path = %s",db_path,new_path);
			res = handle_v_file_table_insert(token_dnode->bucket_name,&v_file_info);
			if(res != 0)
			{
				DMCLOG_E("insert file table error");
				return NULL;
			}
		}
	}

	DMCLOG_D("real path:%s",v_file_info.real_path);
	FILE *pf = rfsvfs_fopen(v_file_info.real_path, mode);
	if(pf == NULL)
	{
		DMCLOG_E("fopen %s error",v_file_info.real_path);
		return NULL;
	}

	VFILE *vf = (VFILE *)calloc(1,sizeof(VFILE));
	vf->fp = pf;
	vf->srcPath = (char *)calloc(1,strlen(path) + 1);
	assert(vf->srcPath != NULL);
	strcpy(vf->srcPath,path);

	vf->realPath = (char *)calloc(1,strlen(v_file_info.real_path) + 1);
	assert(vf->realPath != NULL);
	strcpy(vf->realPath,v_file_info.real_path);

	vf->token = token;
	return vf;
}

VFILE* _bfavfs_fopen(BucketObject* sObject,const char * mode,void * token)
{
	int res = 0;
	char real_path[32] = {0};
	v_file_query_t v_file_query;
	memset(&v_file_query,0,sizeof(v_file_query_t));
	S_STRNCPY(v_file_query.bucket_name,sObject->bucket_name,MAX_BUCKET_NAME_LEN);
	v_file_query.cmd = V_FILE_TABLE_QUERY_INFO;
	S_STRNCPY(v_file_query.v_file_info.path,sObject->path,MAX_FILE_PATH_LEN);
	if(strstr(mode,"r"))//download
	{
		res = _handle_v_file_table_query(&v_file_query);
		if(res != 0)
		{
			DMCLOG_E("query file table error");
			return NULL;
		}
		S_STRNCPY(real_path,v_file_query.v_file_info.real_path,MAX_FILE_PATH_LEN);
	}else if(strstr(mode,"w")||strstr(mode,"a"))//upload
	{
		res = _handle_v_file_table_query(&v_file_query);
		if(res != 0)
		{
			// 1 :get new path
			
			res =  rfsvfs_get_new_file_path(real_path);
			if(res != 0)
			{
				DMCLOG_E("get new file path error");
				return NULL;
			}
			// 2 :insert new path to file table
			v_file_insert_t v_file_insert;
			memset(&v_file_insert,0,sizeof(v_file_insert_t));
			v_file_insert.cmd = V_FILE_TABLE_INSERT_INFO;
			S_STRNCPY(v_file_insert.bucket_name,sObject->bucket_name,MAX_BUCKET_NAME_LEN);
			DMCLOG_D("real path:%s,bucket_name = %s",real_path,sObject->bucket_name);
			S_STRNCPY(v_file_insert.v_file_info.path,sObject->path,MAX_FILE_PATH_LEN);
			S_STRNCPY(v_file_insert.v_file_info.real_path,real_path,MAX_REAL_PATH_LEN);
			DMCLOG_D("path = %s,real_path = %s",v_file_insert.v_file_info.path,real_path);
			res = _handle_v_file_table_insert(&v_file_insert);
			if(res != 0)
			{
				DMCLOG_E("insert file table error");
				return NULL;
			}
		}
	}

	DMCLOG_D("real path:%s",real_path);
	FILE *pf = rfsvfs_fopen(real_path, mode);
	if(pf == NULL)
	{
		DMCLOG_E("fopen %s error",real_path);
		return NULL;
	}

	VFILE *vf = (VFILE *)calloc(1,sizeof(VFILE));
	assert(vf != NULL);
	vf->fp = pf;
	vf->srcPath = (char *)calloc(1,strlen(sObject->path) + 1);
	assert(vf->srcPath != NULL);
	strcpy(vf->srcPath,sObject->path);

	vf->realPath = (char *)calloc(1,strlen(real_path) + 1);
	assert(vf->realPath != NULL);
	strcpy(vf->realPath,real_path);

	vf->bobject = (BucketObject *)calloc(1,sizeof(BucketObject));
	assert(vf->bobject != NULL);

	strcpy(vf->bobject,sObject);
	return vf;
}



/**
 * 文件读取方法
 */
size_t bfavfs_fread( void *buffer, size_t size, size_t count, VFILE *vf)
{
	return rfsvfs_fread(buffer,size,count,vf->fp);
}


/**
 * 文件写方法
 */
size_t bfavfs_fwrite(const void* buffer, size_t size, size_t count, VFILE* vf)
{
	return rfsvfs_fwrite( buffer, size,count, vf->fp);
}


/**
 * 文件指针偏移
 * fromwhere（偏移起始位置：文件头0(SEEK_SET)，当前位置1(SEEK_CUR)，文件尾2(SEEK_END)）为基准，偏移offset（指针偏移量）个字节的位置。
 */
int bfavfs_fseek(VFILE *vf, long offset, int fromwhere)
{
	return rfsvfs_fseek(vf->fp,offset,fromwhere);
}

/**
 * 判断文件是否已读完
 * 文件未结束返回0
 * 文件结束返回1
 */
int bfavfs_feof(VFILE* vf)
{
	return rfsvfs_feof(vf->fp);
}


/**
 * 关闭文件指针
 */
int bfavfs_fclose( VFILE *vf,void* token)
{
	int res = 0;
	token_dnode_t *token_dnode = (token_dnode_t *)token;
	struct stat st;
	DMCLOG_D("real_path = %s",vf->realPath);
	if(rfsvfs_fstat(vf->realPath,&st) != 0)
	{
		DMCLOG_E("stat error");
		res = -1;
		goto EXIT;
	}
	char db_path[MAX_FILE_PATH_LEN] = {0};
	if(vf->srcPath != NULL)
	{
		sprintf(db_path,"/%s/%s",token_dnode->bucket_name,vf->srcPath);
	}else{
		sprintf(db_path,"/%s",token_dnode->bucket_name);
	}

	v_file_info_t v_file_info;
	memset(&v_file_info,0,sizeof(v_file_info_t));
	strcpy(v_file_info.path,db_path);
	v_file_info.atime = st.st_atime;
	v_file_info.ctime = st.st_ctime;
	v_file_info.mtime = st.st_mtime;
	v_file_info.size = st.st_size;
	v_file_info.isDir = S_ISDIR(st.st_mode);
	v_file_info.type =  db_get_mime_type(db_path,strlen(db_path));
	if(*vf->uuid)
	{
		S_STRNCPY(v_file_info.uuid,vf->uuid,MAX_FILE_UUID_LEN);
	}else{
		strcpy(v_file_info.uuid,"13141314");
	}
	res = handle_v_file_table_update(token_dnode->bucket_name,V_FILE_TABLE_UPDATE_FILE_INFO,rfsvfs_remove,&v_file_info);
	if(res != 0)
	{
		DMCLOG_E("update file table error");
		res = -1;
		goto EXIT;
	}
EXIT:
	rfsvfs_fclose(vf->fp);
	safe_free(vf->srcPath);
	safe_free(vf->realPath);
	safe_free(vf);
	return res;
}

/**
 * 关闭文件指针
 */
int _bfavfs_fclose( VFILE *vf,void* token)
{
	ENTER_FUNC();
	int res = 0;
	struct stat st;
	DMCLOG_D("real_path = %s",vf->realPath);
	if(rfsvfs_fstat(vf->realPath,&st) != 0)
	{
		DMCLOG_E("stat error");
		res = -1;
		goto EXIT;
	}
	
	v_file_update_t v_file_update;
	memset(&v_file_update,0,sizeof(v_file_update_t));
	S_STRNCPY(v_file_update.v_file_info.path,vf->srcPath,MAX_FILE_PATH_LEN);
	DMCLOG_D("vf->srcPath = %s",vf->srcPath);
	S_STRNCPY(v_file_update.bucket_name,vf->bobject->bucket_name,MAX_BUCKET_NAME_LEN);
	DMCLOG_D("bucket_name = %s",v_file_update.bucket_name);
	
	v_file_update.v_file_info.atime = st.st_atime;
	v_file_update.v_file_info.ctime = st.st_ctime;
	v_file_update.v_file_info.mtime = st.st_mtime;
	v_file_update.v_file_info.size = st.st_size;
	v_file_update.v_file_info.isDir = S_ISDIR(st.st_mode);
	v_file_update.v_file_info.type =  db_get_mime_type(vf->srcPath,strlen(vf->srcPath));
	if(*vf->uuid)
	{
		S_STRNCPY(v_file_update.v_file_info.uuid,vf->uuid,MAX_FILE_UUID_LEN);
	}else{
		strcpy(v_file_update.v_file_info.uuid,"13141314");
	}
	v_file_update.cmd = V_FILE_TABLE_UPDATE_FILE_INFO;
	v_file_update.remove = rfsvfs_remove;
	res = _handle_v_file_table_update(&v_file_update);
	if(res != 0)
	{
		DMCLOG_E("update file table error");
		res = -1;
		goto EXIT;
	}
EXIT:
	rfsvfs_fclose(vf->fp);
	safe_free(vf->srcPath);
	safe_free(vf->realPath);
	safe_free(vf);
	EXIT_FUNC();
	return res;
}


/**
 * 获取文件属性
 */
int bfavfs_fstat(const char *path,struct stat *st,void *token)
{
	ENTER_FUNC();
	token_dnode_t *token_dnode = (token_dnode_t *)token;
	v_file_info_t v_file_info;
	memset(&v_file_info,0,sizeof(v_file_info_t));
	char db_path[MAX_FILE_PATH_LEN] = {0};
	if(*path)
	{
		sprintf(db_path,"/%s/%s",token_dnode->bucket_name,path);
	}else{
		sprintf(db_path,"/%s",token_dnode->bucket_name);
	}
	S_STRNCPY(v_file_info.path,db_path,MAX_FILE_PATH_LEN);
	int res = handle_v_file_table_query(token_dnode->bucket_name,&v_file_info);
	if(res != 0)
	{
		DMCLOG_E("query v file table error");
		return -1;
	}
	
	if(*v_file_info.real_path)
	{
		DMCLOG_D("real_path:%s",v_file_info.real_path);
		res = rfsvfs_fstat(v_file_info.real_path,st);
		if(res != 0)
		{
			DMCLOG_E("stat %s error[%d]",path,errno);
			return -1;
		}
	}else{
		st->st_size = 0;
		st->st_mode = S_IFDIR;
	}
	
	EXIT_FUNC();
	return 0;
}

/**
 * 获取文件属性
 */
int _bfavfs_fstat(BucketObject* sObject,struct stat *st,void *token)
{
	v_file_query_t v_file_query;
	memset(&v_file_query,0,sizeof(v_file_query_t));
	S_STRNCPY(v_file_query.bucket_name,sObject->bucket_name,MAX_BUCKET_NAME_LEN);
	v_file_query.cmd = V_FILE_TABLE_QUERY_INFO;
	S_STRNCPY(v_file_query.v_file_info.path,sObject->path,MAX_FILE_PATH_LEN);
	int res = _handle_v_file_table_query(&v_file_query);
	if(res != 0)
	{
		DMCLOG_E("query v file table error");
		return -1;
	}
	
	if(*v_file_query.v_file_info.real_path)
	{
		DMCLOG_D("real_path:%s",v_file_query.v_file_info.real_path);
		res = rfsvfs_fstat(v_file_query.v_file_info.real_path,st);
		if(res != 0)
		{
			DMCLOG_E("stat %s error[%d]",v_file_query.v_file_info,errno);
			return -1;
		}
	}else{
		st->st_size = 0;
		st->st_mode = S_IFDIR;
	}
	return 0;
}


/**
 *通过UUID 判断文件是否存在
 */
int bfavfs_exist(const char *uuid,void *token)
{
	ENTER_FUNC();
	token_dnode_t *token_dnode = (token_dnode_t *)token;

	int res = handle_file_uuid_exist_query(token_dnode->bucket_name,uuid);
	if(res != 0)
	{
		DMCLOG_E("query v file table by uuid error");
		return -1;
	}
	return 0;
}


/**
 *通过UUID list 判断文件是否存在
 */
int bfavfs_list_exist(struct dl_list *head,void *token)
{
	ENTER_FUNC();
	token_dnode_t *token_dnode = (token_dnode_t *)token;

	int res = handle_uuid_list_exist_query(token_dnode->bucket_name,head);
	if(res != 0)
	{
		DMCLOG_E("query v file table by uuid error");
		return -1;
	}
	return 0;
}

/**
 *通过UUID list 判断文件是否存在
 */
int _bfavfs_exist(BucketObject* sObject,void *token)
{
	v_file_query_t v_file_query;
	memset(&v_file_query,0,sizeof(v_file_query_t));
	S_STRNCPY(v_file_query.bucket_name,sObject->bucket_name,MAX_BUCKET_NAME_LEN);
	if(sObject->head != NULL)
	{
		memcpy(&v_file_query.v_file_list.head,sObject->head,sizeof(struct dl_list));
		v_file_query.cmd = V_FILE_TABLE_QUERY_LIST_BY_UUID;
	}else{
		v_file_query.cmd = V_FILE_TABLE_QUERY_INFO_BY_UUID;
		S_STRNCPY(v_file_query.v_file_info.uuid,sObject->file_uuid,MAX_FILE_UUID_LEN);
	}
	int res = _handle_v_file_table_query(&v_file_query);
	if(res != 0)
	{
		DMCLOG_E("query v file table by uuid error");
		return -1;
	}
	return 0;
}





/**
 * 文件拷贝
 */
int bfavfs_fcopy(char* des_path,const char* src_path,void * token)
{
	if(des_path == NULL||src_path == NULL||token == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	
	token_dnode_t *token_dnode = (token_dnode_t *)token;
	v_file_info_t v_file_info;
	memset(&v_file_info,0,sizeof(v_file_info_t));
	if(*src_path)
	{
		sprintf(v_file_info.path,"/%s/%s",token_dnode->bucket_name,src_path);
	}else{
		sprintf(v_file_info.path,"/%s",token_dnode->bucket_name);
	}

	if(*des_path)
	{
		sprintf(v_file_info.des_path,"/%s/%s",token_dnode->bucket_name,des_path);
	}else{
		sprintf(v_file_info.des_path,"/%s",token_dnode->bucket_name);
	}

	int res = handle_v_file_table_update(token_dnode->bucket_name,V_FILE_TABLE_UPDATE_FILE_COPY,rfsvfs_remove,&v_file_info);
	if(res != 0)
	{
		DMCLOG_E("copy v file table error");
		return -1;
	}
	DMCLOG_D("v_file_info.des_path:%s",v_file_info.des_path);
	return 0;
}

/**
 * 多桶拷贝
 */
int _bfavfs_fcopy(BucketObject* sObject,BucketObject* dObject,void * token)
{
	if(sObject == NULL||dObject == NULL||token == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	
	v_file_update_t v_file_update;
	memset(&v_file_update,0,sizeof(v_file_update_t));
	S_STRNCPY(v_file_update.v_file_info.path,sObject->path,MAX_FILE_PATH_LEN);
	S_STRNCPY(v_file_update.v_file_info.bucket_name,sObject->bucket_name,MAX_BUCKET_NAME_LEN);
	S_STRNCPY(v_file_update.v_des_info.path,dObject->path,MAX_FILE_PATH_LEN);
	S_STRNCPY(v_file_update.v_des_info.bucket_name,dObject->bucket_name,MAX_BUCKET_NAME_LEN);
	
	v_file_update.cmd = V_FILE_TABLE_UPDATE_FILE_COPY;
	v_file_update.remove = rfsvfs_remove;
	int res = _handle_v_file_table_update(&v_file_update);
	if(res != 0)
	{
		DMCLOG_D("copy v file table error,src_path = %s,des_path:%s",v_file_update.v_des_info.path,v_file_update.v_file_info.path);
		return -1;
	}
	DMCLOG_D("src_path = %s,des_path:%s",v_file_update.v_des_info.path,v_file_update.v_file_info.path);
	return 0;
}



/**
 * 文件移动
 */
int bfavfs_fmove(char* des_path,const char* src_path,void * token)
{
	char db_path[MAX_FILE_PATH_LEN] = {0};
	if(des_path == NULL||src_path == NULL||token == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	
	token_dnode_t *token_dnode = (token_dnode_t *)token;
	v_file_info_t v_file_info;
	memset(&v_file_info,0,sizeof(v_file_info_t));
	if(*src_path)
	{
		sprintf(v_file_info.path,"/%s/%s",token_dnode->bucket_name,src_path);
	}else{
		sprintf(v_file_info.path,"/%s",token_dnode->bucket_name);
	}

	if(*des_path)
	{
		sprintf(v_file_info.des_path,"/%s/%s",token_dnode->bucket_name,des_path);
	}else{
		sprintf(v_file_info.des_path,"/%s",token_dnode->bucket_name);
	}
	
	int res = handle_v_file_table_update(token_dnode->bucket_name,V_FILE_TABLE_UPDATE_FILE_MOVE,rfsvfs_remove,&v_file_info);
	if(res != 0)
	{
		DMCLOG_E("move v file table error");
		return -1;
	}
	DMCLOG_D("v_file_info.des_path:%s",v_file_info.des_path);
	return 0;
}

/**
 * 多桶移动
 */
int _bfavfs_fmove(BucketObject* sObject,BucketObject* dObject,void * token)
{
	if(sObject == NULL||dObject == NULL||token == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	
	v_file_update_t v_file_update;
	memset(&v_file_update,0,sizeof(v_file_update_t));
	S_STRNCPY(v_file_update.v_file_info.path,sObject->path,MAX_FILE_PATH_LEN);
	S_STRNCPY(v_file_update.v_file_info.bucket_name,sObject->bucket_name,MAX_BUCKET_NAME_LEN);
	S_STRNCPY(v_file_update.v_des_info.path,dObject->path,MAX_FILE_PATH_LEN);
	S_STRNCPY(v_file_update.v_des_info.bucket_name,dObject->bucket_name,MAX_BUCKET_NAME_LEN);

	v_file_update.cmd = V_FILE_TABLE_UPDATE_FILE_MOVE;
	v_file_update.remove = rfsvfs_remove;
	int res = _handle_v_file_table_update(&v_file_update);
	if(res != 0)
	{
		DMCLOG_D("move v file table error,src_path = %s,des_path:%s",v_file_update.v_des_info.path,v_file_update.v_file_info.path);
		return -1;
	}
	DMCLOG_D("src_path = %s,des_path:%s",v_file_update.v_des_info.path,v_file_update.v_file_info.path);
	return 0;
}


/**
 * 文件重命名
 */
int bfavfs_frename(const char* src_path,char* des_path,void * token)
{
	if(des_path == NULL||src_path == NULL||token == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	
	token_dnode_t *token_dnode = (token_dnode_t *)token;
	v_file_info_t v_file_info;
	memset(&v_file_info,0,sizeof(v_file_info_t));
	if(*src_path)
	{
		sprintf(v_file_info.path,"/%s/%s",token_dnode->bucket_name,src_path);
	}else{
		sprintf(v_file_info.path,"/%s",token_dnode->bucket_name);
	}

	if(*des_path)
	{
		sprintf(v_file_info.des_path,"/%s/%s",token_dnode->bucket_name,des_path);
	}else{
		sprintf(v_file_info.des_path,"/%s",token_dnode->bucket_name);
	}
	//v_file_info.type = db_get_mime_type(v_file_info.des_path,strlen(v_file_info.des_path));
	int res = handle_v_file_table_update(token_dnode->bucket_name,V_FILE_TABLE_UPDATE_FILE_RENAME,rfsvfs_remove,&v_file_info);
	if(res != 0)
	{
		DMCLOG_E("rename v file table error");
		return -1;
	}
	DMCLOG_D("v_file_info.des_path:%s",v_file_info.des_path);
	return 0;
}

/**
 * 文件重命名
 */
int _bfavfs_frename(BucketObject* sObject,BucketObject* dObject,void * token)
{
	if(sObject == NULL||dObject == NULL||token == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	
	v_file_update_t v_file_update;
	memset(&v_file_update,0,sizeof(v_file_update_t));
	S_STRNCPY(v_file_update.v_file_info.path,sObject->path,MAX_FILE_PATH_LEN);
	S_STRNCPY(v_file_update.v_file_info.bucket_name,sObject->bucket_name,MAX_BUCKET_NAME_LEN);
	S_STRNCPY(v_file_update.v_des_info.path,dObject->path,MAX_FILE_PATH_LEN);
	S_STRNCPY(v_file_update.v_des_info.bucket_name,dObject->bucket_name,MAX_BUCKET_NAME_LEN);
	S_STRNCPY(v_file_update.bucket_name,dObject->bucket_name,MAX_BUCKET_NAME_LEN);
	v_file_update.cmd = V_FILE_TABLE_UPDATE_FILE_RENAME;
	v_file_update.remove = rfsvfs_remove;
	int res = _handle_v_file_table_update(&v_file_update);
	if(res != 0)
	{
		DMCLOG_D("rename v file table error,src_path = %s,des_path:%s",v_file_update.v_des_info.path,v_file_update.v_file_info.path);
		return -1;
	}
	DMCLOG_D("src_path = %s,des_path:%s",v_file_update.v_des_info.path,v_file_update.v_file_info.path);
	return 0;
}



/**
 * 删除文件
 
 */
int bfavfs_remove(char* path,void * token)
{
	if(path == NULL || token == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	token_dnode_t *token_dnode = (token_dnode_t *)token;
	v_file_info_t v_file_info;
	memset(&v_file_info,0,sizeof(v_file_info_t));
	if(*path)
	{
		sprintf(v_file_info.path,"/%s/%s",token_dnode->bucket_name,path);
	}else{
		sprintf(v_file_info.path,"/%s",token_dnode->bucket_name);
	}
	
	int res = handle_v_file_table_delete(token_dnode->bucket_name,V_FILE_TABLE_DELETE_INFO,rfsvfs_remove,&v_file_info);
	if(res != 0)
	{
		DMCLOG_E("delete v file table error");
		return -1;
	}
	DMCLOG_D("path:%s",v_file_info.path);
	return 0;
}

/**
 * 删除目录下类型文件
 
 */
int _bfavfs_remove(BucketObject* sObject,void * token)
{
	if(sObject == NULL || token == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	v_file_delete_t v_file_delete;
	memset(&v_file_delete,0,sizeof(v_file_delete_t));
	S_STRNCPY(v_file_delete.bucket_name,sObject->bucket_name,MAX_BUCKET_NAME_LEN);
	S_STRNCPY(v_file_delete.v_file_info.path,sObject->path,MAX_FILE_PATH_LEN);
	if(sObject->file_type > 0)
	{
		v_file_delete.cmd = V_FILE_TABLE_DELETE_TYPE_BY_PATH;
		v_file_delete.v_file_info.type = sObject->file_type;
	}else{
		v_file_delete.cmd = V_FILE_TABLE_DELETE_INFO;
	}
	
	v_file_delete.remove = rfsvfs_remove;
	
	int res = _handle_v_file_table_delete(&v_file_delete);
	if(res != 0)
	{
		DMCLOG_E("delete v file table error");
		return -1;
	}
	DMCLOG_D("path:%s",v_file_delete.v_file_info.path);
	return 0;
}

/**
 * 删除目录下类型文件
 
 */
int bfavfs_remove_type(char* path,int file_type,void * token)
{
	if(path == NULL || token == NULL||file_type < 0)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	token_dnode_t *token_dnode = (token_dnode_t *)token;
	v_file_info_t v_file_info;
	memset(&v_file_info,0,sizeof(v_file_info_t));
	if(*path)
	{
		sprintf(v_file_info.path,"/%s/%s",token_dnode->bucket_name,path);
	}else{
		sprintf(v_file_info.path,"/%s",token_dnode->bucket_name);
	}

	v_file_info.type = file_type;
	
	int res = handle_v_file_table_delete(token_dnode->bucket_name,V_FILE_TABLE_DELETE_TYPE_BY_PATH,rfsvfs_remove,&v_file_info);
	if(res != 0)
	{
		DMCLOG_E("delete v file table error");
		return -1;
	}
	DMCLOG_D("path:%s",v_file_info.path);
	return 0;
}




/**
 * 创建文件夹
 
 */
int bfavfs_mkdir(char* path,void * token)
{
	int res = 0;
	token_dnode_t *token_dnode = (token_dnode_t *)token;
	v_file_info_t v_file_info;
	memset(&v_file_info,0,sizeof(v_file_info_t));
	char db_path[MAX_FILE_PATH_LEN] = {0};
	if(*path)
	{
		sprintf(db_path,"/%s/%s",token_dnode->bucket_name,path);
	}else{
		sprintf(db_path,"/%s",token_dnode->bucket_name);
	}
	S_STRNCPY(v_file_info.path,db_path,MAX_FILE_PATH_LEN);
	v_file_info.isDir = 1;
	DMCLOG_D("path = %s",path);
	res = handle_v_file_table_insert(token_dnode->bucket_name,&v_file_info);
	if(res != 0)
	{
		DMCLOG_E("mkdir file table error");
		return -1;
	}
	return 0;
}

/**
 * 创建文件夹
 
 */
int _bfavfs_mkdir(BucketObject* sObject,void * token)
{
	int res = 0;
	v_file_insert_t v_file_insert;
	memset(&v_file_insert,0,sizeof(v_file_insert_t));
	v_file_insert.cmd = V_FILE_TABLE_INSERT_INFO;
	S_STRNCPY(v_file_insert.bucket_name,sObject->bucket_name,MAX_BUCKET_NAME_LEN);
	S_STRNCPY(v_file_insert.v_file_info.path,sObject->path,MAX_FILE_PATH_LEN);
	v_file_insert.v_file_info.isDir = 1;
	res = _handle_v_file_table_insert(&v_file_insert);
	if(res != 0)
	{
		DMCLOG_E("mkdir file table error");
		return NULL;
	}
	return 0;
}



/**
 * 设置文件属性
 */
int bfavfs_fsetattr(const char *path,void *token)
{
	ENTER_FUNC();
	token_dnode_t *token_dnode = (token_dnode_t *)token;
	v_file_info_t v_file_info;
	memset(&v_file_info,0,sizeof(v_file_info_t));
	char real_path[32] = {0};
	char db_path[MAX_FILE_PATH_LEN] = {0};
	if(*path)
	{
		sprintf(db_path,"/%s/%s",token_dnode->bucket_name,path);
	}else{
		sprintf(db_path,"/%s",token_dnode->bucket_name);
	}
	S_STRNCPY(v_file_info.path,db_path,MAX_FILE_PATH_LEN);
	int res = handle_v_file_table_query(token_dnode->bucket_name,&v_file_info);
	if(res != 0)
	{
		DMCLOG_E("query v file table error");
		return -1;
	}
	
	if(*v_file_info.real_path)
	{
		DMCLOG_D("real_path:%s",v_file_info.real_path);
		//TODO for xwm
		add_media_to_list(v_file_info.real_path,v_file_info.type,media_prc_thpool_cb,0);
	}else{
		DMCLOG_E("the %s is not exist",path);
		return -1;
	}
	
	EXIT_FUNC();
	return 0;
}

/**
 * 设置文件属性
 */
int _bfavfs_fsetattr(BucketObject* sObject,void *token)
{
	ENTER_FUNC();
	v_file_query_t v_file_query;
	memset(&v_file_query,0,sizeof(v_file_query_t));
	S_STRNCPY(v_file_query.bucket_name,sObject->bucket_name,MAX_BUCKET_NAME_LEN);
	v_file_query.cmd = V_FILE_TABLE_QUERY_INFO;
	S_STRNCPY(v_file_query.v_file_info.path,sObject->path,MAX_FILE_PATH_LEN);
	int res = _handle_v_file_table_query(&v_file_query);
	if(res != 0)
	{
		DMCLOG_E("query v file table error");
		return -1;
	}
	
	if(*v_file_query.v_file_info.real_path)
	{
		DMCLOG_D("real_path:%s",v_file_query.v_file_info.real_path);
		//TODO for xwm
		#if 0
		char tmp_path[1024];
		memset(tmp_path,0,sizeof(tmp_path));
		strcpy(tmp_path,"/tmp/mnt/SD-disk-1");
		strcat(tmp_path,v_file_info.real_path);
		DMCLOG_D("v_file_info.type:%d",v_file_info.type);
		add_media_to_list(tmp_path,v_file_info.type,media_prc_thpool_cb,0);
		#endif
	}else{
		DMCLOG_E("the %s is not exist",v_file_query.v_file_info.real_path);
		return -1;
	}
	EXIT_FUNC();
	return 0;
}





