#ifndef _URL_FILTER_H_
#define _URL_FILTER_H_

char *get_arg_from_url(char *url, char *arg_name);
int check_signature(char *sign_str, char *access_key, char *ssig, 
	int ssig_start, int ssig_end);

#endif