#include "cgipost.h"


void postStr(char *relstr, int sorg,int flag)
{
	char *sendstr;
	char tmp_str[1024]="\0";

	sendstr = (char *)malloc(8192);
	memset(sendstr, 0, 8192);
	if(sorg)
	{
		//set
		strcpy(sendstr,"<?xml version=\"1.0\" encoding=\"utf-8\"?>");
		strcat(sendstr, "<");
		strcat(sendstr, SETSYSSTR);
		strcat(sendstr, ">");
		
		//strcat(sendstr, relstr);
		if( error_num )
			sprintf(tmp_str,"<Return status=\"false\" > %s </Return>",error_info);
		else if(!flag)
			sprintf(tmp_str,"<Return status=\"false\" ></Return>");
		else
			sprintf(tmp_str,"<Return status=\"true\" ></Return>");
		strcat(sendstr, tmp_str);
		strcat(sendstr, "</");
		strcat(sendstr, SETSYSSTR);
		strcat(sendstr, ">");
		
	}
	else
	{
		//get
		//printf("poststr get\n");
		strcpy(sendstr, "<");
		strcat(sendstr, GETSYSSTR);
		strcat(sendstr, ">");
		
		strcat(sendstr, relstr);
		if(!error_num && flag)
			strcat(sendstr, "<Return status=\"true\" ></Return>");
		if(!flag)
			strcat(sendstr, "<Return status=\"false\" ></Return>");
		strcat(sendstr, "</");
		strcat(sendstr, GETSYSSTR);
		strcat(sendstr, ">");
		// printf("HTTP/1.1 200 OK\r\n");		
	}


	//post str to client
	printf("Content-type:text/html\r\n\r\n");
	//printf("<html><title>cgi test</title>\n");
	printf("%s", sendstr);
	//printf("\n</html>\n");
	fflush(stdout);
	free(sendstr);

}


