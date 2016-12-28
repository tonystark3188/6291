#include "cgihandle.h"
#include "cgiget.h"
#include "uci_for_cgi.h"
#include <sys/mman.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <sys/types.h>        
#include <sys/socket.h>      
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>             
#include <ctype.h>
#include <time.h>           
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>      
#include <setjmp.h>

#define AR9300_OTP_BASE			0x14000
#define AR9300_OTP_STATUS		0x15f18
#define AR9300_OTP_STATUS_TYPE		0x7
#define AR9300_OTP_STATUS_VALID		0x4
#define AR9300_OTP_STATUS_ACCESS_BUSY	0x2
#define AR9300_OTP_STATUS_SM_BUSY	0x1
#define AR9300_OTP_READ_DATA		0x15f1c

#define otp_adder 0xb8114030
	void one_int_read(unsigned int addree,unsigned int *num)
	{
		unsigned int map_base;
		int fbb;
		fbb=open("/dev/mem",O_RDWR | O_SYNC);
		map_base=(char *)mmap(0,1024*128,PROT_READ|PROT_WRITE,MAP_SHARED,fbb,0x18114000);
		*num=*(volatile unsigned int *)(map_base+(addree-0xb8114000));
//		printf("read map_base=%x num=%x \n",map_base,*num);
		close(fbb);
	}

	int ath9k_hw_wait(unsigned int reg,unsigned int mask,unsigned int val,unsigned int timeout)
	{
		int i;
		unsigned int data;
	
		for (i = 0; i<(timeout/20);i++) {
				one_int_read(0xb8115f18,&data);
			if (data== val)
				return 0;
			usleep(20);
		}
		return -1;
	}

	int one_read_time(unsigned int addree,unsigned int *num)
	{
		unsigned int tmpint;
		one_int_read(addree,&tmpint);
		if (ath9k_hw_wait(AR9300_OTP_STATUS, AR9300_OTP_STATUS_TYPE,AR9300_OTP_STATUS_VALID, 1000)<0)
			return -1;	
	//	one_int_read(0xb8115f18,&tmpint);
		one_int_read(0xb8115f1c,num);
		return 0; 
	}

	int read_buffer_otp(unsigned int addree,unsigned char *buff,unsigned int len)
	{
		int i;
		unsigned int data,addr;
		addr=addree&0xfffffffc;
		for(i=0;i<len;i=i+3)
		{
			if(one_read_time(addr,&data)<0)
			{
				return -1;
			}
			buff[i]=((data&0xff000000)>>24);
			buff[i+1]=((data&0x00ff0000)>>16);
			buff[i+2]=((data&0x0000ff00)>>8);
	/*		
			buff[i+2]=((data&0xff000000)>>24);
			buff[i+1]=((data&0x00ff0000)>>16);
			buff[i]=((data&0x0000ff00)>>8);
	*/
			addr=addr+4;
		}
		return 0;
	}

int encrypt_read()
{
	unsigned char mac[20];
	unsigned char buff[32];
	int i;
	unsigned char mac_product[20];
	memset(mac,0,20);
	memset(mac_product,0,20);
	read_buffer_otp(otp_adder,buff,32);
/*
	mac[0]=swim_chang(buff[0]);
	mac[1]=swim_chang(buff[1]);
	mac[2]=swim_chang(buff[4]);
	mac[3]=swim_chang(buff[5]);
	mac[4]=swim_chang(buff[8]);
	mac[5]=swim_chang(buff[9]);
	mac[6]=swim_chang(buff[12]);
	mac[7]=swim_chang(buff[13]);
	mac[8]=swim_chang(buff[16]);
	mac[9]=swim_chang(buff[17]);
	mac[10]=swim_chang(buff[20]);
	mac[11]=swim_chang(buff[21]);

	if(get_Product_mac(mac_product)<0)
	{
		printf("get product error\n");
		return -1;
	}

	if(!strcmp(mac_product,mac))
	{
		return 0;
	}else
		{
		return -1;
	}
*/
	for(i=0;i<32;i++)
	{
		if(buff[i]!=0)
		{
			return 0;
		}
	}
	return -1;
}

#ifdef ENCRYPT_ENABLE
char buffer[BUFSIZ]; 
char * getPIDformPST(char *iface)
	{
	
		FILE *read_fp; 
		int chars_read; 
		char *pid; 
		memset( buffer, 0, BUFSIZ ); 
		char tmpStr[30];
		//	printf("int size=%d\n",sizeof(int));
		sprintf(tmpStr,"ps -w |grep %s|grep -v grep",iface);
	//	printf("tmpStr=%s\n",tmpStr);
		pid=0;
		read_fp=popen(tmpStr,"r");
		if(read_fp!=NULL)
			{
			chars_read = fread(buffer, sizeof(char), BUFSIZ-1, read_fp); 
			if (chars_read > 0) 
			{ 
			//			printf("buffer=%s\n",buffer);
				pid=strtok(buffer," ");
				if(pid!=NULL)
					{
	//				printf("\n%s pid=%s\n",iface,pid);
					pclose(read_fp); 
					return pid;
				}else
					{
					pclose(read_fp); 
					return 0;//NULL
				}
			} 
			else 
			{ 
				pclose(read_fp); 
				return 0; //NULL
			} 
	
		}else	return 0;//NULL
	
	
	}
#endif

int xmlgetroot(char *xmlstr, char *rootstr);

void main(void)
{
	char *cgistr;	
#ifdef XML2
	xmlDoc		   *doc = NULL;
	xmlNodePtr     curNode;
	xmlNodePtr	   rootnode;
#else
	xmlNodePtr doc;
	char rootstr[100];
	char tmp[200];
#endif

	int ret;
#ifdef ENCRYPT_ENABLE
			unsigned char ptr_name[]="encrypt";
			if(getPIDformPST(ptr_name)==NULL)
			{
				return;
			}
#endif
			if(encrypt_read()<0)
			{
				return;
			}	
	cgistr = getCgiStr() + 5;  //skip "data="
	ctx=uci_alloc_context();
	if(cgistr == 0)
		return;

#ifdef XML2
	//
	doc = xmlParseMemory(cgistr,strlen(cgistr));
	
	if(doc == NULL)
	{
		//WRONG
		goto main_error;
	}

	rootnode = xmlDocGetRootElement(doc);
	if(rootnode == NULL)
	{
		//WRONG
		xmlFreeDoc(doc);
		goto main_error;
	}
#else
	//doc =  mxmlLoadString(NULL, cgistr,		MXML_NO_CALLBACK);
	ret = xmlgetroot(cgistr,rootstr);
#endif

	ret = roothandle(rootstr, cgistr+ret);

#ifdef XML2
	xmlFreeDoc(doc);
#else
	//mxmlDelete(tree);
#endif
	
	
	
	
main_error:	
	
	freeCgiStr();
	//xmlFreeDoc(doc);
	
	uci_free_context(ctx);
}


//<setSysInfo><SSID name="" encrypt="" password="" tkip_aes="" encrypt_len ="" format="" ></SSID></setSysInfo>

int xmlgetroot(char *xmlstr, char *rootstr)
{
	int i,j;
	char tmp[200];
	if(xmlstr[0]!='<')
		return -1;

	i=1;
	if(xmlstr[i] == '?')
	{
		while(xmlstr[i++] != '>');
		
		i++;
	}
	//get rootstr
	//i=1;
	j=0;
	while(xmlstr[i] == ' ')
		i++;
	memset(tmp, 0, 200);
	while((xmlstr[i]!= ' ') && (xmlstr[i]!= '>') )
	{
		tmp[j++]=xmlstr[i++];
	}

	//get '>' ??

	if(xmlstr[i] != '>')
		while(xmlstr[i++] == ' ');

	if(xmlstr[i] != '>')
		return -1;

	//get right root
	strcpy(rootstr, tmp);
	return (i+1);
}

void xmlGetTag(char *rootstr, char *tagstr)
{
	int i,j;
	char tmp[200];
	if(rootstr[0]!='<')
		return -1;

	//get rootstr
	i=1;
	j=0;
	while(rootstr[i] == ' ')
		i++;

	memset(tmp, 0, 200);
	while(1)
	{
		if(rootstr[i] == ' ' || rootstr[i] == '>' || rootstr[i] == '/')
			break;
		tmp[j++]=rootstr[i++];
	}

	strcpy(tagstr, tmp);

}

char *xmlGetprop(char *tag, char *name)
{
	int i;
	int j;
	int len;

	char *tagtmp;
	char *nstr;
	char *ntmp;

	i=0;
	while(tag[i++] != '>');

	tagtmp = (char *)malloc(i+2);
	memset(tagtmp, 0, i+2);
	memcpy(tagtmp, tag, i);

	i=0;
	while(1)
	{
		ntmp = strstr(tagtmp+i, name);

		if(ntmp == NULL)
		{
			free(tagtmp);
			return NULL;
		}
		
		if(ntmp[strlen(name)]!= '=')
		{
			if(ntmp > tagtmp)
				i = ntmp - tagtmp + strlen(name);
			else
				return NULL;
				
			continue;
		}
		else
			break;
	}

	//get name str;
	i=0;
	while(ntmp[i++] != '\"');
	j=i;
	//i++;
	while(ntmp[i++] != '\"');

	if((i-j)== 1)
	{
		free(tagtmp);
		return NULL;
	}
	nstr = (char*)malloc(i-j+2);
	memset(nstr, 0, i-j+2);
	memcpy(nstr, ntmp+j, i-j-1);
	free(tagtmp);

	return nstr;
}


int xmlGetNextTag(char *tagstr, char *nodrstr)
{
	int i;
	int j;
	if(tagstr[0] != '<' || tagstr[0]==0)
		return 0;

	if(tagstr[1]=='/' || tagstr[1]==0)
		return 0;

	i=1;
	j=0;

	while(1)
	{
		if(tagstr[i] == ' ' || tagstr[i] == '>' || tagstr[i] == '/' ||tagstr[i] == 0)
		{
			break;
		}
		
		nodrstr[j++]=tagstr[i++];
	}
	nodrstr[j++] = 0;
	return 1;
}

