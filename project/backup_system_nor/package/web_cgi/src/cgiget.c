#include "cgiget.h"


char *goutstr=0;



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

#define CGI_LOG
#ifdef CGI_LOG	
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
#endif

char *GetString(char *qurey, char *getstr)
{
	char *tmp;
	char *tmp1;
	int i=0;
	char *outstr;
	char *realstr;
	tmp = strstr(qurey, getstr);
	if(tmp == NULL)
	{
		return NULL;
	}
	
	tmp1=tmp;

	while(1)
	{
		if(*tmp =='&' || *tmp==0)
			break;
		i++;
		tmp++;
	}
	

	outstr = (char *)malloc(i+1);
	memcpy(outstr, tmp1, i);
	*(outstr+i)=0;
	//printf("%s\n",outstr);
	realstr = urlDecode(outstr);
	free(outstr);
	return realstr;
}
#if 0
char* getcgidata(FILE* fp, char* requestmethod)
{

       char* input;
       int len;
	   char *pppp;
       int size = 1024;
//       int i = 0;
     if(!strcmp(requestmethod, "GET"))
       {

              input = getenv("QUERY_STRING");
              return input;

       }
       else if (!strcmp(requestmethod, "POST"))
       {
   		    pppp=getenv("CONTENT_LENGTH");
             len = atoii(pppp);
              input = (char*)malloc(sizeof(char)*(size + 1));    

              if (len == 0)
              {
                     input[0] = '\0';

                     return input;
              }
        
		fgets(input, len+1, stdin);
			input[len]='\0';
		 return input;
       }

       return NULL;
}
#endif

char * GetStringFromWeb()
{
	char method[10];
	char *outstring;
	int string_length = 0;
	
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
		if(NULL != getenv("QUERY_STRING")){
			string_length=strlen(getenv("QUERY_STRING"));
			outstring=malloc(sizeof(char)*string_length+1);
			strcpy(outstring,getenv("QUERY_STRING"));
			string_length=strlen(outstring);
		}
		//fprintf(stdout,"%s<br>",outstring);
	}
#ifdef CGI_LOG			
cgi_log(outstring);
#endif	
	if(string_length==0)
	{
		return NULL;
	}
	return outstring;
	
	
}




char *getCgiStr(void)
{
	char *query;
	char *outstr;
	//char *wp;
	//char *req_method;
	char * web_string;

#if 0
	goutstr = (char *)malloc(256);
	memset(goutstr,0,256);
	
	//strcpy(goutstr, "<setSysInfo><SSID name=\"airflash\" encrypt=\"WPA2\" channel=\"8\" password=\"88888888\" tkip_aes=\"aes\" encrypt_len=\"\" format=\"\"></SSID></setSysInfo>");
	//strcpy(goutstr,"<setSysInfo><WorkMode value=\"0\"></WorkMode></setSysInfo>");
	//strcpy(goutstr,"<setSysInfo><JoinWireless><AP name=\"airflash\" encrypt=\"WPA2-PSK\" password=\"88888888\" tkip_aes=\"aes\"></AP></JoinWireless></setSysInfo>");
	//strcpy(goutstr,"<setSysInfo><JoinWireless name=\"airflash\" encrypt=\"WPA2-PSK\" password=\"88888888\" tkip_aes=\"aes\"></JoinWireless></setSysInfo>");
	//strcpy(goutstr, "<getSysInfo><SSID></SSID><RemoteAP></RemoteAP><WorkMode></WorkMode><FTP></FTP><SAMBA></SAMBA><DMS></DMS></getSysInfo>");
	//strcpy(goutstr,"<setSysInfo><JoinWired><DHCP></DHCP></JoinWired></setSysInfo>");
	//strcpy(goutstr,"<setSysInfo><FTP user=\"pisen\" password=\"123456\" path=\"\" enable=\"ON\" anonymous_en=\"OFF\"></FTP></setSysInfo>");
	strcpy(goutstr, "<getSysInfo><APList></APList></getSysInfo>");
	//strcpy(goutstr,"<setSysInfo><Time value=\"2011-12-12 09:40:30\"></Time></setSysInfo>");
#else
	//goutstr = (char *)malloc(1024);
	//memset(goutstr,0,1024);
	//query = (char *)malloc(1024);
	//memset(query,0,1024);
	//query = getenv("QUERY_STRING");
	//req_method = getenv("REQUEST_METHOD");
	//wp = getcgidata(stdin, req_method);
	web_string=GetStringFromWeb();
	//printf("%s\n",web_string);
	goutstr = GetString(web_string, CGISTR);
	//goutstr=goutstr+5;
	//goutstr = web_string;
	//goutstr=GetStringFromWeb();
	//printf("%s\n",goutstr);
#endif
	//change str (not need)
	free(web_string);
	return goutstr;
}

void freeCgiStr(void)
{
	//do nothing
	if(goutstr)
		free(goutstr);
	goutstr = 0;
}



