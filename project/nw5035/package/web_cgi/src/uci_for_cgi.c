#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <uci.h>
#include "my_debug.h"
static const char *delimiter = " ";

static enum {
	CLI_FLAG_MERGE =    (1 << 0),
	CLI_FLAG_QUIET =    (1 << 1),
	CLI_FLAG_NOCOMMIT = (1 << 2),
	CLI_FLAG_BATCH =    (1 << 3),
	CLI_FLAG_SHOW_EXT = (1 << 4),
	CLI_FLAG_NOPLUGINS= (1 << 5),
} flags;

struct uci_context *ctx;
//int uci_get_str_len=0;

unsigned char* urlDecode(char *string)  
{
		int destlen = 0;
		unsigned char *src, *dest;
		unsigned char *newstr;
	
		if (string == NULL) return NULL;
	
		for (src = string; *src != '\0'; src++) 	
		{
		   if (*src == '%')
			{
				destlen++;
				src++;
			}
		   else destlen++;
		}
		newstr = (unsigned char *)malloc(destlen + 1);
		src = string;
		dest = newstr;
	
		while (*src != '\0')  
		{
			if (*src == '%')
			{
				char h = toupper(src[1]);
				char l = toupper(src[2]);
				int vh, vl;
				vh = isalpha(h) ? (10+(h-'A')) : (h-'0');
				vl = isalpha(l) ? (10+(l-'A')) : (l-'0');
				*dest++ = ((vh<<4)+vl);
				src += 3;
			} 
			else if (*src == '+') 
			{
				*dest++ = ' ';
				src++;
			} 
			else
			{
				*dest++ = *src++;
			}
		}
		
		*dest = 0;
	
	   return newstr;
	}



char * GetStringFromWeb()
{
	char method[10];
	char *outstring;
	int string_length;
	
	strcpy(method,getenv("REQUEST_METHOD"));
	
	if(! strcmp(method,"POST"))
	{
		//fprintf(stdout,"method is post<br>");
		string_length=atoi(getenv("CONTENT_LENGTH"));
		if(string_length!=0)
		{
			outstring=malloc(sizeof(char)*string_length+1);
			fread(outstring,sizeof(char),string_length,stdin);
			//fprintf(stdout,"%s<br>",outstring);
		}
	}

	else if(! strcmp(method,"GET"))
	{
		//fprintf(stdout,"method is get<br>");
		string_length=strlen(getenv("QUERY_STRING"));
		outstring=malloc(sizeof(char)*string_length+1);
		strcpy(outstring,getenv("QUERY_STRING"));
		string_length=strlen(outstring);
		//fprintf(stdout,"%s<br>",outstring);
	}
	
	if(string_length==0)
	{
		return NULL;
	}
	return outstring;
	
	
}

int processString(char *string,char *name,char *value)
{
#define debug 0
	char *ret_value=value;
	char *p_name;
	//p_debug(string);
	//p_debug(name);


	if(string==NULL || strlen(string)==0)
	{
		#if debug
		printf("the string is wrong. ");
		#endif
		return -1;
	}
	
	if((p_name=strstr(string,name))==NULL)
	{
		#if debug
		printf("the name is not found. ");
		#endif
		return -1;
	}
	//else{
	//	p_debug("find name=%s",name);
	//	p_debug("p_name=%s",p_name);
	//}
	p_name+=strlen(name)+1;
	if((*p_name)=='&' || !(*p_name) )
	{
		#if debug
		printf("the value is empty. ");
		#endif
		*value='\0';
		return 0;
	}
	while( *p_name != '&' && *p_name)
	{
		if(*p_name =='+')
		{
			*ret_value=' ';
		}
		else
		{
			*ret_value=*p_name;
		}
		p_name++;
		
		ret_value++;
	}
	*ret_value='\0';
	//p_debug("value=%s",value);
	return 0;
}

void cgi_log( char *string){
	FILE *fw_fp;
	int f_size=0;
	if( (fw_fp=fopen("/tmp/cgi_log.txt","ab+"))==NULL)    // write and read,binary
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

