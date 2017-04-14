#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <uci.h>

static const char *delimiter = " ";

static enum {
	CLI_FLAG_MERGE =    (1 << 0),
	CLI_FLAG_QUIET =    (1 << 1),
	CLI_FLAG_NOCOMMIT = (1 << 2),
	CLI_FLAG_BATCH =    (1 << 3),
	CLI_FLAG_SHOW_EXT = (1 << 4),
	CLI_FLAG_NOPLUGINS= (1 << 5),
} flags;

struct uci_context *ctx = NULL;
//int uci_get_str_len=0;

void cgi_log( char *string){
	FILE *fw_fp;
	int f_size=0;
	if( (fw_fp=fopen("/tmp/log.txt","ab+"))==NULL)    // write and read,binary
	{
		exit(1);
	}		
	f_size=fwrite(string,1,strlen(string),fw_fp);
	fclose(fw_fp);
	return;
}

static void cli_perror(void)
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
		//printf("%s\n", o->v.string);
		//uci_get_str_len=strlen(o->v.string);
		strcpy(uci_str_value,o->v.string);
		break;
	case UCI_TYPE_LIST:
		uci_foreach_element(&o->v.list, e) {
			printf("%s%s", (sep ? delimiter : ""), e->name);
			sep = true;
		}
		printf("\n");
		break;
	default:
		printf("<unknown>\n");
		break;
	}
}

int uci_get_option_value(char *uci_option_str,char *uci_str_value)
{
	struct uci_element *e;
	struct uci_ptr ptr;
	int ret = UCI_OK;
	
	if (uci_lookup_ptr(ctx, &ptr, uci_option_str, true) != UCI_OK) {
		//cli_perror();
		return 1;
	}
	
	e = ptr.last;
	
	if (!(ptr.flags & UCI_LOOKUP_COMPLETE)) {
		ctx->err = UCI_ERR_NOTFOUND;
		cli_perror();
		return 1;
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
	return 0;
}
int uci_set_option_value(char *uci_str)
{
	struct uci_element *e;
	struct uci_ptr ptr;
	int ret = UCI_OK;
	if (uci_lookup_ptr(ctx, &ptr, uci_str, true) != UCI_OK) {
		//cli_perror();
		return 1;
	}
	e = ptr.last;
	ret=uci_set(ctx,&ptr);
	if(ret==UCI_OK)
		ret=uci_save(ctx,ptr.p);
	
	if(ret != UCI_OK)
	{
		//cli_perror();
		return 1;
	}
	return 0;
}

