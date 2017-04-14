/*
 * =============================================================================
 *
 *       Filename:  search_task.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2016/9/9 16:45:38
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Neo, Oliver (), 515296288jf@163.com
 *   Organization:  
 *
 * =============================================================================
 */
#include <ctype.h>
#include <sys/sysinfo.h>
#include<ctype.h>
#include "base.h"
#include "task/task_base.h"
#include "disk_manage.h"
#include "router_inotify.h"
#include "list.h"
#include "search_task.h"
#include "file_json.h"


search_manage_list_t searchManageList;
extern int	exit_flag;
extern int router_para_listen_port;


char *str_letter_tolower(char *src){
	int i = 0;
	char *dest = NULL;
	if(src == NULL){
		return NULL;
	}

	dest = (char *)calloc(1, strlen(src)+1);
	if(dest == NULL){
		return NULL;
	}

	for(i = 0; i < strlen(src); i++){
		dest[i] = tolower(src[i]);
	}

	return dest;
}

char *str_letter_toupper(char *src){
	int i = 0;
	char *dest = NULL;
	if(src == NULL){
		return NULL;
	}

	dest = (char *)calloc(1, strlen(src)+1);
	if(dest == NULL){
		return NULL;
	}

	for(i = 0; i < strlen(src); i++){
		dest[i] = toupper(src[i]);
	}

	return dest;
}


char *notify_search_manage_comb_send_buf(search_manage_info_t *p_search_manage_info)
{
    char *send_buf = NULL;
    int res_sz = 0;
    int cmd = 114;//FN_FILE_SEARCH_INOTIFY
    int seq = 0;
    int error = 0;
	file_info_t *item;

	DMCLOG_D("searchStatus: %d", p_search_manage_info->searchStatus);
    JObj* response_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj *response_data_array = JSON_NEW_ARRAY();
    JObj* para_info = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(para_info, "status",JSON_NEW_OBJECT(p_search_manage_info->searchStatus,int));
	JSON_ADD_OBJECT(para_info, "seq",JSON_NEW_OBJECT(p_search_manage_info->seq,int));
	if(p_search_manage_info->searchStatus == 1)
    {    	
		DMCLOG_D("statusCode: %d, curCount: %d, list_nfiles: %d", p_search_manage_info->statusCode, p_search_manage_info->curNfiles, p_search_manage_info->listNfiles);
    	JSON_ADD_OBJECT(para_info, "statusCode",JSON_NEW_OBJECT(p_search_manage_info->statusCode,int));
    	JSON_ADD_OBJECT(para_info, "curCount",JSON_NEW_OBJECT(p_search_manage_info->curNfiles,int));
		JSON_ADD_OBJECT(para_info, "listCount",JSON_NEW_OBJECT(p_search_manage_info->listNfiles,int));
	
		JObj *file_info_array = JSON_NEW_ARRAY();
		dl_list_for_each(item, &p_search_manage_info->head, file_info_t, next)
		{
			JObj* file_info_json = JSON_NEW_EMPTY_OBJECT();
			if(item->path != NULL){
				//DMCLOG_D("file_path: %s", item->path);
				JSON_ADD_OBJECT(file_info_json, "filePath",JSON_NEW_OBJECT(item->path,string));
			}
			else{
				//DMCLOG_D("file_path is null");
				JSON_ADD_OBJECT(file_info_json, "filePath",JSON_NEW_OBJECT("",string));
			}
			//DMCLOG_D("isFolder:%d", item->isFolder);
			JSON_ADD_OBJECT(file_info_json, "isFolder",JSON_NEW_OBJECT(item->isFolder, int));
			JSON_ARRAY_ADD_OBJECT (file_info_array, file_info_json);
		}
		JSON_ADD_OBJECT(para_info, "fileList", file_info_array);
	
	}
	
    JSON_ARRAY_ADD_OBJECT (response_data_array,para_info);
    JSON_ADD_OBJECT(response_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(cmd,int));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_search_manage_info->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(error,int));
    JSON_ADD_OBJECT(response_json, "header", header_json);
    char *response_str = JSON_TO_STRING(response_json);
    if(response_str == NULL)
    {
        JSON_PUT_OBJECT(response_json);
        return NULL;
    }
    res_sz = strlen(response_str);
    send_buf = (char*)calloc(1,res_sz + 1);
    if(send_buf == NULL)
    {
        JSON_PUT_OBJECT(response_json);
        return NULL;
    }
    strcpy(send_buf,response_str);
    DMCLOG_D("send_buf = %s",send_buf);
    JSON_PUT_OBJECT(response_json);
    return send_buf;
}


int notify_search_manage_parse_recv_buf(search_manage_info_t *p_search_manage_info, char *recv_buf)
{
	int res = 0;
	int status = 0;
	JObj *r_json = JSON_PARSE(recv_buf);
    if(r_json == NULL)
    {
        DMCLOG_D("access NULL");
        res = -1;
        goto exit;
    }
    if(is_error(r_json))
    {
        DMCLOG_D("### error:post data is not a json string");
        res = -1;
        goto exit;
    }

	JObj *data_json = JSON_GET_OBJECT(r_json,"data");
    if(data_json == NULL)
    {
        DMCLOG_D("access NULL");
        res = -1;
        goto exit;
    }
    JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
    if(para_json == NULL)
    {
        DMCLOG_D("access NULL");
        res = -1;
        goto exit;
    }

	JObj *status_json = JSON_GET_OBJECT(para_json,"status");
    if(status_json  == NULL)
    {
        DMCLOG_D("access NULL");
        goto exit;
    }
    status = JSON_GET_OBJECT_VALUE(status_json,int);

	if(status == 1){
		JObj *statusCode_json = JSON_GET_OBJECT(para_json,"statusCode");
	    if(statusCode_json  == NULL)
	    {
	        DMCLOG_D("access NULL");
	        goto exit;
	    }
	    p_search_manage_info->recvStatusCode = JSON_GET_OBJECT_VALUE(statusCode_json,int);
	}

exit:
	if(r_json != NULL)
        JSON_PUT_OBJECT(r_json);
    return res;
}

int search_manage_handle_inotify(search_manage_info_t *p_search_manage_info)
{
    int enRet;
    int client_fd;
    char *send_buf = NULL;
	char *recv_buf = NULL;
	unsigned int time_out = 1000;

    send_buf = notify_search_manage_comb_send_buf(p_search_manage_info);
    if(send_buf == NULL){
        enRet = -1;
        goto exit;;
    }
    enRet = DM_UdpSend(p_search_manage_info->client_fd, send_buf, strlen(send_buf), &p_search_manage_info->clientAddr);
    if(enRet < 0){
        DMCLOG_D("sendto fail,errno = %d",errno);
		enRet = -1;
        goto exit;
    }
	
	enRet = DM_UdpReceive(p_search_manage_info->client_fd, &recv_buf, &time_out,&p_search_manage_info->clientAddr);
	if(enRet < 0){
        DMCLOG_D("receive fail,errno = %d",errno);
		enRet = -1;
        goto exit;
    }

	enRet = notify_search_manage_parse_recv_buf(p_search_manage_info, recv_buf);
	if(enRet < 0){
        DMCLOG_D("parse recv buf fail,errno = %d",errno);
		enRet = -1;
        goto exit;
    }
		
exit:
    safe_free(send_buf);
	safe_free(recv_buf);
    return enRet;
}



int search_file_list_read(search_manage_info_t *p_search_manage_info)
{
	//ENTER_FUNC();
	int ret = 0;
	file_info_t *item, *n;
	int file_list_len = 0, send_len = 0;
	if(p_search_manage_info == NULL){
		DMCLOG_D("p_search_manage_info null");
		ret = -1;
		goto EXIT;
	}

	file_list_len = dl_list_len(&p_search_manage_info->head);
	if(file_list_len <= 0){
		goto EXIT;
	}
	if(file_list_len >= SEND_LIST_INFO_MAX){
		send_len = SEND_LIST_INFO_MAX;
	}
	else{
		send_len = file_list_len;
	}

	p_search_manage_info->client_fd = DM_UdpClientInit(PF_INET, p_search_manage_info->port, SOCK_DGRAM,p_search_manage_info->ip,&p_search_manage_info->clientAddr);
    if(p_search_manage_info->client_fd <= 0){
		ret = -1;
		goto EXIT;
    }

	if(p_search_manage_info->statusCode == p_search_manage_info->recvStatusCode)
		p_search_manage_info->statusCode++;
	
	ret = search_manage_handle_inotify(p_search_manage_info);
	if(ret != 0){
		ret = -1;
		goto EXIT;
	}	
	
	DMCLOG_D("pInfo->statusCode: %d, pInfo->recvStatusCode: %d", p_search_manage_info->statusCode, p_search_manage_info->recvStatusCode);
	if(p_search_manage_info->statusCode == p_search_manage_info->recvStatusCode){
		DMCLOG_D("response success");
		int i = 0;
		dl_list_for_each_safe(item, n, &p_search_manage_info->head, file_info_t, next){
			i++;
			free_db_fd(&item);
			dl_list_del(&item->next);
			if(i >= file_list_len){
				break;
			}
		}
	}
	else{
		DMCLOG_D("response fail");
	}
	
	#if 0
	DMCLOG_D("curNfiles: %u, listNfiles: %u", p_search_manage_info->curNfiles, p_search_manage_info->listNfiles);
	dl_list_for_each(item, &p_search_manage_info->head, file_info_t, next){
		DMCLOG_D("isFolder: %d", item->isFolder);
		if(item->path != NULL)
			DMCLOG_D("path: %s, isFolder: %d", item->path, item->isFolder);
	}

	dl_list_for_each_safe(item, n, &p_search_manage_info->head, file_info_t, next){
		free_db_fd(&item);
		dl_list_del(&item->next);
	}
	#endif
	
EXIT:
	DM_DomainClientDeinit(p_search_manage_info->client_fd);
	//EXIT_FUNC();
	return ret;
}



static struct file_dnode **search_one_dir(file_search_info_t *pInfo, const char *path, unsigned *nfiles_dir, file_list_t *plist)
{
	struct file_dnode *dn, *cur, **dnp;
	struct dirent *entry;
	DIR *dir;
	int ret = 0;
	file_info_t *item,*n;
	unsigned i = 0;//, nfiles;
	char *fullname = NULL;
	char *name_tolower = NULL;
	char *search_str_tolower = NULL;
	struct  sysinfo info; 
	int32_t cur_time;
	struct stat	statbuf;
	int32_t fun_list_flag = 0;
	dir = warn_opendir(path);
	if (dir == NULL) {
		return NULL;	/* could not open the dir */
	}
	dn = NULL;
	*nfiles_dir = 0;
	
	while ((entry = readdir(dir)) != NULL && (!(*pInfo->flags & FLAG_CLOSED))) {
		//DMCLOG_D("ent->d_name: %s", entry->d_name);
			if(strcmp(entry->d_name, ".") == 0
               || strcmp(entry->d_name, "..") == 0){
               	//DMCLOG_D("out1");
                continue;
            }
			
			if (entry->d_name[0] == '.') {
				//DMCLOG_D("out2");
				continue;
			}

			fullname = concat_path_file(path, entry->d_name);
			if(fullname == NULL){
				DMCLOG_E("format fullname error!");
				continue;
			}
	
			name_tolower = str_letter_tolower(entry->d_name);
			if(name_tolower == NULL){
				DMCLOG_E("format name_tolower error!");
				continue;
			}

			search_str_tolower = str_letter_tolower(pInfo->search_string);
			if(search_str_tolower == NULL){
				DMCLOG_E("format search_str_tolower error!");
				continue;
			}
		
			sysinfo(&info); 
			cur_time = info.uptime;			
			
			if(entry->d_type == 8)  // d_type：8-文件，4-目录
			{
				if(NULL != strstr(name_tolower, search_str_tolower)){
					DMCLOG_D("file match: %s\n", entry->d_name);
					
					file_info_t *file_info = (file_info_t *)calloc(1,sizeof(file_info_t));
					if(file_info == NULL){
						continue;
					}

					fun_list_flag = get_func_list_flag();
					if((fun_list_flag & (1 << 12)) && dm_get_attr_hide(fullname)){
						file_info->attr = true;
					}
					else{
						file_info->attr = false;
					}

					file_info->path = (char *)calloc(1, strlen(fullname)-strlen(DOCUMENT_ROOT)+1);
					if(file_info->path == NULL){
						safe_free(file_info);
						continue;
					}
					memcpy(file_info->path, fullname+strlen(DOCUMENT_ROOT)+1, strlen(fullname)-strlen(DOCUMENT_ROOT));
					file_info->name = strdup(entry->d_name);
					file_info->isFolder = 0;
					
					if(my_stat(fullname, &statbuf) == 0){
						file_info->file_size = statbuf.st_size;
						file_info->modify_time = statbuf.st_ctime;
						DMCLOG_D("file_size: %lld, modify_time:%d", file_info->file_size, file_info->modify_time);
					}
					
					dl_list_add_tail(&plist->head, &file_info->next);
					pInfo->cur_nfiles++;
					//pInfo->list_nfiles++;
					//pInfo->record_time = cur_time;

					#ifdef SEARCH_TASK_MANAGE
					file_info_t *search_file_info = (file_info_t *)calloc(1,sizeof(file_info_t));
					search_file_info->isFolder = 0;
					search_file_info->path = strdup(fullname);
					
					search_manage_info_t *search_manage_item;
					pthread_mutex_lock(&searchManageList.mutex);	
					dl_list_for_each(search_manage_item, &(searchManageList.head), search_manage_info_t, next)
					{
						if(search_manage_item->seq == pInfo->seq){
							search_manage_item->curNfiles++;
							search_manage_item->listNfiles++;
							dl_list_add_tail(&search_manage_item->head, &search_file_info->next);
							break;
						}
					}
					pthread_mutex_unlock(&searchManageList.mutex);
					#endif
				}
				else{
					//DMCLOG_D("file no match\n");
				}
			}
			else
			{
				if(NULL != strstr(name_tolower, search_str_tolower)){
					DMCLOG_D("dir match: %s\n", entry->d_name);
										
					file_info_t *file_info = (file_info_t *)calloc(1,sizeof(file_info_t));
					if(file_info == NULL){
						continue;
					}

					fun_list_flag = get_func_list_flag();
					if((fun_list_flag & (1 << 12)) && dm_get_attr_hide(fullname)){
						file_info->attr = true;
					}
					else{
						file_info->attr = false;
					}

					file_info->path = (char *)calloc(1, strlen(fullname)-strlen(DOCUMENT_ROOT)+1);
					if(file_info->path == NULL){
						safe_free(file_info);
						continue;
					}
					memcpy(file_info->path, fullname+strlen(DOCUMENT_ROOT)+1, strlen(fullname)-strlen(DOCUMENT_ROOT));
					file_info->name = strdup(entry->d_name);
					file_info->isFolder = 1;
					
					if(my_stat(fullname, &statbuf) == 0){
						file_info->file_size = statbuf.st_size;
						file_info->modify_time = statbuf.st_ctime;
						DMCLOG_D("file_size: %lld, modify_time:%d", file_info->file_size, file_info->modify_time);
					}
					
					dl_list_add_tail(&plist->head, &file_info->next);
					pInfo->cur_nfiles++;
					//pInfo->list_nfiles++;
					//pInfo->record_time = cur_time;

					#ifdef SEARCH_TASK_MANAGE
					file_info_t *search_file_info = (file_info_t *)calloc(1,sizeof(file_info_t));
					search_file_info->isFolder = 0;
					search_file_info->path = strdup(fullname);
					
					search_manage_info_t *search_manage_item;
					pthread_mutex_lock(&searchManageList.mutex);	
					dl_list_for_each(search_manage_item, &(searchManageList.head), search_manage_info_t, next)
					{
						if(search_manage_item->seq == pInfo->seq){
							search_manage_item->curNfiles++;
							search_manage_item->listNfiles++;
							dl_list_add_tail(&search_manage_item->head, &search_file_info->next);
							break;
						}
					}
					pthread_mutex_unlock(&searchManageList.mutex);
					#endif
				}
				else{
					//DMCLOG_D("dir no match\n");
				}
				
				cur = xzalloc(sizeof(*cur));
				cur->fullname = strdup(fullname);
				cur->fname_allocated = 1;
				cur->dn_next = dn;
				dn = cur;
				(*nfiles_dir)++;
			}
			safe_free(fullname);
			safe_free(name_tolower);
			safe_free(search_str_tolower);

			pInfo->list_nfiles = dl_list_len(&plist->head);
			#ifdef SEARCH_TASK_TCP			
			if((pInfo->list_nfiles >= SEND_LIST_INFO_MIN) || ((cur_time - pInfo->record_time) >= SEND_LIST_INFO_TIME)){				
				DMCLOG_D("list_nfiles: %d, cur_time:%d, pInfo->record_time: %d",pInfo->list_nfiles, cur_time, pInfo->record_time);
				pInfo->record_time = cur_time; 
				if(pInfo->list_nfiles >= SEND_LIST_INFO_MAX){
					pInfo->send_nfiles = SEND_LIST_INFO_MAX;
				}
				else{
					pInfo->send_nfiles = pInfo->list_nfiles;
				}
				
				pInfo->statusCode++;
				ret = search_handle_tcp_inotify(pInfo, plist);
				if(ret != 0){
					DMCLOG_D("search_handle_tcp_inotify error");
				}

				if(pInfo->send_nfiles > 0){
					i = 0;
					dl_list_for_each_safe(item, n, &plist->head, file_info_t, next)
					{
						i++;
						free_db_fd(&item);
						dl_list_del(&item->next);
						if(i >= pInfo->send_nfiles){
							break;
						}
					}
				}				
			}
			#endif
			
			#if defined(SEARCH_TASK_UDP) || defined(SEARCH_TASK_MANAGE) 
			if((pInfo->list_nfiles >= 5) || ((pInfo->list_nfiles >= 1) && ((cur_time - pInfo->record_time) >= 5))){
				DMCLOG_D("list_nfiles: %d, cur_time:%d, pInfo->record_time: %d",pInfo->list_nfiles, cur_time, pInfo->record_time);
				#ifdef SEARCH_TASK_UDP
				ret = search_handle_udp_inotify(pInfo, plist);
				if(ret < 0){
					DMCLOG_D("search_handle_udp_inotify error");
				}
				#endif
				dl_list_for_each_safe(item, n, &plist->head, file_info_t, next)
				{
					free_db_fd(&item);
					dl_list_del(&item->next);
				}
				pInfo->list_nfiles = 0;
			}
			#endif
	}
EXIT:
	closedir(dir);
	if(*nfiles_dir > 0){
		dnp = dnalloc(*nfiles_dir);
		for (i = 0; /* i < nfiles - detected via !dn below */; i++) {
			dnp[i] = dn;	/* save pointer to node in array */
			dn = dn->dn_next;
			if (!dn)
				break;
		}
	    return dnp;
	}
	else{
		return NULL; 
	}
}


int search_file_traversal(file_search_info_t *pInfo, struct file_dnode **dn, file_list_t *plist)
{
	struct dirent *ent = NULL;
	DIR *pDir;
	struct file_dnode **subdnp;
	unsigned nfiles_dir = 0;	
	
	if(*pInfo->flags & FLAG_CLOSED) 
	{
		DMCLOG_E("quit the del task");
		return 0;
	}
	
	for (; (*dn)&&(!(*pInfo->flags & FLAG_CLOSED)); dn++) {
		if(*pInfo->flags & FLAG_CLOSED){
			DMCLOG_D("socket close");
			break;
		}
		subdnp = search_one_dir(pInfo, (*dn)->fullname, &nfiles_dir, plist);
		usleep(10);
		if(nfiles_dir > 0&&pInfo->currentOnly == 0){
			if(search_file_traversal(pInfo, subdnp, plist) != 0){
				DMCLOG_D("search %s error", (*dn)->fullname);
			}
			dfree(subdnp);
		}
	}
	return 0;
}


int check_search_path(_In_ char *s_search_path, _Out_ char **d_search_path)
{	
	int ret = 0;
	char *lc = NULL;
	int len = 0;
	struct stat statbuf;
	char *p_path = NULL;
	if(s_search_path == NULL){
		ret = -1;
		goto EXIT;
	}

    lc = get_last_char(s_search_path, '/');
	if(lc == NULL){
		len = strlen(s_search_path);
	}else{
		len = strlen(s_search_path)-1;
	}

	p_path = (char *)calloc(1, len+1);
	if(p_path == NULL){
		ret = -1;
		goto EXIT;
	}
	memcpy(p_path, s_search_path, len);
	
	if(access(p_path,F_OK) != 0){
		ret = -1;
		safe_free(p_path);
		goto EXIT;
	}

	if(lstat(p_path, &statbuf) < 0){
        ret = -1;
		safe_free(p_path);
		goto EXIT;
    }

	if(S_ISREG(statbuf.st_mode)){
		ret = -1;
		safe_free(p_path);
		goto EXIT;
	}

	DMCLOG_D("p_path: %s", p_path);
	*d_search_path = p_path;
EXIT:
	return ret;
}


int search_handle_tcp_inotify(file_search_info_t *pInfo, file_list_t *plist)
{
	if(pInfo->c == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	
	char *buf = dm_file_search_list_inotify(pInfo, plist);
	if(buf == NULL)
	{
		DMCLOG_E("alloc error");
		return -1;
	}
	read_stream_comb(&pInfo->c->loc,buf);
	safe_free(buf);
	return 0;
}


void search_inotify_func(void *self)
{
    ENTER_FUNC();
    file_search_info_t *pInfo = (file_search_info_t *)self;
    char *search_path = pInfo->search_path;
    const char *last_search_path = NULL;
    int res = 0;
	int error = 0;
	int i = 0;
#if defined(SEARCH_TASK_UDP) || defined(SEARCH_TASK_MANAGE) 
	char buf[1024];
    memset(buf,0,1024);
#endif

	res = _get_info_from_dev_list(pInfo->session,pInfo->ip,&(pInfo->port));
    if(res < 0)
    {
    	DMCLOG_E("_get_info_from_dev_list error");
		if(router_para_listen_port == 0)
			pInfo->port = 13101;
		else
			pInfo->port = router_para_listen_port;
    }

#ifdef SEARCH_TASK_UDP
    pInfo->client_fd = DM_UdpClientInit(PF_INET, pInfo->port, SOCK_DGRAM,pInfo->ip,&pInfo->clientAddr);
    if(pInfo->client_fd <= 0)
    {
        error = ERROR_FILE_SEARCH;
		pInfo->status = -1;
        EXIT_FUNC();
		goto EXIT;
    }
#endif

	DMCLOG_D("search_path: %s, search_string: %s", search_path, pInfo->search_string);
	if(check_search_path(search_path, &last_search_path) < 0)
	{
		error = ERROR_FILE_SEARCH;
		pInfo->status = -1;
		goto EXIT;
	}
	
	DMCLOG_D("last_search_path: %s, search_string: %s", last_search_path, pInfo->search_string);
	pInfo->status = 1;

#ifdef SEARCH_TASK_MANAGE
	search_manage_info_t *search_manage_item;
	search_manage_info_t *search_manage_info = (search_manage_info_t *)calloc(1,sizeof(search_manage_info_t));
	strcpy(search_manage_info->ip, pInfo->ip);
	search_manage_info->port = pInfo->port;
	search_manage_info->seq = pInfo->seq;
	search_manage_info->curNfiles = 0;
	search_manage_info->listNfiles = 0;
	search_manage_info->searchStatus = 1;
	dl_list_init(&search_manage_info->head);
	pthread_mutex_lock(&searchManageList.mutex);
	dl_list_add_tail(&searchManageList.head, &search_manage_info->next);
	pthread_mutex_unlock(&searchManageList.mutex);
#endif

	struct file_dnode **dnp;
	struct file_dnode dn;
	file_list_t *plist;
	file_info_t *item,*n;
	pInfo->list_nfiles = 0;
	memset(&dn,0,sizeof(struct file_dnode));
	dn.fullname = last_search_path;
	dnp = dnalloc(1);
	dnp[0] = &dn;

	plist = (file_list_t *)calloc(1,sizeof(file_list_t));
	if(plist == NULL){
		goto EXIT;
	}
		
	dl_list_init(&plist->head);

	if(search_file_traversal(pInfo, dnp, plist) != 0)
    {
		pInfo->status = -1;
		error = ERROR_FILE_SEARCH;
        EXIT_FUNC();
		goto EXIT;
    }
	
	pInfo->list_nfiles = dl_list_len(&plist->head);
	DMCLOG_D("nfiles_list: %d", pInfo->list_nfiles);
	#ifdef SEARCH_TASK_TCP
	while((pInfo->list_nfiles > 0) && (!(*pInfo->flags & FLAG_CLOSED))){
		if(pInfo->list_nfiles >= SEND_LIST_INFO_MAX){
			pInfo->send_nfiles = SEND_LIST_INFO_MAX;
		}
		else{
			pInfo->send_nfiles = pInfo->list_nfiles;
		}
		
		pInfo->statusCode++;
		res = search_handle_tcp_inotify(pInfo, plist);
		if(res != 0){
			DMCLOG_E("search handle tcp inotifu error");
			break;
		}

		i = 0;
		dl_list_for_each_safe(item, n, &plist->head, file_info_t, next)
		{
			i++;
			free_db_fd(&item);
			dl_list_del(&item->next);
			if(i >= pInfo->send_nfiles){
				break;
			}
		}
				
		pInfo->list_nfiles = dl_list_len(&plist->head);
	}
	#endif
	
	#if defined(SEARCH_TASK_UDP) || defined(SEARCH_TASK_MANAGE) 
	if((pInfo->list_nfiles > 0) && (!(*pInfo->flags & FLAG_CLOSED))){	
		#ifdef SEARCH_TASK_UDP
		res = search_handle_udp_inotify(pInfo, plist);
		if(res < 0){
			DMCLOG_D("search_handle_udp_inotify error");
		}
		#endif
		dl_list_for_each_safe(item, n, &(plist->head), file_info_t, next)
		{
			free_db_fd(&item);
			dl_list_del(&item->next);
		}
		pInfo->list_nfiles = 0;
	}
	#endif
EXIT:
	pInfo->status = 2;
#ifdef SEARCH_TASK_MANAGE
	pthread_mutex_lock(&searchManageList.mutex);	
	dl_list_for_each(search_manage_item, &(searchManageList.head), search_manage_info_t, next)
	{
		if(search_manage_item->seq == pInfo->seq){
			search_manage_item->searchStatus = 2;
			break;
		}
	}
	pthread_mutex_unlock(&searchManageList.mutex);
#endif

#ifdef SEARCH_TASK_UDP
	res = search_handle_udp_inotify(pInfo, NULL);
	if(res < 0){
		DMCLOG_D("search_handle_udp_inotify error");
	}
	DM_DomainClientDeinit(pInfo->client_fd);
#endif

	DMCLOG_D("cmd: %d", pInfo->cmd);
#if defined(SEARCH_TASK_UDP) || defined(SEARCH_TASK_MANAGE) 
	dm_file_search_inotify(pInfo->cmd,error,buf); 
    DM_MsgSend(pInfo->sock,buf,strlen(buf));
#endif

#ifdef SEARCH_TASK_TCP
	res = search_handle_tcp_inotify(pInfo, NULL);
	if(res < 0){
		DMCLOG_D("search_handle_udp_inotify error");
	}
#endif

    *(pInfo->flags) |= FLAG_CLOSED;
    *(pInfo->flags) &= ~(FLAG_R | FLAG_W | FLAG_ALWAYS_READY);
	safe_free(last_search_path);
    safe_free(pInfo->search_path);
    safe_free(pInfo->search_string);
	safe_free(plist);
    safe_free(self);
    EXIT_FUNC();
    return ;
}


void search_manage_task_func()
{
	ENTER_FUNC();
	search_manage_info_t *item, *n;
	memset(&searchManageList, 0, sizeof(search_manage_list_t));
	dl_list_init(&searchManageList.head);
	while(exit_flag == 0){
		pthread_mutex_lock(&searchManageList.mutex);
		dl_list_for_each_safe(item, n, &searchManageList.head, search_manage_info_t, next){
			search_file_list_read(item);
			if((item->searchStatus == 2) && (dl_list_len(&item->head) == 0)){
				dl_list_del(&item->next);
				safe_free(item);				
			}
		}
		pthread_mutex_unlock(&searchManageList.mutex);
		usleep(500000);
	}
	   
	EXIT_FUNC();
    return ;
}


