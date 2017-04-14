#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "uci_for_cgi.h"
#include "md5.h"
#include "http_client.h"
int dm_create_local_file(http_tcpclient *pclient)
{
	p_debug("save path = %s", pclient->tmp_path);
    pclient->local_fd = open(pclient->tmp_path, O_CREAT|O_WRONLY,0644);
    if(pclient->local_fd <= 0)
    {
        return -1;
    }
    return pclient->local_fd;
}

int http_tcpclient_create(http_tcpclient *pclient,const char *host, int port,int time_out)
{
	int ret=0;
	//p_debug("access http_tcpclient_create host = %s,port = %d", host, port);
	struct hostent *he;
	struct timeval tv_out;

	if(pclient != NULL) 
	{
		if((he = gethostbyname(host))==NULL){
			ret=-2;
		}else{
			strcpy(pclient->host,host);
			pclient->remote_port = port;
			strcpy(pclient->remote_ip,inet_ntoa( *((struct in_addr *)he->h_addr) ));

			pclient->_addr.sin_family = AF_INET;
			pclient->_addr.sin_port = htons(pclient->remote_port);
			pclient->_addr.sin_addr = *((struct in_addr *)he->h_addr);

			if((pclient->socket = socket(AF_INET,SOCK_STREAM,0))==-1){
				ret=-3;
			}else{
				pclient->time_out = time_out;
				int flags = fcntl(pclient->socket, F_GETFL, 0);  
				fcntl(pclient->socket, F_SETFL, flags|O_NONBLOCK); 

				/* setsockopt set send and rev time out 
				tv_out.tv_sec = time_out / MSECS_IN_SEC;
				tv_out.tv_usec = (time_out % MSECS_IN_SEC) * USECS_IN_MSEC;
				setsockopt(pclient->socket, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));
				setsockopt(pclient->socket, SOL_SOCKET, SO_SNDTIMEO, &tv_out, sizeof(tv_out));
		 */	}
		}
	}else ret=-1;
	//if(ret<0)	report_status_err_time++;
	//p_debug("leave http_tcpclient_create ret=%d",ret);
	return ret;
}

int http_tcpclient_conn(http_tcpclient *pclient)
{
	//p_debug("access http_tcpclient_conn");
	
	if(pclient->connected)
		return 1;
/*
	if(connect(pclient->socket, (struct sockaddr *)&pclient->_addr,sizeof(struct sockaddr))==-1){
		return -1;
	}
	*/
	//none block way
	int ret=0;
	int res=connect(pclient->socket, (struct sockaddr *)&pclient->_addr,sizeof(struct sockaddr));

	if (0 == res)  
	 {  
	 	 	//printf("socket connect succeed immediately.\n");  
	        ret = 0;  
	 }  
	 else  
	 {  
  		//printf("get the connect result by select().\n");  
  		 if (errno == EINPROGRESS)  
  		 {  
            int times = 0;  
            while (times++ < 5)  
            {  
                fd_set rfds, wfds;  
                struct timeval tv;  
                  
                //printf("errno = %d\n", errno);  
                FD_ZERO(&rfds);  
                FD_ZERO(&wfds);  
                FD_SET(pclient->socket, &rfds);  
                FD_SET(pclient->socket, &wfds);  
                  
                /* set select() time out */  
                tv.tv_sec = 10;   
                tv.tv_usec = 0;  
                int selres = select(pclient->socket + 1, &rfds, &wfds, NULL, &tv);  
                switch (selres)  
                {  
                    case -1:  
                        p_debug("select error\n");  
                        ret = -1;  
                        break;  
                    case 0:  
                        p_debug("select time out\n");  
                        ret = -1;  
                        break;  
                    default:  
                        if (FD_ISSET(pclient->socket, &rfds) || FD_ISSET(pclient->socket, &wfds))  
                        {  
                        #if 0 // not useable in linux environment, suggested in <<Unix network programming>>  
                            int errinfo, errlen;  
                            if (-1 == getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, &errinfo, &errlen))  
                            {  
                                printf("getsockopt return -1.\n");  
                                ret = -1;  
                                break;  
                            }  
                            else if (0 != errinfo)  
                            {  
                                printf("getsockopt return errinfo = %d.\n", errinfo);  
                                ret = -1;  
                                break;  
                            }  
                              
                            ret = 0;  
                            printf("connect ok?\n");  
                        #else  
                        #if 1  
                            connect(pclient->socket, (struct sockaddr *)&pclient->_addr, sizeof(struct sockaddr_in));  
                            int err = errno;  
                            if  (err == EISCONN)  
                            {  
	                            pclient->connected = 1;
                                p_debug("connect success.\n");  
                                ret = 0;  
                            }  
                            else  
                            {  
                                p_debug("connect failed. errno = %d\n, retry", errno);  
                                //p_debug("FD_ISSET(sock_fd, &rfds): %d\n FD_ISSET(sock_fd, &wfds): %d\n", FD_ISSET(pclient->socket, &rfds) , FD_ISSET(pclient->socket, &wfds));  
                                ret = errno;  
                            }  
                        #else  
                        char buff[2];  
                        if (read(sock_fd, buff, 0) < 0)  
                        {  
                            printf("connect failed. errno = %d\n", errno);  
                            ret = errno;  
                        }  
                        else  
                        {  
                            printf("connect finished.\n");  
                            ret = 0;  
                        }  
                        #endif  
                        #endif  
                        }  
                        else  
                        {  
                            p_debug("haha\n");  
                        }  
                }  
                  
                if (-1 != selres && (ret != 0))  
                {  
                    p_debug("check connect result again... %d\n", times);  
                    continue;  
                }  
                else  
                {  
                    break;  
                }  
            }  
	  }  
	  else  
	  {  
	   p_debug("connect to host failed.\n");  
	   ret = errno;  
	  }  
	 } 

	pclient->connected = 1;

	return 0;
}
/*
 * Check whether full request is buffered Return headers length, or 0
 */
int
get_headers_len(const char *buf, size_t buflen)
{
	const char	*s, *e;
	int		len = 0;

	for (s = buf, e = s + buflen - 1; len == 0 && s < e; s++)
		/* Control characters are not allowed but >=128 is. */
		if (!isprint(*(unsigned char *)s) && *s != '\r' && *s != '\n' && *(unsigned char *)s < 128)
			len = -1;
		else if (s[0] == '\n' && s[1] == '\n')
			len = s - buf + 2;
		else if (s[0] == '\n' && &s[1] < e &&
		    s[1] == '\r' && s[2] == '\n')
			len = s - buf + 3;

	return (len);
}
#define APPLICATION_NAME	"letv"
#define DMCLOG_D(log_fmt, log_arg...) \
    do{ \
		struct timeval tnow;\
				gettimeofday(&tnow, NULL);\
				printf( "[%s][%08d.%06d][%s:%d][%s][DEBUG] " log_fmt "\n", APPLICATION_NAME,(unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec,\
							 __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
    } while (0)
    
int http_tcpclient_recv(http_tcpclient *pclient,char **lpbuff,int size)
{
	//none block mode  recv

	char buff[BUFFER_SIZE]="\0";
	int rs=1;
	*lpbuff = NULL;
	int recvnum=0;
	int times=0;
	while(rs)
	{
		ssize_t buflen = recv(pclient->socket, buff,BUFFER_SIZE,0);
		if(buflen < 0)
		{
		// 由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读
		// 在这里就当作是该次事件已处理
			if(errno == EAGAIN||errno == EWOULDBLOCK||errno == EINTR)
				{
				//p_debug("no data now, pleas wait");//等待接收
				usleep(10000);
				times++;
				if(times>1500){//5s--500
						DMCLOG_D("wait time out...,recvnum=%d",recvnum);
						p_debug("iii=%s",*lpbuff);
						return recvnum;
					}
				else 
					continue;
			}
			else{
				//p_debug("no data now, pleas wait");
				usleep(10000);
				times++;
				if(times>500) 
					return recvnum;
				else 
					continue;
				return -1;
			}
		}
		else if(buflen == 0)
		{
			// 这里表示对端的socket已正常关闭.
			printf("socket close nomally\n");
         	return recvnum;
		}
		times=0;

		recvnum+=buflen;
		if(*lpbuff == NULL){
			p_debug("access1\n");
			*lpbuff = (char*)malloc(buflen);
			if(*lpbuff == NULL)
				return -2;
		}else{
			p_debug("access2\n");
			*lpbuff = (char*)realloc(*lpbuff,recvnum);
			if(*lpbuff == NULL)
				return -2;
		}
		memcpy(*lpbuff+recvnum-buflen,buff,buflen);
	//	p_debug("lpbuff=%s",*lpbuff);
	}

	return recvnum;
#if 0 //block mode  recv
	int recvnum=0,tmpres=0;
	char buff[BUFFER_SIZE]="\0";
	int enRet = 0;
	*lpbuff = NULL;
	while(recvnum < size || size==0){
		memset(buff,0,BUFFER_SIZE);
		//p_debug("pclient->socket = %d\n",pclient->socket);
		if ((enRet = DM_WaitDataAvailable(pclient->socket, pclient->time_out)) != MSGRET_SUCCESS)
		{
			p_debug("enRet = %d\n",enRet);
			break;
		}
		memset(buff,0,BUFFER_SIZE);
		tmpres = recv(pclient->socket,buff,BUFFER_SIZE,0);
		//p_debug("buff = %d,%s\n",strlen(buff),buff);
		if(tmpres <= 5&&*buff <= 0)
		{
			p_debug("break\n");
			break;
		}
		recvnum += tmpres;
		if(*lpbuff == NULL){
			//p_debug("access1\n");
			*lpbuff = (char*)malloc(recvnum);
			if(*lpbuff == NULL)
				return -2;
		}else{
			//p_debug("access2\n");
			*lpbuff = (char*)realloc(*lpbuff,recvnum);
			if(*lpbuff == NULL)
				return -2;
		}
		memcpy(*lpbuff+recvnum-tmpres,buff,tmpres);
	}
	return recvnum;
#endif
}

int http_tcpclient_recv_re_url(http_tcpclient *pclient,char **lpbuff,int size)
{
	//none block mode  recv

	char buff[BUFFER_SIZE]="\0";
	int rs=1;
	*lpbuff = NULL;
	int recvnum=0;
	int times=0;
	while(rs)
	{
		ssize_t buflen = recv(pclient->socket, buff,BUFFER_SIZE,0);
		if(buflen < 0)
		{
		// 由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读
		// 在这里就当作是该次事件已处理
			if(errno == EAGAIN||errno == EWOULDBLOCK||errno == EINTR)
				{
				//p_debug("no data now, pleas wait");//等待接收
				usleep(10000);
				times++;
				if(times>1500){
						p_debug("wait time out...");
						return recvnum;
					}
				else 
					continue;
			}
			else{
				//p_debug("no data now, pleas wait");
				usleep(10000);
				times++;
				if(times>1000) 
					return recvnum;
				else 
					continue;
				return -1;
			}
		}
		else if(buflen == 0)
		{
			// 这里表示对端的socket已正常关闭.
			printf("socket close nomally\n");
         	return recvnum;
		}
		times=0;
		if(buflen != sizeof(buff))
			rs = 0;
		else{
			rs = 1;// 需要再次读取
		}
		recvnum+=buflen;
		if(*lpbuff == NULL){
			p_debug("access1\n");
			*lpbuff = (char*)malloc(buflen);
			if(*lpbuff == NULL)
				return -2;
		}else{
			p_debug("access2\n");
			*lpbuff = (char*)realloc(*lpbuff,recvnum);
			if(*lpbuff == NULL)
				return -2;
		}
		memcpy(*lpbuff+recvnum-buflen,buff,buflen);
		//p_debug("lpbuff=%s",*lpbuff);
	}

	return recvnum;

#if 0
	int recvnum=0,tmpres=0;
	char buff[BUFFER_SIZE]="\0";
	int enRet = 0;
	*lpbuff = NULL;
	while(recvnum < size || size==0){
		memset(buff,0,BUFFER_SIZE);
		/*
		if ((enRet = DM_WaitDataAvailable(pclient->socket, pclient->time_out)) != MSGRET_SUCCESS)
		{
			printf("enRet = %d\n",enRet);
			break;
		}*/
		memset(buff,0,BUFFER_SIZE);
		tmpres = recv(pclient->socket,buff,BUFFER_SIZE,0);
		//p_debug("buff = %d,%s\n",strlen(buff),buff);

		if(tmpres <= 5 && *buff <= 0)
		{
			p_debug("break\n");
			break;
		}
		recvnum += tmpres;
		if(*lpbuff == NULL){
			p_debug("access1\n");
			*lpbuff = (char*)malloc(recvnum);
			if(*lpbuff == NULL)
				return -2;
		}else{
			p_debug("access2\n");
			*lpbuff = (char*)realloc(*lpbuff,recvnum);
			if(*lpbuff == NULL)
				return -2;
		}
		memcpy(*lpbuff+recvnum-tmpres,buff,tmpres);
	}
	//p_debug("recvnum = %d", recvnum);
	return recvnum;
#endif
}

#define MIN_DISK_FREE_SIZE 2097152 // 20MB

long long getDiskSize(){
	p_debug("access getDiskSize");
	FILE *read_fp = NULL;
	char availableSize[64]={0};
	long long freeSize=0;
	read_fp = popen("df |grep USB-disk|awk -F \' \' \'{print $4}\'", "r");
	if(read_fp != NULL)
	{
		if(fgets(availableSize,63, read_fp)!=NULL){
			p_debug("available_size=(%s)",availableSize);
			freeSize=atoi(availableSize);
			p_debug("freeSize=%lld",freeSize);
		}
		pclose(read_fp);
		p_debug("freeSize2=%lld",freeSize<<10);
		return (freeSize<<10);
	}
	return 0;
}
int http_get_video_size(http_tcpclient *pclient){
	int recvnum=0,tmpres=0,req_len,uri_len;
	char buff[DM_DOWN_PERSIZE]="\0";
	int enRet = 0;
	int ret = 0;	
	int i = 0;	
	char *p=NULL;
	char *e=NULL;
	char *start=NULL;
	char *end_number=NULL;

	long long per_bytes = 0;
    long long already_bytes = 0;
    //size_t cur_position = 0;
	long long total_size = 0;
	long long downloaded_size = 0;
	int http_status = 0;
	char http_status_str[16]="\0";
	int breath_flag=0;
	/*
	if ((enRet = DM_WaitDataAvailable(pclient->socket, pclient->time_out)) != MSGRET_SUCCESS)
	{
		printf("enRet = %d\n",enRet);
		return enRet;
	}*/
	//sleep(5);
	int rs=1;
	int times=0;
	while(rs)
	{
		tmpres = recv(pclient->socket, buff,DM_DOWN_PERSIZE,0);
		if(tmpres < 0)
		{
		// 由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读
		// 在这里就当作是该次事件已处理
			if(errno == EAGAIN||errno == EWOULDBLOCK||errno == EINTR)
				{
				p_debug("no data now, pleas wait");//等待接收
				usleep(10000);
				times++;
				if(times>1000){
						p_debug("wait time out...");
						return recvnum;
					}
				else 
					continue;
			}
			else{
				p_debug("no data now, pleas wait2");
				usleep(10000);
				times++;
				if(times>1000) 
					return recvnum;
				else 
					continue;
				return -1;
			}
		}
		else if(tmpres == 0)
		{
			// 这里表示对端的socket已正常关闭.
			printf("socket close nomally\n");
			return recvnum;
		}
		else {//get data !!!!
			if(tmpres<16) continue;
			p_debug("first read buf=(%s),strlen(buff)=%d",buff,strlen(buff));
			break;
		}

	}
	parse_headers(buff,	sizeof(buff), &pclient->ch);
	
	p_debug("pclient->dn->total_size = %lld", pclient->dn->total_size); 
	if(pclient->dn->downloaded_size == 0)
				pclient->dn->total_size = pclient->ch.cl.v_big_int;

	p_debug("pclient->dn->total_size = %lld", pclient->dn->total_size); 
	//safe_free(pclient->request);
	write_list_to_file(VIDEO);
	saveOneTaskToFile(pclient->dn);
	updateVer(0);
	return 0;
}


int get_md5sum(char *src,char *result,int len){
	int i = 0;
	yasm_md5_context context;
	unsigned char checksum[16]="\0";;
	char tmp[64]="\0";
	if(result == NULL || src == NULL)
	{
		return -1;
	}
	
	memset(checksum, 0, 16);
    yasm_md5_init (&context);
	yasm_md5_update (&context, src, strlen (src));
	yasm_md5_final (checksum, &context);
	memset(tmp, 0, 64);
	for(i = 0; i < 16; i++)
	{
		//p_debug ("%02x", (unsigned int) checksum[i]);
 		sprintf(tmp+2*i, "%02x", (unsigned int) checksum[i]);
	}
	//printf("result=%s\n",tmp);
	//strncpy(result, tmp,len);
	strncpy(result, tmp,len);
	result[len]='\0';
//	printf("result2=%s\n",result);
	return 0;
}

unsigned int str_to_uint(char *s){

	//p_debug("s=%s",s);
	if(strlen(s) != 8) return -1;

	unsigned int ui[8];
	int i=0;
	for(i=0;i<8;i++){
		if(s[i]>='a')
			ui[i]=s[i]-87;
		else
			ui[i]=s[i]-'0';
		
		//p_debug("s=%c,ui[%d]=%x",s[i],i,ui[i]);
		
	}
	//p_debug("ui[0]=%x",ui[0]<<28);
	//p_debug("ui[1]=%x",ui[1]<<24);
	//p_debug("ui[2]=%x",ui[2]<<20);	
	unsigned int uin=ui[0]<<28|ui[1]<<24|ui[2]<<20|ui[3]<<16|ui[4]<<12|ui[5]<<8|ui[6]<<4|ui[7];
	//p_debug("uin=%x",uin);

	return uin;
}
#define AES_KEY_LEN 32

int gen_k(char *k){
//	char *mac="84:5d:d7:00:11:22";
	char *mac="84:5D:D7:A1:12:27";
	char *sn="84:5D:D7:A1:12:27";
	int i=0;
//	char *k[16]={0};
	get_md5sum(mac,k,AES_KEY_LEN);
	p_debug("k=%s",k);
	//unsigned int key[4] = {0xd55fda32,0x14d99bd7,0x9f7a0ef8,0x972df216};
	char tmp[8]={0};
	unsigned int key[4];

	p_debug("int %d",sizeof(int));
	p_debug("char %d",sizeof(char));
	p_debug("long %d",sizeof(long));		
	p_debug("uint %d",sizeof(unsigned int));		

	char *p=k;
	for(i=0;i<4;i++){
		strncpy(tmp,p,8);
		tmp[8]='\0';
		p=p+8;
		key[i]=str_to_uint(tmp);
		p_debug("k[%d]=%x",i,key[i]);
	}

}

int http_tcpclient_download(http_tcpclient *pclient, int type)
{
	int recvnum=0,tmpres=0,req_len,uri_len;
	char buff[DM_DOWN_PERSIZE]="\0";
	int enRet = 0;
	int ret = 0;	
	int i = 0;	
	int j = 0;		
	char *p=NULL;
	char *e=NULL;
	char *start=NULL;
	char *end_number=NULL;
	long long fsize=0;
	long long per_bytes = 0;
    long long already_bytes = 0;
    //size_t cur_position = 0;
	long long total_size = 0;
	long long downloaded_size = 0;
	int http_status = 0;
	char http_status_str[16]="\0";
	int breath_flag=0;
	/*
	if ((enRet = DM_WaitDataAvailable(pclient->socket, pclient->time_out)) != MSGRET_SUCCESS)
	{
		printf("enRet = %d\n",enRet);
		return enRet;
	}*/
	//sleep(5);
	int rs=1;
	int times=0;
	while(rs)
	{
		tmpres = recv(pclient->socket, buff,DM_DOWN_PERSIZE,0);
		if(tmpres < 0)
		{
		// 由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读
		// 在这里就当作是该次事件已处理
			if(errno == EAGAIN||errno == EWOULDBLOCK||errno == EINTR)
				{
				//p_debug("no data now, pleas wait");//等待接收
				usleep(10000);
				times++;
				if(times>1500){
						p_debug("wait header time out...");
						return recvnum;
					}
				else 
					continue;
			}
			else{
				//p_debug("no data now, pleas wait2");
				usleep(10000);
				times++;
				if(times>1000) 
					return recvnum;
				else 
					continue;
				return -1;
			}
		}
		else if(tmpres == 0)
		{
			// 这里表示对端的socket已正常关闭.
			printf("socket close nomally\n");
			return recvnum;
		}
		else {//get data !!!!
			times=0;
			if(tmpres<5) continue;
			//p_debug("first read buf=(%s),strlen(buff)=%d",buff,strlen(buff));
			break;
		}

	}

	//p_debug("buff = %d,%s",strlen(buff),buff);
	req_len = get_headers_len(buff,tmpres);
	//p_debug("req_len=%d",req_len);
	if (req_len == 0)
		return -2;
	else if (req_len < MIN_REQ_LEN)
		return -3;
	else if ((pclient->request = my_strndup(buff, req_len)) == NULL)
		return -4;
	
	pclient->headers = memchr(pclient->request, '\n', req_len);
	assert(pclient->headers != NULL);
	assert(pclient->headers < pclient->request + req_len);
	if (pclient->headers > pclient->request && pclient->headers[-1] == '\r')
		pclient->headers[-1] = '\0';
	*pclient->headers++ = '\0';
	/*
	 * Now make a copy of the URI, because it will be URL-decoded,
	 * and we need a copy of unmodified URI for the access log.
	 * First, we skip the REQUEST_METHOD and shift to the URI.
	 */

	p = pclient->request;
	
	//p=buff;
	//pclient->headers=buff;

	//p_debug("pclient->request=%s",pclient->request);
	/* Now comes the HTTP-Version in the form HTTP/<major>.<minor> */
	if (strncmp(p, "HTTP/", 5) != 0) {
		return -5;
	}
	p += 5;
	/* Parse the HTTP major version number */
	pclient->major_version = strtoul(p, &end_number, 10);
	if (end_number == p || *end_number != '.') {
		return -6;
	}
	
	/* Parse the minor version number */
	p = end_number + 1;
	pclient->minor_version = strtoul(p, &end_number, 10);
	if (end_number == p || *end_number != ' ') {
		return -7;
	}
	/* Version must be <=1.1 */
	if (pclient->major_version > 1 ||
	    (pclient->major_version == 1 && pclient->minor_version > 1)) {
		return -8;
	}

	/* get http status */
	p = end_number + 1;
	end_number = strchr(p, ' ');
	if(end_number != NULL)
	{
		memset(http_status_str, 0, 16);
		memcpy(http_status_str, p, end_number - p);
		http_status = atoi(http_status_str);
		p_debug("http_status = %d", http_status);
		if((http_status == 200) || (http_status == 206)){

		}else {
			//p_debug("http status error");
			if(type==VIDEO) {
				if(pclient->dn->download_status==DOWNLOADING) pclient->dn->download_status=RETRY_DOWNLOAD;;
				pclient->dn->errorCode=RETRY_DOWNLOAD;;
				strcpy(pclient->dn->error_msg,"Get Download URL Failed.");
				updateVer(0);
			}
			safe_close(pclient->local_fd);
			safe_free(pclient->request);	
			return 0;
		}
	}

	if(type==VIDEO)
	{	


		parse_headers(pclient->headers,
		    (pclient->request + req_len) - pclient->headers, &pclient->ch);
		
//#define letv_encrypt


		//pclient->dn->downloaded_size = 0;

		/* Remove the length of request from total, count only data */
		if(pclient->dn->downloaded_size == 0)
			pclient->dn->total_size = pclient->ch.cl.v_big_int;
		
		p_debug("pclient->dn->total_size = %lld", pclient->dn->total_size);	
		//dm_create_local_file(pclient);
		if(pclient->dn->downloaded_size >= pclient->dn->total_size) 
		{
			safe_free(pclient->request);
			pclient->dn->download_status=DONE;
			return 0;
		}
		if(getDiskSize()<=(pclient->dn->total_size-pclient->dn->downloaded_size+MIN_DISK_FREE_SIZE))
			{
			safe_free(pclient->request);
			pclient->dn->download_status=DISK_UNWRITEABLE;
			return -9;
		}
		if((dm_create_local_file(pclient)<0)){
			//safe_close(pclient->local_fd);
			safe_free(pclient->request);
			pclient->dn->download_status=DISK_UNWRITEABLE;
			return -9;
		}
				char *k=(char*)malloc(AES_KEY_LEN);
#ifdef letv_encrypt	
				
		#define AES_BLOCK_SIZE	16
		
		unsigned char input[DM_DOWN_PERSIZE];
		unsigned char output[DM_DOWN_PERSIZE];
		unsigned int align_len = 0;
		unsigned int pad, len, i;
		unsigned int key[4] = {0xd55fda32,0x14d99bd7,0x9f7a0ef8,0x972df216};
		unsigned int iv[4] = {0x7f444fd5,0xd2002d29,0x4b96c34d,0xc57d297e};

		//char *k=(char*)malloc(AES_KEY_LEN);
		memset(k,0,AES_KEY_LEN);
		gen_k(k);
		struct aes_key aes_key;
		struct aes_data aes_data;
		aes_key.key = (char *)k;
		aes_key.keylen = 16;
		aes_key.bitmode = AES_KEY_128BIT;
		aes_key.aesmode = AES_MODE_ECB;
		aes_key.iv = (char *)iv;
		aes_key.ivlen = 16;
		aes_key.encrypt = 0;
p_debug("k=%s",k);

		long long file_len=pclient->dn->total_size;
#endif

		
		lseek(pclient->local_fd,pclient->dn->downloaded_size,SEEK_SET);
		already_bytes = pclient->dn->downloaded_size;

#ifdef letv_encrypt

		int aes_fd;
		int fd = open("/dev/jz-aes", O_RDWR);
		if(fd < 0) {
			p_debug("open jz-aes error!");
			return -1;
		}
		aes_fd = fd;
		ret = ioctl(aes_fd, AES_LOAD_KEY, &aes_key);
		if(ret < 0) {
			p_debug("ioctl! AES_LOAD_KEY");
		}

		p_debug("req_len=%d,tmpres=%d",req_len,tmpres);

		aes_data.input = (char*)(buff + req_len);
		aes_data.input_len = tmpres - req_len;
		aes_data.output = (char*)output;
		
		ret = ioctl(aes_fd, AES_DO_CRYPT, &aes_data);
		//ret = fwrite(output, 1, (tmpres - req_len), pclient->local_fd);
		ret = write(pclient->local_fd,output,(tmpres - req_len));		
		if (ret<0) {
			p_debug("ret=%d,write video failed.",ret);		
			safe_close(pclient->local_fd);
			safe_free(pclient->request);	
			safe_free(k);
			return ret;
		}
		file_len -= (tmpres-req_len);
		p_debug("file_len=%d",file_len);		
#else

		ret=write(pclient->local_fd,buff + req_len,tmpres - req_len);
		if(ret<0){//-1 write failed.
			p_debug("ret=%d,write video failed.",ret);		
			safe_close(pclient->local_fd);
			safe_free(pclient->request);	
			safe_free(k);
			return ret;
		}

#endif
		
		already_bytes += (tmpres - req_len);
		p_debug("6666 already_bytes = %lld", already_bytes);
		char percent[8]="\0";
		long long download_size=0;
		do{
			ssize_t buflen = recv(pclient->socket, buff,DM_DOWN_PERSIZE,0);
			if(buflen < 0)
			{
			// 由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读
			// 在这里就当作是该次事件已处理
				if(errno == EAGAIN||errno == EWOULDBLOCK||errno == EINTR)
					{
					//p_debug("no data now, pleas wait");//等待接收
					usleep(10000);
					times++;
					if(times>3000){//30 秒
							p_debug("wait video stream time out...");
							safe_close(pclient->local_fd);
							safe_free(pclient->request);	
							return recvnum;
						}
					else 
						continue;
				}
				else{
					//p_debug("no data now, pleas wait");
					usleep(10000);
					times++;
					if(times>1000) 
						return recvnum;
					else 
						continue;
					return -1;
				}
			}
			else if(buflen == 0)
			{
				// 这里表示对端的socket已正常关闭.
				printf("socket close nomally\n");
				safe_close(pclient->local_fd);
				safe_free(pclient->request);
				return recvnum;
			}
			times=0;
			//system("mcu_control -s 4");// 1s breath
			//updateSysVal("led_status","4");
			
			if(buflen != DM_DOWN_PERSIZE){	
				//p_debug("read finished...buflen=%d",buflen);	
				rs = 0;
			}
			else{
				//p_debug("read again...");
				rs = 1;// 需要再次读取
			}
			recvnum+=buflen;
			

#ifdef letv_encrypt
			if(!(file_len-buflen)){//last read
				align_len = ((buflen / AES_BLOCK_SIZE) + 1) * AES_BLOCK_SIZE;
				/* PKCS5Padding */
				pad = AES_BLOCK_SIZE - buflen % AES_BLOCK_SIZE;
				for (i = buflen; i < align_len; i++) {
					input[i] = pad;
	//				printf("PKCS5:input[%d]=%02x ",i,input[i]);
				}
	//			printf("\n");
	
				aes_data.input = buff;
				aes_data.input_len = buflen + pad;
				aes_data.output = output;
	
				ret = ioctl(aes_fd, AES_DO_CRYPT, &aes_data);
	
				//ret = fwrite(output, 1, buflen + pad, pclient->local_fd);
				ret = write( pclient->local_fd,output,buflen + pad);
				if (ret<0) {
					printf("last fwrite! error\n");
					safe_free(k);
					return -1;
				}

			}else{

				aes_data.input = buff;
				aes_data.input_len = buflen;
				aes_data.output = output;
				ret = ioctl(aes_fd, AES_DO_CRYPT, &aes_data);
				ret = write(pclient->local_fd,output,buflen);
				if (!ret) {
					p_debug("ret=%d,write video failed.",ret);		
					safe_close(pclient->local_fd);
					safe_free(pclient->request);	
					safe_free(k);
					return ret;
				}
				file_len -= buflen;
			}
			safe_free(k);
#else
			ret=write(pclient->local_fd,buff,buflen);			
			if(ret<0){//-1 write failed.
				p_debug("ret=%d,write video failed.",ret);		
				safe_close(pclient->local_fd);
				safe_free(pclient->request);						
				return ret;
			}
#endif			
			already_bytes += buflen;
			download_size +=buflen;
			pclient->dn->downloaded_size=already_bytes;
			//p_debug("download_size = %lld", download_size);
			//p_debug("already_bytes = %ld", already_bytes);
			//lseek(pclient->local_fd,already_bytes,SEEK_SET);

			if(type==VIDEO&&(i>=200)){
				i=0;
				
				//float pr=0;
				if(pclient->dn->total_size!=0) pclient->dn->percent = (float)pclient->dn->downloaded_size/(float)pclient->dn->total_size*100;
				else pclient->dn->percent=0;
				p_debug("P=%s,V=%s loading..[%lld]..%lld/%lld,[[[[%.2f%%]]]]",pclient->dn->pid,pclient->dn->vid,per_bytes,pclient->dn->downloaded_size,pclient->dn->total_size,pclient->dn->percent);
				//p_debug("download_status1===================%d",pclient->dn->download_status);
				j++;
				if(j>10){
					j=0;
					//system("cp -f /tmp/mnt/USB-disk-1/letv/.tasklist /tmp/mnt/USB-disk-1/letv/.tasklist.bak");
					//usleep(100000);						
					//system("mcu_control -s 4");// 1s breath
					//updateSysVal("led_status","4");
				}
				if(!breath_flag){
					breath_flag=1;
					updateSysVal("isDownloading","true");
					//system("mcu_control -s 4");// 1s breath
					updateSysVal("led_status","4");
				}
				//p_debug("download_status2===================%d",pclient->dn->download_status);
				double speed=0;
				time_t tt=(time(NULL)-pclient->dn->update_task_time);
				
				if(tt > 0 ){
					if(download_size < 512) speed=1;
					else if (download_size < 1024) speed=1;
					else speed=(double)(download_size>>10)/(double)(tt);
					p_debug("t=%d,download_size=%lld,speed=%f",tt,download_size,speed);
							
			
					
					//p_debug("download_status4===================%d",pclient->dn->download_status);
					char sp[16]="\0";
					sprintf(sp,"%.0f",speed);
					//printf("download_status4===================%d,speed=%skb/s\n",pclient->dn->download_status,sp);
					p_debug("download_status===================%lld,speed=%skb/s",download_size,sp);
					updateSysVal("speed",sp);
					//p_debug("download_status4===================%d,speed=%skb/s",pclient->dn->download_status,sp);
					sprintf(percent,"%.0f",pclient->dn->percent);
					updateSysVal("percent",percent);
					//p_debug("download_status4===================%d",pclient->dn->download_status);
			
					char progress[32]="\0";
					char totalSize[32]="\0";
					sprintf(progress,"%lld",pclient->dn->downloaded_size);
					sprintf(totalSize,"%lld",pclient->dn->total_size);
			
					updateSysVal("progress",progress);
					updateSysVal("totalSize",totalSize);
					pclient->dn->update_task_time=time(NULL);
					//p_debug("download_status3===================%d",pclient->dn->download_status);
					updateVer(0);
			
					update_task_to_list(pclient->dn);
					download_size=0;
				}
			}
			i++;			
		}while( (pclient->dn->downloaded_size < pclient->dn->total_size)&&(pclient->dn->download_status == DOWNLOADING)&&(download_img_first==0));
		p_debug("download_img_first=%d",download_img_first);
		//ctx=uci_alloc_context();

		//p_debug("download_status===================%d",pclient->dn->download_status);
		pclient->dn->downloaded_size=already_bytes;

		if(pclient->dn->total_size!=0) pclient->dn->percent = (float)pclient->dn->downloaded_size/(float)pclient->dn->total_size*100;
		else pclient->dn->percent=0;
		p_debug("P=%s,V=%s loading..[%lld]..%lld/%lld,[[[[%.2f%%]]]]",pclient->dn->pid,pclient->dn->vid,per_bytes,pclient->dn->downloaded_size,pclient->dn->total_size,pclient->dn->percent);
		if(!breath_flag){
			breath_flag=1;
//			system("mcu_control -s 4");// 1s breath
			system("pwm_control 1 0 0;pwm_control 0 0 10000");		
			updateSysVal("led_status","4");
		}
		updateSysVal("speed","0");
		char progress[32]="\0";
		char totalSize[32]="\0";
		sprintf(progress,"%lld",pclient->dn->downloaded_size);
		sprintf(totalSize,"%lld",pclient->dn->total_size);		
		updateSysVal("progress",progress);
		updateSysVal("totalSize",totalSize);

		sprintf(percent,"%.0f",pclient->dn->percent);
		
		updateSysVal("percent",percent);
		pclient->dn->update_task_time=time(NULL);
		update_task_to_list(pclient->dn);
		
	}
	else//image or firmware.
	{
		parse_headers(pclient->headers,
		    (pclient->request + req_len) - pclient->headers, &pclient->ch);
		/* Remove the length of request from total, count only data */
		total_size = pclient->ch.cl.v_big_int;
		p_debug("total_size = %lld", total_size);	
		//p_debug("pclient->dn->total_img_size = %lld", total_size);	
		if(type==VIDEO_IMAGE){
			pclient->dn->total_img_size=total_size;
			p_debug("pclient->dn->total_img_size = %lld", total_size);	
		}
		if(type != FIRMWARE){
			if(getDiskSize()<=MIN_DISK_FREE_SIZE)
				{
				safe_free(pclient->request);
				pclient->dn->download_status=DISK_UNWRITEABLE;
				return -9;
			}
			if(dm_create_local_file(pclient)<0){
				//safe_close(pclient->local_fd);
				safe_free(pclient->request);
				pclient->dn->download_status=DISK_UNWRITEABLE;
				return;
			}
			p_debug("image already_bytes = %lld", already_bytes);
			lseek(pclient->local_fd,already_bytes,SEEK_SET);
		}
		else
		{
			if(dm_create_local_file(pclient)<0){
				//safe_close(pclient->local_fd);
				safe_free(pclient->request);
				//pclient->dn->download_status=DISK_UNWRITEABLE;
				return;
			}
			
			//lseek(pclient->local_fd,pclient->dn->downloaded_size,SEEK_SET);
			//FILE *fp=fopen(,O_CREAT|O_WRONLY,0644);
			fsize= lseek(pclient->local_fd, 0, SEEK_END);
			p_debug("fsize=%lld",fsize);
			lseek(pclient->local_fd,fsize,SEEK_SET);
			//already_bytes=fsize;
			p_debug("FW already_bytes = %lld", fsize);
			//rewind(fd);
			//fclose(fd);
		}		
	
		
		write(pclient->local_fd,buff + req_len,tmpres - req_len);		
		already_bytes += (tmpres - req_len);
		rs=1;
		do{
			ssize_t buflen = recv(pclient->socket, buff,DM_DOWN_PERSIZE,0);
			if(buflen < 0)
			{
			// 由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读
			// 在这里就当作是该次事件已处理
				if(errno == EAGAIN||errno == EWOULDBLOCK||errno == EINTR)
					{
					//p_debug("no data now, pleas wait");//等待接收
					usleep(10000);
					times++;
					if(times>1000){
							p_debug("wait time out...");
							safe_close(pclient->local_fd);
							safe_free(pclient->request);							
							return recvnum;
						}
					else 
						continue;
				}
				else{
					//p_debug("no data now, pleas wait");
					usleep(10000);
					times++;
					if(times>300) 
						return recvnum;
					else 
						continue;
					return -1;
				}
			}
			else if(buflen == 0)
			{
				// 这里表示对端的socket已正常关闭.
				printf("socket close nomally\n");
				return recvnum;
			}
			times=0;

			recvnum+=buflen;

			p_debug("already_bytes = %ld", already_bytes);

			ret=write(pclient->local_fd,buff,buflen);
		
			if(ret<0){//-1 write failed.
				p_debug("ret=%d,write image failed.",ret);		
				safe_close(pclient->local_fd);
				safe_free(pclient->request);						
				return ret;
			}
			already_bytes += buflen;
			//downloaded_size +=buflen;
			if(type==VIDEO_IMAGE){
				pclient->dn->download_img_size=already_bytes;	
				lseek(pclient->local_fd,already_bytes,SEEK_SET);				
			}else if(type==FIRMWARE){
				lseek(pclient->local_fd,already_bytes+fsize,SEEK_SET);
			}else 
				lseek(pclient->local_fd,already_bytes,SEEK_SET);

		}while((already_bytes < total_size));
		 update_task_to_list(pclient->dn);
	}

    p_debug("already_bytes end = %lld",already_bytes);
	safe_close(pclient->local_fd);
	safe_free(pclient->request);
	if((type==FIRMWARE) && (already_bytes!= total_size)) 
		return -2;
	return already_bytes;
}

#if 0// block mode
int http_tcpclient_download(http_tcpclient *pclient, int type)
{
	int recvnum=0,tmpres=0,req_len,uri_len;
	char buff[DM_DOWN_PERSIZE]="\0";
	int enRet = 0;
	int ret = 0;	
	int i = 0;	
	char *p=NULL;
	char *e=NULL;
	char *start=NULL;
	char *end_number=NULL;

	long long per_bytes = 0;
    long long already_bytes = 0;
    //size_t cur_position = 0;
	long long total_size = 0;
	long long downloaded_size = 0;
	int http_status = 0;
	char http_status_str[16]="\0";
	int breath_flag=0;
	/*
	if ((enRet = DM_WaitDataAvailable(pclient->socket, pclient->time_out)) != MSGRET_SUCCESS)
	{
		printf("enRet = %d\n",enRet);
		return enRet;
	}*/
	//sleep(5);
	tmpres = recv(pclient->socket,buff,BUFFER_SIZE,0);
	 if ((tmpres == 0) ||
        ((tmpres == -1) && (tmpres == 131)))  /* new 2.6.21 kernel seems to give us this before rc==0 */
    {
        return MSGRET_DISCONNECTED;
    }
    else if (tmpres < 0)
    {
        p_debug("bad read, rc=%d errno=%d reason:%s",
               tmpres, errno, strerror(errno));
        return MSGRET_INTERNAL_ERROR;
    }
	//p_debug("buff = %d,%s",strlen(buff),buff);
	req_len = get_headers_len(buff,tmpres);

	if (req_len == 0)
		return -2;
	else if (req_len < MIN_REQ_LEN)
		return -3;
	else if ((pclient->request = my_strndup(buff, req_len)) == NULL)
		return -4;
	
	pclient->headers = memchr(pclient->request, '\n', req_len);
	assert(pclient->headers != NULL);
	assert(pclient->headers < pclient->request + req_len);
	if (pclient->headers > pclient->request && pclient->headers[-1] == '\r')
		pclient->headers[-1] = '\0';
	*pclient->headers++ = '\0';
	/*
	 * Now make a copy of the URI, because it will be URL-decoded,
	 * and we need a copy of unmodified URI for the access log.
	 * First, we skip the REQUEST_METHOD and shift to the URI.
	 */
	 #if 0
	for (p = pclient->request, e = p + req_len; *p != ' ' && p < e; p++);
	while (p < e && *p == ' ') p++;

	/* Now remember where URI starts, and shift to the end of URI */
	for (start = p; p < e && !isspace((unsigned char)*p); ) p++;
	uri_len = p - start;
	/* Skip space following the URI */
	while (p < e && *p == ' ') p++;
	#endif
	p = pclient->request;

	/* Now comes the HTTP-Version in the form HTTP/<major>.<minor> */
	if (strncmp(p, "HTTP/", 5) != 0) {
		return -5;
	}
	p += 5;
	/* Parse the HTTP major version number */
	pclient->major_version = strtoul(p, &end_number, 10);
	if (end_number == p || *end_number != '.') {
		return -6;
	}
	
	/* Parse the minor version number */
	p = end_number + 1;
	pclient->minor_version = strtoul(p, &end_number, 10);
	if (end_number == p || *end_number != ' ') {
		return -7;
	}
	/* Version must be <=1.1 */
	if (pclient->major_version > 1 ||
	    (pclient->major_version == 1 && pclient->minor_version > 1)) {
		return -8;
	}

	/* get http status */
	p = end_number + 1;
	end_number = strchr(p, ' ');
	if(end_number != NULL)
	{
		memset(http_status_str, 0, 16);
		memcpy(http_status_str, p, end_number - p);
		http_status = atoi(http_status_str);
		p_debug("http_status = %d", http_status);
		if((http_status == 200) || (http_status == 206)){

		}else {
			//p_debug("http status error");
			if(type==VIDEO) {
				if(pclient->dn->download_status==DOWNLOADING) pclient->dn->download_status=RETRY_DOWNLOAD;;
				pclient->dn->errorCode=RETRY_DOWNLOAD;;
				strcpy(pclient->dn->error_msg,"Get Download URL Failed.");
				updateVer(0);
			}
			safe_close(pclient->local_fd);
			safe_free(pclient->request);	
			return 0;
		}
	}

	if(type==VIDEO)
	{	
		parse_headers(pclient->headers,
		    (pclient->request + req_len) - pclient->headers, &pclient->ch);
		p_debug("pclient->dn->total_size = %lld", pclient->dn->total_size);	
		/* Remove the length of request from total, count only data */
		if(pclient->dn->downloaded_size == 0)
			pclient->dn->total_size = pclient->ch.cl.v_big_int;
		
		p_debug("pclient->dn->total_size = %lld", pclient->dn->total_size);	
		//dm_create_local_file(pclient);
		if(getDiskSize()<=(pclient->dn->total_size-pclient->dn->downloaded_size+MIN_DISK_FREE_SIZE))
			{
			safe_free(pclient->request);
			pclient->dn->download_status=DISK_UNWRITEABLE;
			return -9;
		}
		if((dm_create_local_file(pclient)<0)){
			//safe_close(pclient->local_fd);
			safe_free(pclient->request);
			pclient->dn->download_status=DISK_UNWRITEABLE;
			return -9;
		}
		lseek(pclient->local_fd,pclient->dn->downloaded_size,SEEK_SET);
		already_bytes = pclient->dn->downloaded_size;
		write(pclient->local_fd,buff + req_len,tmpres - req_len);
		already_bytes += (tmpres - req_len);
		//p_debug("6666 already_bytes = %lld", already_bytes);
		char percent[8]="\0";
		long long download_size=0;
		
		do {
			
            per_bytes = recv(pclient->socket, buff, DM_DOWN_PERSIZE, 0);
			download_size +=per_bytes;
			//p_debug("per_bytes = %d", per_bytes);
            if ( per_bytes > 0 ) {
				system("mcu_control -s 4");// 1s breath
				updateSysVal("led_status","4");
                already_bytes += per_bytes;
                if ( already_bytes > pclient->dn->total_size ) { 	/* 澶勭悊鏈�鍚庡浜哱r\n涓や釜瀛楄妭 */
                    per_bytes -= (already_bytes-pclient->dn->total_size);
                    already_bytes = pclient->dn->total_size;
                }
				
				
        		//p_debug("percent = %d\%", (already_bytes*100)/pclient->dn->total_size);
                ret=write(pclient->local_fd,buff,per_bytes);

				if(ret<0){
					p_debug("ret=%d,write failed.",ret);	
					safe_close(pclient->local_fd);
					safe_free(pclient->request);					
					return ret;
				}
				recvnum = already_bytes;
				pclient->dn->downloaded_size=already_bytes;
            } else if ((0 == per_bytes) ||
            	((per_bytes == -1) && (per_bytes == 131)))  /* new 2.6.21 kernel seems to give us this before rc==0 */ 
            {
                recvnum = MSGRET_DISCONNECTED;
                break;
            } else {
                if ( already_bytes < pclient->dn->total_size ) {
                    recvnum = MSGRET_INTERNAL_ERROR;
                    break;
                }
            }


			if(type==VIDEO&&(i>=400)){
				i=0;
				
				//float pr=0;
				if(pclient->dn->total_size!=0) pclient->dn->percent = (float)pclient->dn->downloaded_size/(float)pclient->dn->total_size*100;
				else pclient->dn->percent=0;
				p_debug("P=%s,V=%s loading..[%lld]..%lld/%lld,[[[[%.2f%%]]]]",pclient->dn->pid,pclient->dn->vid,per_bytes,pclient->dn->downloaded_size,pclient->dn->total_size,pclient->dn->percent);
				//p_debug("download_status1===================%d",pclient->dn->download_status);

				if(!breath_flag){
					breath_flag=1;
					system("mcu_control -s 4");// 1s breath
					updateSysVal("led_status","4");
				}
				//p_debug("download_status2===================%d",pclient->dn->download_status);
				double speed=0;
				time_t tt=(time(NULL)-pclient->dn->update_task_time);
				
				if(tt > 0 ){
					if(download_size < 512) speed=1;
					else if	(download_size < 1024) speed=1;
					else speed=(double)(download_size>>10)/(double)(tt);
					p_debug("download_size=%lld,speed=%f",download_size,speed);
					download_size=0;		

					
					//p_debug("download_status4===================%d",pclient->dn->download_status);
					char sp[16]="\0";
					sprintf(sp,"%.0f",speed);
					//printf("download_status4===================%d,speed=%skb/s\n",pclient->dn->download_status,sp);
					p_debug("download_status4===================%d,speed=%skb/s",pclient->dn->download_status,sp);
					updateSysVal("speed",sp);
					//p_debug("download_status4===================%d,speed=%skb/s",pclient->dn->download_status,sp);
					sprintf(percent,"%.0f",pclient->dn->percent);
					updateSysVal("percent",percent);
					//p_debug("download_status4===================%d",pclient->dn->download_status);

					char progress[32]="\0";
					char totalSize[32]="\0";
					sprintf(progress,"%lld",pclient->dn->downloaded_size);
					sprintf(totalSize,"%lld",pclient->dn->total_size);

					updateSysVal("progress",progress);
					updateSysVal("totalSize",totalSize);
					pclient->dn->update_task_time=time(NULL);
					//p_debug("download_status3===================%d",pclient->dn->download_status);
					updateVer(0);

					update_task_to_list(pclient->dn);
				}
			}
			i++;
			
          //  write_list_to_file(LETV_TASKLIST_FILE_PATH,task_dn);//鍐欏叆鏁版嵁
        }while ( (pclient->dn->downloaded_size < pclient->dn->total_size)&&(pclient->dn->download_status == DOWNLOADING)&&(download_img_first==0));
		p_debug("download_img_first=%d",download_img_first);
		//ctx=uci_alloc_context();

		//p_debug("download_status===================%d",pclient->dn->download_status);
		pclient->dn->downloaded_size=already_bytes;

		if(pclient->dn->total_size!=0) pclient->dn->percent = (float)pclient->dn->downloaded_size/(float)pclient->dn->total_size*100;
		else pclient->dn->percent=0;
		p_debug("P=%s,V=%s loading..[%lld]..%lld/%lld,[[[[%.2f%%]]]]",pclient->dn->pid,pclient->dn->vid,per_bytes,pclient->dn->downloaded_size,pclient->dn->total_size,pclient->dn->percent);
		if(!breath_flag){
			breath_flag=1;
			system("mcu_control -s 4");// 1s breath
			updateSysVal("led_status","4");
		}
		updateSysVal("speed","0");
		char progress[32]="\0";
		char totalSize[32]="\0";
		sprintf(progress,"%lld",pclient->dn->downloaded_size);
		sprintf(totalSize,"%lld",pclient->dn->total_size);		
		updateSysVal("progress",progress);
		updateSysVal("totalSize",totalSize);

		sprintf(percent,"%.0f",pclient->dn->percent);
		
		updateSysVal("percent",percent);
		pclient->dn->update_task_time=time(NULL);
		update_task_to_list(pclient->dn);
	}
	else//image or firmware.
	{
		parse_headers(pclient->headers,
		    (pclient->request + req_len) - pclient->headers, &pclient->ch);
		/* Remove the length of request from total, count only data */
		total_size = pclient->ch.cl.v_big_int;
		p_debug("total_size = %lld", total_size);	
		//p_debug("pclient->dn->total_img_size = %lld", total_size);	
		if(type==VIDEO_IMAGE){
			pclient->dn->total_img_size=total_size;
			p_debug("pclient->dn->total_img_size = %lld", total_size);	
		}
		if(type != FIRMWARE){
			if(getDiskSize()<=MIN_DISK_FREE_SIZE)
				{
				safe_free(pclient->request);
				pclient->dn->download_status=DISK_UNWRITEABLE;
				return -9;
			}
			if(dm_create_local_file(pclient)<0){
				//safe_close(pclient->local_fd);
				safe_free(pclient->request);
				pclient->dn->download_status=DISK_UNWRITEABLE;
				return;
			}
			p_debug("image already_bytes = %lld", already_bytes);
			lseek(pclient->local_fd,downloaded_size,SEEK_SET);
		}
		else
		{
			if(dm_create_local_file(pclient)<0){
				//safe_close(pclient->local_fd);
				safe_free(pclient->request);
				pclient->dn->download_status=DISK_UNWRITEABLE;
				return;
			}
			p_debug("FW already_bytes = %lld", already_bytes);
			//lseek(pclient->local_fd,pclient->dn->downloaded_size,SEEK_SET);
			//FILE *fp=fopen(,O_CREAT|O_WRONLY,0644);
			long long fsize= lseek(pclient->local_fd, 0, SEEK_END);

			lseek(pclient->local_fd,fsize,SEEK_SET);
			//rewind(fd);
			//fclose(fd);
		}		
	
		//already_bytes = downloaded_size;
		write(pclient->local_fd,buff + req_len,tmpres - req_len);		
		already_bytes += (tmpres - req_len);

		do {
			memset(buff, 0, DM_DOWN_PERSIZE);
            per_bytes = recv(pclient->socket, buff, DM_DOWN_PERSIZE, 0);
			//p_debug("per_bytes = %d", per_bytes);
            if ( per_bytes > 0 ) {
                already_bytes += per_bytes;
                if ( already_bytes > total_size ) { 	/* 澶勭悊鏈�鍚庡浜哱r\n涓や釜瀛楄妭 */
                    per_bytes -= (already_bytes-total_size);
                    already_bytes = total_size;
                }
				if(type==FIRMWARE) p_debug("FW downloaded_bytes = %lld", already_bytes);
        		//p_debug("percent = %d\%", (already_bytes*100)/pclient->dn->total_size);
                ret=write(pclient->local_fd,buff,per_bytes);
				if(ret<0){//-1 write failed.
					p_debug("ret=%d,write image failed.",ret);		
					safe_close(pclient->local_fd);
					safe_free(pclient->request);						
					return ret;
				}				
				recvnum = already_bytes;
				downloaded_size=already_bytes;
				if(type==VIDEO_IMAGE)pclient->dn->download_img_size=already_bytes;
            } else if ((0 == per_bytes) ||
            	((per_bytes == -1) && (per_bytes == 131)))  /* new 2.6.21 kernel seems to give us this before rc==0 */
            {
                recvnum = MSGRET_DISCONNECTED;
                break;
            } else {//time out and other error
				recvnum = MSGRET_INTERNAL_ERROR;
                break;
            }
          //  write_list_to_file(LETV_TASKLIST_FILE_PATH,task_dn);//鍐欏叆鏁版嵁
        }while (downloaded_size < total_size);
		p_debug("downloaded_size===================%lld",downloaded_size);
	}

    p_debug("already_bytes end = %lld",already_bytes);
	safe_close(pclient->local_fd);
	safe_free(pclient->request);
	if((type==FIRMWARE) && (already_bytes!= total_size)) return -2;
	return recvnum;
}

#endif

int http_tcpclient_getinfo(http_tcpclient *pclient, char **lpbuff)
{
	int recvnum=0,tmpres=0,req_len,uri_len;
	char buff[BUFFER_SIZE]="\0";
	char *recv_buff=NULL;
	int enRet = 0;
	char *p=NULL;
	char *e=NULL;
	char *start=NULL;
	char *end_number=NULL;
	long per_bytes = 0;
    unsigned long already_bytes = 0;
    unsigned long cur_position = 0;
	unsigned long total_size = 0;
	/*
	if ((enRet = DM_WaitDataAvailable(pclient->socket, pclient->time_out)) != MSGRET_SUCCESS)
	{
		p_debug("enRet = %d\n",enRet);
		return enRet;
	}*/
	memset(buff, 0, BUFFER_SIZE);
	tmpres = recv(pclient->socket,buff,BUFFER_SIZE,0);
	 if ((tmpres == 0) ||
        ((tmpres == -1) && (tmpres == 131)))  /* new 2.6.21 kernel seems to give us this before rc==0 */
    {
        return MSGRET_DISCONNECTED;
    }
    else if (tmpres < 0)
    {
        p_debug("bad read, rc=%d errno=%d reason:%s",
               tmpres, errno, strerror(errno));
        return MSGRET_INTERNAL_ERROR;
    }
	//p_debug("buff = %d,%s",strlen(buff),buff);

	req_len = get_headers_len(buff,tmpres);
	if (req_len == 0)
		return -1;
	else if (req_len < MIN_REQ_LEN)
		return -1;
	else if ((pclient->request = my_strndup(buff, req_len)) == NULL)
		return -1;
	//p_debug("000 req_len = %d", req_len);
	pclient->headers = memchr(pclient->request, '\n', req_len);
	assert(pclient->headers != NULL);
	assert(pclient->headers < pclient->request + req_len);
	if (pclient->headers > pclient->request && pclient->headers[-1] == '\r')
		pclient->headers[-1] = '\0';
	*pclient->headers++ = '\0';
	//p_debug("111 pclient->headers = %s\n pclient->request = %s", pclient->headers, pclient->request);	

	/* Now comes the HTTP-Version in the form HTTP/<major>.<minor> */
	p = pclient->request;
	if (strncmp(p, "HTTP/", 5) != 0) {
		return -1;
	}
	p += 5;
	//p_debug("777 p = %s", p);
	/* Parse the HTTP major version number */
	pclient->major_version = strtoul(p, &end_number, 10);
	if (end_number == p || *end_number != '.') {
		return -1;
	}
	p = end_number + 1;
	//p_debug("666 pclient->major_version = %d, p = %s", pclient->major_version, p);
	/* Parse the minor version number */
	pclient->minor_version = strtoul(p, &end_number, 10);
	//p_debug("end_number = %s, p = %s", end_number, p);
	//if (end_number == p || *end_number != '\0') {
	if (end_number == p || *end_number != ' ') {
		return -1;
	}
	//p_debug("333 pclient->minor_version = %d", pclient->minor_version);
	/* Version must be <=1.1 */
	if (pclient->major_version > 1 ||
	    (pclient->major_version == 1 && pclient->minor_version > 1)) {
		return -1;
	}

	
	p = strstr(buff, "\r\n\r\n");
	if(p != NULL){
		total_size = strtoul(p, &end_number, 16);
		if(end_number != NULL)
			end_number = end_number+2;
		//p_debug("p = %s,len = %d,end_number = %s", p, total_size, end_number);
	}
	else
	{
		return -1;
	}
	//p_debug("666 total_size = %d", total_size);
	already_bytes = tmpres - (end_number-buff);
	recv_buff = (char *)calloc(1, already_bytes);
	if(recv_buff == NULL)
		return -1;
	memcpy(recv_buff, end_number, already_bytes);
	//p_debug("777 recv_buff = %s , already_bytes = %d", recv_buff, already_bytes);
	do {
			memset(buff, 0, BUFFER_SIZE);
            per_bytes = recv(pclient->socket, buff, BUFFER_SIZE, 0);
			//p_debug("per_bytes = %d, buff = %s", per_bytes, buff);
            if ( per_bytes > 0 ) {
                already_bytes += per_bytes;
				recv_buff = (char *)realloc(recv_buff, already_bytes);
				if(recv_buff == NULL)
					return -1;
				strcat(recv_buff, buff);
				//p_debug("already_bytes = %d, recv_buff = %s", already_bytes, recv_buff);
                if ( already_bytes > total_size ) { 	/* 澶勭悊鏈�鍚庡浜哱r\n涓や釜瀛楄妭 */
                    per_bytes -= (already_bytes-total_size);
                    already_bytes = total_size;
                }
				recvnum = already_bytes;
               // write(pclient->local_fd,buff,per_bytes);
            } else if ((0 == per_bytes) ||
            	((per_bytes == -1) && (per_bytes == 131)))  /* new 2.6.21 kernel seems to give us this before rc==0 */
            {
                recvnum = MSGRET_DISCONNECTED;
                break;
            } else {//time out and other error
                if ( already_bytes < total_size ) {
                    recvnum = MSGRET_INTERNAL_ERROR;
                    break;
                }
            }
        }//while ((already_bytes < total_size) && (pclient->dn->download_status == DOWNLOADING));
        while ((already_bytes < total_size));
	*lpbuff = recv_buff;
	//p_debug("already_bytes end = %lld,total_size=%lld",already_bytes, total_size);
	return recvnum;
}

int http_tcpclient_send(http_tcpclient *pclient,char *buff,int size)
{
#if 0
	int sent=0,tmpres=0;
	while(sent < size){
		tmpres = send(pclient->socket,buff+sent,size-sent,0);
		//p_debug("send http_tcpclient_send = %d\n",tmpres);
		if(tmpres == -1){
			return -1;
		}
		sent += tmpres;
	}
	return sent;
#endif
#if 1
	int total=size;
	ssize_t tmp;
	const char *p=buff;

	while(1)
	{
		tmp=send(pclient->socket,p,total,0);
		if(tmp < 0)
	    {
	      // 当send收到信号时,可以继续写,但这里返回-1.
	      if(errno == EINTR)
		  	{
			  	printf("return %d",EINTR);
		        return -1;
			}

	      // 当socket是非阻塞时,如返回此错误,表示写缓冲队列已满,
	      // 在这里做延时后再重试.
	      if(errno == EAGAIN)
	      {
	        usleep(1000);
	        continue;
	      }

	      return -1;
	    }

	    if((size_t)tmp == total)
	      return size;

	    total -= tmp;
	    p += tmp;
	}
	return tmp;

#endif
	
}

int http_tcpclient_close(http_tcpclient *pclient)
{
	close(pclient->socket);
	pclient->connected = 0;
	return 0;
}

int http_get_info(http_tcpclient *pclient,char *page, char *request, char **response)
{
	p_debug("access http_get");
	char	*lpbuf=NULL;
	int		len;
	char	h_post[BUFFER_SIZE_1024]="\0";
	char 	h_host[128]="\0";
	memset(h_post,0,sizeof(h_post));
	memset(h_host,0,sizeof(h_host));

	const char *h_header="User-Agent: ZhuiJuShenQi\r\nCache-Control: max-age=0\r\nAccept: text/html,*/*;q=0.8\r\nAccept-Encoding: gzip, deflate, sdch\r\nAccept-Language: zh-CN,zh;q=0.8\r\nConnection: Keep-Alive\r\n"; 
	sprintf(h_post, "GET %s HTTP/1.1\r\n", page);
	sprintf(h_host, "HOST: %s:%d\r\n",pclient->host, pclient->remote_port);
	len = strlen(h_post)+strlen(h_host)+strlen(h_header);
	p_debug("len = %d\n",len);
	lpbuf = (char*)malloc(len);
	if(lpbuf==NULL){
		p_debug("Malloc error.\n");
		return -1;
	}
	strcpy(lpbuf,h_post);
	strcat(lpbuf,h_host);
	strcat(lpbuf,h_header);
	strcat(lpbuf,"\r\n");

	//p_debug("lpbuf = %s\n",lpbuf);
	if(http_tcpclient_send(pclient,lpbuf,len)<0){
		free(lpbuf);
		return -2;
	}
	if(lpbuf)
		free(lpbuf);

	if(http_tcpclient_getinfo(pclient, response) <= 0){
		p_debug("recv error\n");
		return -3;
	}
	//p_debug("*response = %s", *response);
	return 0;
}


int http_get_re_url(http_tcpclient *pclient,char *page, char *request, char **response)
{
	//p_debug("access http_get");
	char	*lpbuf;
	int	len;
	int ret = 0;
	char	h_post[BUFFER_SIZE]="\0";
	char 	h_host[HOST_IP_LEN_256]="\0";;
	memset(h_post,0,sizeof(h_post));
	memset(h_host,0,sizeof(h_host));

	const char *h_header="User-Agent: ZhuiJuShenQi\r\nCache-Control: max-age=0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nAccept-Encoding: gzip, deflate, sdch\r\nAccept-Language: zh-CN,zh;q=0.8\r\nConnection: Keep-Alive\r\n\r\n"; 
	sprintf(h_post, "GET %s HTTP/1.1\r\n", page);
	sprintf(h_host, "HOST: %s:%d\r\n",pclient->host, pclient->remote_port);

	len = strlen(h_header)+strlen(h_post)+strlen(h_host)+1;
	//p_debug("len = %d\n",len);
	lpbuf = (char*)calloc(1, len);
	//p_debug("len = %d\n",len);
	if(lpbuf==NULL){
		p_debug("Malloc error.\n");
		return -1;
	}
	//p_debug("len = %d\n",len);
	strcpy(lpbuf,h_post);
	//p_debug("len = %d\n",len);
	strcat(lpbuf,h_host);
	//p_debug("len1 = %d\n",len);
	strcat(lpbuf,h_header);
	//p_debug("len2 = %d\n",len);
	lpbuf[len]='\0';
	
	//p_debug("lpbuf = %s\n",lpbuf);
	if(http_tcpclient_send(pclient,lpbuf,len-1)<0){
		free(lpbuf);lpbuf=NULL;

		p_debug("http_tcpclient_send error.");
		return -2;
	}
	if(lpbuf)
		free(lpbuf);

	ret = http_tcpclient_recv_re_url(pclient, response, 0);
	if(ret <= 0 && ret != MSGRET_TIMED_OUT){
		p_debug("http_tcpclient_recv_re_url error.");
		return -3;
	}
	//p_debug("*response = %s", *response);
	return 0;
}
#define FILE_UPGRADE_PATH			"/tmp/mnt/USB-disk-1/ota/update.bin"


int http_get(http_tcpclient *pclient,char *page, char *request, char **response, int type)
{
	p_debug("access http_get");
	char *lpbuf=NULL;
//	char *ptmp=NULL;
//	char *end_tmp=NULL;
//	char *tmp=NULL;
	int	len;
	char	h_post[BUFFER_SIZE]="\0";
	char 	h_host[HOST_IP_LEN_256]="\0";
	char 	h_range[128]="\0";
	memset(h_post, 0, BUFFER_SIZE);
	memset(h_host, 0,  sizeof(h_host));
	memset(h_range , 0,  sizeof(h_range));

	const char *h_header="User-Agent: ZhuiJuShenQi\r\nCache-Control: max-age=0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nAccept-Encoding: gzip, deflate, sdch\r\nAccept-Language: zh-CN,zh;q=0.8\r\nConnection: Keep-Alive\r\n\r\n"; 
	sprintf(h_post, "GET %s HTTP/1.1\r\n", page);
	//p_debug("h_post = %s", h_post);
	sprintf(h_host, "HOST: %s:%d\r\n",pclient->host, pclient->remote_port);
	//p_debug("h_host = %s", h_host);
//	p_debug("downloaded_size = %lu", pclient->dn->downloaded_size);

	if(type == VIDEO)
	{ 
		if(pclient->dn->downloaded_size != 0) {
			sprintf(h_range, "Range:bytes=%lld-\r\n", pclient->dn->downloaded_size);		
		}
	}
	if(type==FIRMWARE){
		FILE *fd=fopen(FILE_UPGRADE_PATH,"rb");
		long long fsize=0;

		 if(fd == NULL){			
			fsize=0;
		 }else {
			fseek(fd, 0, SEEK_END);
			fsize= ftell(fd);
			rewind(fd);
			fclose(fd);
		 } 

		sprintf(h_range, "Range:bytes=%lld-\r\n", fsize);
		
	}

	
	len = strlen(h_header)+strlen(h_post)+strlen(h_host)+strlen(h_range)+1;
	//p_debug("len = %d",len);
	lpbuf = (char*)calloc(1, len);
	if(lpbuf==NULL){
		p_debug("Malloc error.\n");
		return -1;
	}
	strcpy(lpbuf,h_post);
	strcat(lpbuf,h_host);
	strcat(lpbuf,h_range);
	strcat(lpbuf,h_header);

/*
	int fd = open("/tmp/header_yes", O_CREAT|O_WRONLY,0644);
	write(fd, lpbuf, strlen(lpbuf));
	close(fd);
*/
	//p_debug("lpbuf = %s\n,strlen=%d",lpbuf,strlen(lpbuf));
	if(http_tcpclient_send(pclient,lpbuf,len-1)<0){
		free(lpbuf);lpbuf=NULL;
		p_debug("http_tcpclient_send error.");
		return -2;
	}
	safe_free(lpbuf);
	if(type==GET_VIDEO_SIZE)
	{
		if(http_get_video_size(pclient)<=0)
		return 0;
	}
	if(http_tcpclient_download(pclient,type) <= 0){
		p_debug("http_tcpclient_download error\n");
		return -3;
	}
	else
		p_debug("download success\n");
	//p_debug("leave http_get");
	return 0;
}

int http_post(http_tcpclient *pclient,char *page, char *request, char **response)
{
	//p_debug("access http_post");
	char *lpbuf=NULL;
	char *ptmp=NULL;
	char *end_tmp=NULL;
	char *tmp=NULL;

	int	len;
	char	h_post[BUFFER_SIZE_1024]="\0";
	char 	h_host[HOST_IP_LEN_256]="\0";
	char 	h_content_len[128]="\0";
	char 	h_content_type[256]="\0";;
	
	const char *h_header="User-Agent: ZhuiJuShenQi\r\nCache-Control: no-cache\r\nAccept: */*\r\nAccept-Language: zh-cn\r\nConnection: Keep-Alive\r\n";

	memset(h_post, 0, sizeof(h_post));
	sprintf(h_post, "POST %s HTTP/1.1\r\n", page);
	memset(h_host, 0, sizeof(h_host));
	sprintf(h_host, "HOST: %s:%d\r\n",pclient->host, pclient->remote_port);
	memset(h_content_type, 0, sizeof(h_content_type));
	sprintf(h_content_type, "Content-Type: text/html\r\n");
	memset(h_content_len, 0, sizeof(h_content_len));
	sprintf(h_content_len,"Content-Length: %d\r\n\r\n", strlen(request));
	//p_debug("h_content_len = %s,request=%s\n",h_content_len,request);

	len = strlen(h_post)+strlen(h_host)+strlen(h_header)+strlen(h_content_type)+strlen(h_content_len)+strlen(request)+1;
	//p_debug("len = %d\n",len);
	lpbuf = (char*)malloc(len);
	if(lpbuf==NULL){
		p_debug("Malloc error.\n");
		return -1;
	}

	memset(lpbuf,0,len);
	//p_debug("len = %d\n",len);


	strcpy(lpbuf,h_post);
	strcat(lpbuf,h_host);
	strcat(lpbuf,h_header);
	strcat(lpbuf,h_content_type);
	strcat(lpbuf,h_content_len);
	//strcat(lpbuf,"\r\n");
	strcat(lpbuf,request);
	//strcat(lpbuf,"\r\n");
	//p_debug("lpbuf = %s\n,strlen=%d",lpbuf,strlen(lpbuf));
	if(http_tcpclient_send(pclient,lpbuf,len-1)<0){
		safe_free(lpbuf);
		p_debug("http_tcpclient_send error.");
		return -2;
	}
	memset(lpbuf,0,len);
	if(http_tcpclient_recv(pclient,&lpbuf,0) <= 0){
		safe_free(lpbuf);
		p_debug("http_tcpclient_recv error.");
		return -3;
	}

	memset(h_post,0,sizeof(h_post));
	strncpy(h_post,lpbuf+9,3);
	if(atoi(h_post)!=200){
		safe_free(lpbuf);
		return -5;
	}
	//p_debug("before cut lpbuf = %s\n",lpbuf);
	tmp = strstr(lpbuf+4,"HTTP");
	if(tmp)
        {
		lpbuf[tmp-lpbuf] = 0;
	}
	//p_debug("after cut lpbuf = %s\n",lpbuf);
	ptmp = (char*)strchr(lpbuf,'{');
	if(ptmp == NULL){
		safe_free(lpbuf);
		return -3;
	}

	end_tmp = (char*)strrchr(lpbuf, '}') ;
	if(end_tmp == NULL)
	{
		safe_free(lpbuf);
		return -4;
	}
	len = (end_tmp + 1) - ptmp + 1;
	*response=(char*)malloc(len);
	if(*response == NULL){
		safe_free(lpbuf);
		return -1;
	}
	memset(*response,0,len);
	strncpy(*response,ptmp,len-1);
	safe_free(lpbuf);
	return 0;
}

int http_upload(http_tcpclient *pclient,char *page, char *filename, char **response)
{
	int	fd;
	char	*lpbuf=NULL;
	char 	*ptmp=NULL;
	
	char 	*fbuf=NULL;
	int	len, nSize;
	char	h_post[BUFFER_SIZE_1024]="\0";
	char 	h_host[HOST_IP_LEN_256]="\0";
	char 	h_content_len[128]="\0";
	char	h_content_type[256]="\0";
	char 	h_content_str[1024]="\0";;

	const char *h_header="User-Agent: Mozilla/4.0\r\nCache-Control: no-cache\r\nAccept: */*\r\nAccept-Language: zh-cn\r\nConnection: Keep-Alive\r\n";
	const char *h_data_boun="---------------------------7d9ab1c50098";

	//开始组包;
	memset(h_post, 0, sizeof(h_post));
	sprintf(h_post, "POST %s HTTP/1.1\r\n", page);
	memset(h_host, 0, sizeof(h_host));
	sprintf(h_host, "HOST: %s:%d\r\n",pclient->remote_ip, pclient->remote_port);
	memset(h_content_type, 0, sizeof(h_content_type));
	sprintf(h_content_type, "Content-Type: multipart/form-data; boundary=%s\r\n", h_data_boun);

	memset(h_content_str, 0, sizeof(h_content_str));
	snprintf(h_content_str, 1023, "--%s\r\nContent-Disposition: form-data; name=\"fileName\"; filename=\"%s\"\r\n\r\n", h_data_boun, filename);
	//开始读取文件内容
	if (filename == NULL || (fd = open(filename, O_RDONLY)) == -1) {
		return -6;
	}
	nSize = lseek(fd, 0, SEEK_END);
	if (nSize == -1) return -2;
	lseek(fd, (off_t)0, SEEK_SET);
	fbuf = (char *)calloc(nSize + 1, sizeof(char));
	if (NULL == fbuf){
		return -7;
	}
	if (read(fd, fbuf, nSize) == -1) return -8;
	fbuf[nSize] = 0;

	memset(h_content_len, 0, sizeof(h_content_len));
	sprintf(h_content_len,"Content-Length: %d\r\n\r\n", strlen(h_content_str)+nSize+strlen(h_data_boun)+6);
	len = strlen(h_post)+strlen(h_host)+strlen(h_header)+strlen(h_content_len)+strlen(h_content_type)+nSize+strlen(h_data_boun)+6+1;
	lpbuf = (char*)malloc(len);
	if(lpbuf==NULL){
		p_debug("Malloc error.\n");
		return -1;
	}
	strcpy(lpbuf,h_post);
	strcat(lpbuf,h_host);
	strcat(lpbuf,h_header);
	strcat(lpbuf,h_content_type);
	strcat(lpbuf,h_content_len);
	strcat(lpbuf,"\r\n");
	strcat(lpbuf,h_content_str);
	strcat(lpbuf,fbuf);
	strcat(lpbuf,"\r\n");
	memset(h_content_str, 0, sizeof(h_content_str));
	sprintf(h_content_str, "--%s--", h_data_boun);
	strcat(lpbuf,h_content_str);
	strcat(lpbuf,"\r\n");

	//发送包
	if(http_tcpclient_send(pclient,lpbuf,len)<0){
		free(lpbuf);
		return -2;
	}
	free(lpbuf);
	
	//接收包
	if(http_tcpclient_recv(pclient,&lpbuf,0) <= 0){
		return -3;
	}

	/*响应代码,|HTTP/1.1 200 OK|
	 *从第10个字符开始,共3位
	 * */
	memset(h_post,0,sizeof(h_post));
	strncpy(h_post,lpbuf+9,3);
	if(atoi(h_post)!=200){
		if(lpbuf) free(lpbuf);
		return atoi(h_post);
	}
	ptmp = (char*)strstr(lpbuf,"\r\n\r\n");
	if(ptmp == NULL){
		free(lpbuf);
		return -4;
	}
	ptmp += 4;/*跳过\r\n*/

	len = strlen(ptmp)+1;
	*response=(char*)malloc(len);
	if(*response == NULL){
		if(lpbuf) free(lpbuf);
		return -5;
	}
	memset(*response,0,len);
	memcpy(*response,ptmp,len-1);

	/*从头域找到内容长度,如果没有找到则不处理*/
	ptmp = (char*)strstr(lpbuf,"Content-Length:");
	if(ptmp != NULL){
		char *ptmp2;
		ptmp += 15;
		ptmp2 = (char*)strstr(ptmp,"\r\n");
		if(ptmp2 != NULL){
			memset(h_post,0,sizeof(h_post));
			strncpy(h_post,ptmp,ptmp2-ptmp);
			if(atoi(h_post)<len)
				(*response)[atoi(h_post)] = '\0';
		}
	}

	if(lpbuf) free(lpbuf);

	return 0;
}

