#include <sys/socket.h>  
#include <linux/netlink.h>  
#include <linux/rtnetlink.h>  
#include <linux/if.h> 
#include <arpa/inet.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "base.h"
#include "hidisk_errno.h"

#include "network/route_table.h"


typedef int (*filter_hook)(struct nlmsghdr *, rt_match_cond_t *, void *, size_t len);

#define NL_REC_BUF_LEN 4096


static int nl_socket_init(int *p_sockfd)
{
	int sockfd;
	struct sockaddr_nl nl_skaddr;
	int ret;
	
	sockfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if(sockfd < 0)
	{
		log_warning("create netlink socket failed");
		return ESOCK;
	}

	ret = fcntl(sockfd, F_SETFL, O_NONBLOCK);  
    if(ret < 0)
	{    
        close(sockfd);  
        return EFCNTL;  
    }  
  
	memset(&nl_skaddr, 0, sizeof(nl_skaddr));
	nl_skaddr.nl_family = AF_NETLINK;
	if(bind(sockfd, (struct sockaddr *)&nl_skaddr, sizeof(nl_skaddr)) < 0)
	{
		log_warning("bind netlink socket faild");
		close(sockfd);
		return ESOCK_BIND;
	}

	*p_sockfd = sockfd;
	return RET_SUCCESS;
}


static int nl_request(int nl_socket)
{
	struct nl_req
	{
		struct nlmsghdr nl_hdr;
		struct rtgenmsg g;
	};

	struct nl_req req;
	struct sockaddr_nl snl; 
	int ret;

	memset(&snl, 0, sizeof(snl));
 	snl.nl_family = AF_NETLINK;
	
	req.nl_hdr.nlmsg_len   = NLMSG_LENGTH(sizeof(struct rtgenmsg));
	req.nl_hdr.nlmsg_type  = RTM_GETROUTE;
	req.nl_hdr.nlmsg_flags = NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST; 
	req.g.rtgen_family     = AF_INET;

	ret = sendto(nl_socket, (void *)&req, sizeof(req), 0, (struct sockaddr *)&snl, sizeof(snl));
	if(ret < 0)
	{
		log_warning("nl socket send request failed");
		return ESOCK_SEND;
	}

	return RET_SUCCESS;
}


static int nl_receive(int nl_sockfd, char *buf, size_t len, size_t *rec_len)
{
	struct iovec iov = {buf, len};  
    struct sockaddr_nl snl;  
    struct msghdr msg = {( void* )&snl, sizeof(snl), &iov, 1, NULL, 0, 0};
	int ret;

	do
	{
		ret = recvmsg(nl_sockfd, &msg, 0);
	}while(ret <= 0 && errno == EINTR);

	*rec_len = ret;

	if(ret <= 0)
	{
		log_warning("nl recvmsg failed");
		return ESOCK_RECV;
	}

	return RET_SUCCESS;
}


static int nl_parse(char *buf, size_t len, filter_hook parse_callback, 
				rt_match_cond_t *c, void *arg, size_t sz)
{
	struct nlmsghdr *p_msg_hdr;
	int status = len;
	int ret = RET_SUCCESS;

	for (p_msg_hdr = (struct nlmsghdr *)buf;
			NLMSG_OK(p_msg_hdr, status);  
                p_msg_hdr = NLMSG_NEXT(p_msg_hdr, status))
	{
        /* Finish of reading. */  
        if(p_msg_hdr->nlmsg_type == NLMSG_DONE)  
        {
			return ret;
		}

		/* Error handling. */  
        if(p_msg_hdr->nlmsg_type == NLMSG_ERROR)
		{  
            struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(p_msg_hdr);  
  
            /* If the error field is zero, then this is an ACK */  
            if(err->error == 0 )
			{  
                /* return if not a multipart message, otherwise continue */  
                if (!( p_msg_hdr->nlmsg_flags & NLM_F_MULTI))
				{  
                    return RET_SUCCESS;  
                }  
                continue;  
            }  
  
            if(p_msg_hdr->nlmsg_len < NLMSG_LENGTH (sizeof(struct nlmsgerr)))
			{  
                log_warning("error: message truncated"); 
				
                return ERT_MSG_TRUNC;  
            }  
            log_warning("error: type=%u, seq=%u, pid=%d",   
                        err->msg.nlmsg_type, err->msg.nlmsg_seq,  
                        err->msg.nlmsg_pid);  
   
            return ERT_MSG_TYPE;  
        }

		ret = parse_callback(p_msg_hdr, c, arg, sz);
		if(ret == RET_SUCCESS)//find matched route table item, return..
		{
			log_warning("parse callback success, find matched item.");
			return ret;
		}
		else if(ret == ERT_COND_NOT_MATCH)//not found , continue
		{
			continue;
		}
		else//error..
		{
            log_warning("callback failed");
			return ret;
		}
	}

	if(status) 
	{  
        log_warning("error: data remnant size %d",  status);  
        return ERT_DATA_REMNANT;  
    }  	
	
	return ret;
}


static void nl_parse_rtattr ( struct rtattr **tb, int max, struct rtattr *rta, int len )  
{  
    while(RTA_OK(rta, len))
	{  
        if ( rta->rta_type <= max )  
            tb[rta->rta_type] = rta;  
        rta = RTA_NEXT(rta,len);  
    }  
}  


static int rt_parse_get_gateway(struct nlmsghdr *p_msg_hdr, rt_match_cond_t *c, 
						        void *buf, size_t sz)
{
	struct rtmsg *rtm;  
    struct rtattr *attr[RTA_MAX + 1];
	size_t len;
	void *dest;
	void *gateway;
	char anyaddr[16]={0};
	struct in_addr inaddr;

    log_debug("enter rt parse get gateway");

	rtm = NLMSG_DATA(p_msg_hdr);
	len = p_msg_hdr->nlmsg_len - NLMSG_LENGTH(sizeof(struct rtmsg)); 
	if(len < 0)
	{
		return ERT_MSG_TRUNC;
	}

	memset (attr, 0, sizeof(attr)); 
	nl_parse_rtattr(attr, RTA_MAX, RTM_RTA(rtm), len);

	if ( p_msg_hdr->nlmsg_type != RTM_NEWROUTE )  
        return ERT_COND_NOT_MATCH;  
    if ( rtm->rtm_type != RTN_UNICAST )  
        return ERT_COND_NOT_MATCH;  

//log
    if(attr[RTA_DST] != NULL)
        dest = RTA_DATA(attr[RTA_DST]);
    else
        dest = anyaddr;

    if(attr[RTA_GATEWAY] != NULL)
        gateway = RTA_DATA(attr[RTA_GATEWAY]);
    else
        gateway = anyaddr;

    inaddr.s_addr = *(unsigned int*)dest;
    log_debug("dest:%s(0x%X)",inet_ntoa(inaddr),inaddr.s_addr);
    inaddr.s_addr = *(unsigned int*)gateway;
    log_debug("gateway:%s(0x%X)",inet_ntoa(inaddr),inaddr.s_addr);
//log

    if(c->type == DST_ADDR_STR)
    {   
        log_debug("our target dest:%s",c->dest.str_addr);
        if(inet_pton(AF_INET, c->dest.str_addr, &inaddr) != 1)
        {
            return ESOCK_ADDR_TRANSF;
        }
    }
    else
    {
        inaddr = c->dest.sockaddr.sin_addr;
        log_debug("our target dest:0x%X",inaddr.s_addr);
    }
	
	if(inaddr.s_addr == *(in_addr_t *)dest)//dest match
	{
		log_debug("find matched route item!");
		if(attr[RTA_GATEWAY] != NULL)
		{
			gateway = RTA_DATA(attr[RTA_GATEWAY]);
		}
		else
		{
			gateway = anyaddr;
		}
		
		inaddr.s_addr = *(unsigned int*)gateway;
		S_STRNCPY((char *)buf, inet_ntoa(inaddr), sz);
			
		log_debug("gateway:%s\n", (char *)buf);
		return RET_SUCCESS;
	}
    else
    {
        log_debug("not match");
    }
	
	return ERT_COND_NOT_MATCH;
}


static int get_route_table(filter_hook filter, rt_match_cond_t *c, void *buf, size_t sz)
{
	int sockfd;
	int ret;
	char rev_buf[NL_REC_BUF_LEN];
	size_t len;

	if((ret = nl_socket_init(&sockfd)) != RET_SUCCESS)
	{
		log_warning("nl socket init error, ret(0x%x)", ret);
		return ret;
	}

	if((ret = nl_request(sockfd)) < 0)
	{
		log_warning("nl request error, ret(0x%x)", ret);
		close(sockfd);
		return ret;
	}

	if((ret = nl_receive(sockfd, rev_buf, NL_REC_BUF_LEN, &len)) != RET_SUCCESS)
	{
		log_warning("nl receive error, ret(0x%x)", ret);
		close(sockfd);
		return ret;
	}

	if((ret = nl_parse(rev_buf, len, filter, c, buf, sz)) != RET_SUCCESS)
	{
		log_warning("nl parse error, ret(0x%x)", ret);
		close(sockfd);
		return ret;
	}
	
	close(sockfd);
	return ret;
}


int get_route_gateway(rt_match_cond_t *c, void *buf, size_t sz)
{
	return get_route_table(rt_parse_get_gateway, c, buf, sz);
}

#if 0
int main(int argc, char **argv)
{
	int ret;
	rt_match_cond_t c;
	char buf[20];

	c.type = DST_ADDR_STR;
	strcpy(c.dest.str_addr, "192.168.4.8");

	ret = get_route_gateway(&c, buf, sizeof(buf));

	printf("ret = %d",ret);

	return 0;
}
#endif

#define ROUTE_TABLE_FILE "/proc/net/route"

#define SKIP_SPACE(p) do{\
    ++p;\
    if(*p == ' ')\
    {\
        break;\
    }\
    }while(1)

#define SKIP_NOT_SPACE(p) do{\
    ++p;\
    if(*p != ' ')\
    {\
        break;\
    }\
    }while(1)

#define SKIP_CHAR(p, c) do{\
    ++p;\
    if(*p == c)\
    {\
        break;\
    }\
    }while(1)
    
/*
Iface   Destination     Gateway         Flags   RefCnt  Use     Metric  Mask            MTU     Window  IRTT                                                       
eth0    0041A8C0        00000000        0001    0       0       0       00FFFFFF        0       0       0                                                                               
eth1    0070A8C0        00000000        0001    0       0       0       00FFFFFF        0       0       0                                                                               
eth1    0000FEA9        00000000        0001    0       0       1000    0000FFFF        0       0       0                                                                            
eth0    00000000        0241A8C0        0003    0       0       100     00000000        0       0       0                                                                             
eth1    00000000        0170A8C0        0003    0       0       100     00000000        0       0       0 
*/
static int _parse_gateway_from_buf(char *line, char *gateway)
{
    char *start;
    char *end;
    
    SKIP_CHAR(line, '\t');
    SKIP_CHAR(line, '\t');
    start = line+1;
    
    SKIP_CHAR(line, '\t');
    end = line;

    memcpy(gateway, start, (end-start));
    return RET_SUCCESS;
}

int get_route_gateway_ext(rt_match_cond_t *c, void *buf, size_t sz)
{
    FILE *f = fopen(ROUTE_TABLE_FILE, "r");
    if(f == NULL)
    {
        log_warning("fopen(%s) failed", ROUTE_TABLE_FILE);
        return EOPEN;
    }

    char line[256];
    char gateway[16];
    struct in_addr sin_addr;
    while(1)
    {
        memset(line, 0, sizeof(line));
        if(fgets(line, sizeof(line), f) == NULL)
        {
            log_trace("EOF");
            break;
        }

        if(strncmp(line, c->inf_name, strlen(c->inf_name)) )
        {
            continue;
        }

        memset(gateway, 0, sizeof(gateway));
        _parse_gateway_from_buf(line, gateway);
        log_debug("gateway: %s", gateway);
        if(strcmp(gateway, "00000000") == 0)
        {
            continue;
        }

        sin_addr.s_addr = strtoul(gateway, NULL, 16);
        S_STRNCPY(buf, inet_ntoa(sin_addr), sz);
        break;
    }

    fclose(f);
    return RET_SUCCESS;
}

