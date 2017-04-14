#include "cgiget.h"
#include "uci_for_cgi.h"


char *goutstr=0;

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



