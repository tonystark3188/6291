#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <mntent.h>
#include <sys/vfs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>     /*Unix 标准函数定义*/
#include <sys/types.h>  
#include <libudev.h>
#include <locale.h>
#include "router_defs.h"
#include "my_debug.h"

#ifdef SUPPORT_OPENWRT_PLATFORM
#include "uci.h"


static enum {
	CLI_FLAG_MERGE =    (1 << 0),
	CLI_FLAG_QUIET =    (1 << 1),
	CLI_FLAG_NOCOMMIT = (1 << 2),
	CLI_FLAG_BATCH =    (1 << 3),
	CLI_FLAG_SHOW_EXT = (1 << 4),
	CLI_FLAG_NOPLUGINS= (1 << 5),
} flags;

static void cli_perror(struct uci_context *ctx)
{
	if (flags & CLI_FLAG_QUIET)
		return;
	uci_perror(ctx, "uci");
}

static void uci_show_value(struct uci_option *o,char *uci_str_value)
{
	struct uci_element *e;
	bool sep = false;

	switch(o->type) {
	case UCI_TYPE_STRING:
		strcpy(uci_str_value,o->v.string);
		break;
	case UCI_TYPE_LIST:
		uci_foreach_element(&o->v.list, e) {
			printf("%s%s", (sep ? "?" : ""), e->name);
			sep = true;
		}
		printf("\n");
		break;
	default:
		printf("<unknown>\n");
		break;
	}
}


static struct uci_context* init_uci_context(void)
{
	struct uci_context *ctx = NULL;
	ctx=uci_alloc_context();
	if(ctx==NULL)
		return NULL;
	return ctx;
}

static void free_uci_context(struct uci_context *ctx)
{
	if(ctx != NULL)
		uci_free_context(ctx);
}


int uci_get_option_value(char *uci_option_str,char *uci_str_value)
{
	struct uci_context *ctx = NULL;
	struct uci_element *e = NULL;
	struct uci_ptr ptr;
	int ret = UCI_OK;
	ctx = init_uci_context();
	if(ctx == NULL){
		return -1;
	}
	
	if (uci_lookup_ptr(ctx, &ptr, uci_option_str, true) != UCI_OK) {
		goto except_1;
	}
	
	e = ptr.last;
	
	if (!(ptr.flags & UCI_LOOKUP_COMPLETE)) {
		ctx->err = UCI_ERR_NOTFOUND;
		cli_perror(ctx);
		goto except_1;
	}
	switch(e->type) {
	case UCI_TYPE_SECTION:
		printf("%s\n", ptr.s->type);
		break;
	case UCI_TYPE_OPTION:
		uci_show_value(ptr.o,uci_str_value);
		break;
	default:
		break;
	}

	free_uci_context(ctx);
	return 0;
except_1:
	free_uci_context(ctx);
	return -1;
}

int uci_set_option_value(char *uci_str)
{
	struct uci_context *ctx = NULL;
	struct uci_element *e;
	struct uci_ptr ptr;
	int ret = UCI_OK;
	ctx = init_uci_context();
	if(ctx == NULL){
		return -1;
	}

	if (uci_lookup_ptr(ctx, &ptr, uci_str, true) != UCI_OK) {
		goto except_2;
	}
	e = ptr.last;
	ret=uci_set(ctx,&ptr);
	if(ret != UCI_OK){
		goto except_2;
	}
	
	ret = uci_save(ctx,ptr.p);
	if(ret != UCI_OK){
		goto except_2;
	}

	ret = uci_commit(ctx, &ptr.p, 1);
	if(ret != UCI_OK){
		goto except_2;
	}

	free_uci_context(ctx);
	return 0;
except_2:
	free_uci_context(ctx);
	return -1;
}

static int shell_system_cmd(const char * cmd,char *cmd_result) 
{ 
	FILE * fp; 
//	int res; char buf[1024]; 
	char buffer[512];
	int chars_read=0;
//printf("shell_system_cmd %s\n",cmd);
	if (cmd == NULL) 
	{ 
		printf("my_system cmd is NULL!\n");
		return -1;
	}

	if ((fp = popen(cmd, "r") ) == NULL) 
	{ 
		perror("popen");
		return -1;
//		printf("popen error: %s/n", strerror(errno)); return -1; 
	} 
	else
	{	
		memset(buffer,0,512);
		chars_read=fread(buffer, sizeof(char), 512-1, fp); 
//		printf("read buffer=%s %d\n",buffer,chars_read);
		if(chars_read>0&&cmd_result!=NULL&&strcmp(cmd_result,"NULL")!=0)
			{
			if(buffer[chars_read-1]=='\r'||buffer[chars_read-1]=='\n')
				{
				strncpy(cmd_result,buffer,chars_read-1);
				 pclose(fp);
				 return chars_read-1;
				}
			else
				strncpy(cmd_result,buffer,chars_read);
			}
		if (  pclose(fp)<0) 
		{ 
			printf("close popen file pointer fp error!\n"); return -1;
		} 
	}
//	printf("get Valuse=%s \n",cmd_result);
	return chars_read;
} 

int shell_uci_get_value(char *uci_option_str,char *uci_str_value)
{
return shell_system_cmd(uci_option_str,uci_str_value);
}
int shell_uci_set_value(char *uci_str)
{
return shell_system_cmd(uci_str,NULL);
}

int shell_uci_find_value(char *macth_patten,char *uci_str)
{
char buffer[256]={0};
//printf("uci_find_value =%s \n",macth_patten);
//if(strcmp(uci_str,"NULL"==0))
	{
	sprintf(buffer,"uci show|awk '/%s$/ {print $1;exit}\'",macth_patten);
//	printf("shell_uci_find_value =%s \n",buffer);
//	strcpy(uci_str,buffer);
	return shell_system_cmd(buffer,uci_str);
	}
//return 0;
}
#endif

