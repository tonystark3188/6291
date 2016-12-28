#include "base64.h"
#include "hmac_sha1.h"
#include "my_debug.h"


char *get_arg_from_url(char *url, char *arg_name)
{
	char *arg_value = NULL;
	char *p_start = NULL, *p_end = NULL;

	if(url == NULL || arg_name == NULL){
		DMCLOG_E("url or arg_name is error");
		return NULL;
	}

	while((p_start = strstr(url, arg_name)) != NULL){
		if((p_start = p_start + strlen(arg_name)) != NULL){
			if(*p_start == '=' && (p_start++) != NULL) {
				if((p_end = strchr(p_start, '&')) != NULL)
					p_end--;
				else
					p_end = p_start + strlen(p_start);
				
				arg_value = (char *)calloc(1, p_end - p_start + 2);
				if(arg_value != NULL){
					memcpy(arg_value, p_start, p_end - p_start + 1);
					DMCLOG_D("arg_value; %s", arg_value);
					return arg_value;
				}
				else{
					DMCLOG_E("malloc error!");
					return NULL;
				}
			}
		}
	}

	return NULL;
}



//0: Match successful  
//1: Match fail
//-1:Error
int check_signature(char *sign_str, char *access_key, char *ssig, 
	int ssig_start, int ssig_end)
{	
	char ssig_str[256];
	char ssig_base64[256];
	char *p_ssig = NULL;
	if(sign_str == NULL || access_key == NULL || ssig == NULL){
		DMCLOG_E("sign_str/access_key/ssig is error");
		return -1;
	}

	DMCLOG_D("sign_str: %s, access_key: %s, ssig: %s", sign_str, access_key, ssig);
	int	t=20;
	memset(ssig_str, 0, sizeof(ssig_str));
	hmac_sha(access_key, strlen(access_key), sign_str, strlen(sign_str), ssig_str, t);

	if(!strlen(ssig_str)){
		DMCLOG_E("get ssig str fail");
		return -1;
	}
	DMCLOG_D("ssig_str: %s", ssig_str);

	memset(ssig_base64, 0, sizeof(ssig_base64));
	base64_encode(ssig_str, ssig_base64, strlen(ssig_str));

	if(!strlen(ssig_base64)){
		DMCLOG_E("get ssig base64 fail");
		return -1;
	}
	DMCLOG_D("ssig_base64: %s", ssig_base64);

	if(strlen(ssig_base64) < ssig_end){
		DMCLOG_E("match ssig fail");
		return 1;
	}

	p_ssig = ssig_base64 + ssig_start - 1;
	DMCLOG_D("p_ssig: %s", p_ssig);
	if(!strncmp(p_ssig, ssig, ssig_end - ssig_start)){
		DMCLOG_D("match successful");
		return 0;
	}
	else{
		DMCLOG_D("mathc fail");
		return 1;
	}
}

#if 0
#define ACCESS_KEY_NAME		"KID"
#define SSIG_NAME			"ssig"
#define MATCH_SSIG_START	5
#define MATCH_SSIG_END		15

int main(int argc, char *argv[]){
	int ret = -1;
	char *url = "http://192.168.222.254/webdav/test/test.mp3?KID=512312&ssig=OWM1MmE0NW";
	char *sign_url = NULL;
	char *access_key = NULL;
	char *ssig = NULL;
	char *p_query;

	if((p_query = strchr(url, '?')) == NULL){
		DMCLOG_E("has no arg of url: %s", url);
		ret = -1;
		goto EXIT;
	}

	sign_url = (char *)calloc(1, p_query - url + 1);
	if(sign_url == NULL){
		DMCLOG_E("malloc error");
		ret = -1;
		goto EXIT;
	}

	memcpy(sign_url, url, p_query - url);

	DMCLOG_D("sign_url: %s", sign_url);

	if((access_key = get_arg_from_url(p_query, ACCESS_KEY_NAME)) == NULL){
		DMCLOG_E("get arg:%s fail", ACCESS_KEY_NAME);
		ret = -1;
		goto EXIT;
	}
	DMCLOG_D("access_key: %s", access_key);

	if((ssig = get_arg_from_url(p_query, SSIG_NAME)) == NULL){
		DMCLOG_E("get arg:%s fail", ACCESS_KEY_NAME);
		ret = -1;
		goto EXIT;
	}
	DMCLOG_D("ssig: %s", ssig);

	ret = check_signature(sign_url, access_key, ssig, MATCH_SSIG_START, MATCH_SSIG_END);
	if(!ret){
		DMCLOG_D("match success");
		ret = 0;
		goto EXIT;
	}
	else{
		DMCLOG_D("match fail");
		ret = -1;
		goto EXIT;
	}

EXIT:
	if(sign_url != NULL)
		free(sign_url);

	if(access_key != NULL)
		free(access_key);

	if(ssig != NULL)
		free(ssig);

	return ret;
}
#endif
