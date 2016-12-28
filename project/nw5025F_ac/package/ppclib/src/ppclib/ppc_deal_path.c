#include "ppc_deal_path.h"
#include "ppc_token_list.h"
#include "base.h"

static int is_absolute_path(const char *src_path)
{	
	if(src_path == NULL){
		return -1;
	}
	
	if(*src_path == '/'){
		DMCLOG_D("is absolute path");
		return 1;
	}
	else{
		DMCLOG_D("is relative path");
		return 0;
	}
}

char *check_return_dot_path(char *src_full_path)
{
	char *head = NULL, *tail = NULL;
	char *before = NULL, *after = NULL, *mid = NULL;
	if(src_full_path == NULL){
		return NULL;
	}

	//DMCLOG_D("src_full_path: %s", src_full_path);
	char *tmp_full_path = (char *)calloc(1, strlen(src_full_path) + 1);
	if(tmp_full_path == NULL){
		DMCLOG_E("malloc error");
		return NULL;
	}

	char *new_path = (char *)calloc(1, strlen(src_full_path) + 1);
	if(new_path == NULL){
		DMCLOG_E("malloc error");
		safe_free(tmp_full_path);
		return NULL;
	}

	memcpy(tmp_full_path, src_full_path, strlen(src_full_path));
	if(strlen(tmp_full_path) > 1 && *(tmp_full_path+strlen(tmp_full_path) - 1) == '/'){
		*(tmp_full_path+strlen(tmp_full_path) - 1) = '\0';
	}

	head = tmp_full_path;
	while(strlen(tmp_full_path)){
		mid = strstr(head, "/..");
		if(mid == NULL){
			memcpy(new_path, tmp_full_path, strlen(tmp_full_path));
			safe_free(tmp_full_path);
			return new_path;
		}
		else{
			after = strchr(mid+1, '/');
			
			if(mid == head && after == NULL){
				strcpy(new_path, "/");
				safe_free(tmp_full_path);
				return new_path;
			}
			else if(mid == head && after != NULL){
				memcpy(new_path, after, strlen(after));
			}
			else if(mid != head && after == NULL){
				for(before = mid - 1; before != head && *before != '/'; before--);
				if(before == head){
					strcpy(new_path, "/");
					safe_free(tmp_full_path);
					return new_path;
				}
				else{
					memcpy(new_path, head, before - head);
				}					
			}
			else if(mid != head && after != NULL){
				for(before = mid - 1; before != head && *before != '/'; before--);
				if(before == head){
					memcpy(new_path, after, strlen(after));
					safe_free(tmp_full_path);
					return new_path;
				}
				else{
					memcpy(new_path, head, before - head);
					strcat(new_path, after);
				}	
			}
			else{
				DMCLOG_E("ERROR !!!");
				safe_free(tmp_full_path);
				safe_free(new_path);
				return NULL;
			}

			memset(tmp_full_path, '\0', strlen(tmp_full_path));
			memcpy(tmp_full_path, new_path, strlen(new_path));
			memset(new_path, '\0', strlen(new_path));
		}
	}
	
	safe_free(tmp_full_path);
	safe_free(new_path);
	return NULL;
}

/*
  *return vaule:
  *0:   success
  *1:   no need change
  *-1: fial
  */
int create_new_full_path(const char *src_path, char **dest_path, _int64_t token)
{
	char *work_dir_old = NULL;
	char *work_dir_new = NULL;
	char *src_full_path = NULL;
	int realloc_len = 0;
	int is_absolute = 0;
	char *p_src_path = NULL;
	if(src_path == NULL){
		return -1;
	}

	if((is_absolute = is_absolute_path(src_path)) < 0){
		return -1;
	}
		
	if(is_absolute){
		p_src_path = src_path;
		realloc_len = strlen(p_src_path) + 1;
		DMCLOG_D("p_src_path : %s, realloc_len: %d", p_src_path, realloc_len);
		src_full_path = (char *)calloc(1, realloc_len);
		if(src_full_path == NULL){
			return -1;
		}
		
		memcpy(src_full_path, src_path, strlen(src_path));
		*dest_path = src_full_path;
		return 0;
	}
	else{
		//DMCLOG_D("src_path : %s, strlen(src_path): %d", src_path, strlen(src_path));
		if((*src_path == '.' && strlen(src_path) == 1) || 
			(*src_path == '.' && *(src_path+1) == '/' && strlen(src_path) == 2)){
			DMCLOG_D("work dir no need change");
			work_dir_old = get_work_dir_from_ppc_token_list(token);
			if(work_dir_old == NULL){
				DMCLOG_E("get work dir fail");
				return -1;
			}

			realloc_len = strlen(work_dir_old) + 1;	
			src_full_path = (char *)calloc(1, realloc_len);
			if(src_full_path == NULL){
				safe_free(work_dir_old);
				return -1;
			}

			sprintf(src_full_path, "%s", work_dir_old);
			safe_free(work_dir_old);
		}
		else { 
			if(*src_path == '.' && *(src_path+1) == '/' && (src_path+2) != NULL){
				p_src_path = src_path + 2;
			}				
			else{
				p_src_path = src_path;
			}

			//DMCLOG_D("p_src_path: %s, token: %d", p_src_path, token);
			work_dir_old = get_work_dir_from_ppc_token_list(token);
			if(work_dir_old == NULL){
				DMCLOG_E("get work dir fail");
				return -1;
			}

			realloc_len = strlen(work_dir_old) + strlen(p_src_path) + 2;	
			src_full_path = (char *)calloc(1, realloc_len);
			if(src_full_path == NULL){
				safe_free(work_dir_old);
				return -1;
			}

			if(*(work_dir_old + strlen(work_dir_old) - 1) == '/')
				sprintf(src_full_path, "%s%s", work_dir_old, p_src_path);	
			else
				sprintf(src_full_path, "%s/%s", work_dir_old, p_src_path);
			safe_free(work_dir_old);
		}
	}

	work_dir_new = check_return_dot_path(src_full_path);
	if(work_dir_new != NULL){
		*dest_path = work_dir_new;
		safe_free(src_full_path);
		return 0;
	}
	else{
		safe_free(src_full_path);
		return -1;
	}
}


