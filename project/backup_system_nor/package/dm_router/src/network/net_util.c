/*
 * =============================================================================
 *
 *       Filename:  net_util.c
 *
 *    Description:  network util module
 *
 *        Version:  1.0
 *        Created:  2014/9/5 11:05:25
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ye Wenhao (), wenhaoye@126.com
 *   Organization:  
 *
 * =============================================================================
 */
#include "network/net_util.h"
#include "base.h"
#include "network/route_table.h"


#define ARP_TABLE_FILE_NAME "/proc/net/arp"

/*
  /proc/net/arp example:
IP address       HW type     Flags       HW address            Mask     Device
192.168.63.1     0x1         0x2         00:50:56:c0:00:08     *        eth1
192.168.63.254   0x1         0x2         00:50:56:f8:59:5e     *        eth1
 */
static void _parse_arp(char *line, char *ip, char *mac)
{
    int start_pos = 0;
    int i = 0;

    //get ip
    while(line[i] != ' ')
        ++i;
    memcpy(ip, line+start_pos, (i-start_pos));

    //skip HW type
    while(line[i] == ' ')
        ++i;
    while(line[i] != ' ')
        ++i;

    //skip Flags
    while(line[i] == ' ')
        ++i;
    while(line[i] != ' ')
        ++i;

    // get mac
    while(line[i] == ' ')
        ++i;
    start_pos = i;

    while(line[i] != ' ')
        ++i;
    memcpy(mac, line+start_pos, (i-start_pos));

    return;
}

/*
 *Desc: query arp table for filter specific ip.
 *
 *ip: input
 *mac: output
 *size: input
 *Return: 
 */
int query_arp(const char *ip, char *mac, size_t size)
{
    if(ip == NULL || mac == NULL || size < MAX_MAC_ADDR_STR_LEN)
    {
        DMCLOG_D("%s(): invalid argument!\n", __FUNCTION__);
        return -1;
    }

    char tmp_ip[MAX_IP_ADDR_STR_LEN];
    char tmp_mac[32];
    char line[128];
    FILE *f = NULL;

    memset(mac, 0, size);
    f = fopen(ARP_TABLE_FILE_NAME, "r");
    if(f == NULL)
    {
        DMCLOG_D("%s(): fopen(%s) failed!\n", __FUNCTION__, ARP_TABLE_FILE_NAME);
        return -2;
    }

    if(fgets(line, sizeof(line), f) == NULL)
    {
        DMCLOG_D("get first line failed!\n");
        fclose(f);
        return -3;
    }

    while(1)
    {
        memset(line, 0, sizeof(line));
        if(fgets(line, sizeof(line), f) != NULL)
        {
            memset(tmp_ip, 0, sizeof(tmp_ip));
            memset(tmp_mac, 0, sizeof(tmp_mac));
            _parse_arp(line, tmp_ip, tmp_mac);
            if(strncmp(tmp_ip, ip, sizeof(tmp_ip)) == 0)
            {
				// Fix bug: do not use (size-1)
                S_STRNCPY(mac, tmp_mac, size);
                break;
            }
        }
        else    // reach end
        {
            break;
        }
    }

    fclose(f);
    return 0;
}

/*
 *Desc: get mac by specific ip.
 *
 *ip: input
 *mac: output
 *size: input
 *Return: success on 0.
 */
int get_mac_by_ip(const char *ip, char *mac, size_t size)
{
    if(query_arp(ip, mac, size) < 0)
    {
        DMCLOG_D("query_arp() failed!\n");
        return -1;
    }

    if(mac[0] == '\0')
    {
        DMCLOG_D("can not find mac to this ip(%s)\n", ip);
        return -2;
    }

    return 0;
}

/*
 * Desc: get mac by specific sockaddr
 *
 * addr: input, point to specific sockaddr_in.
 * mac: output,
 * size: input,
 * Return: success on RET_SUCCESS, 
 */
int get_mac_by_ip_ext(struct sockaddr_in *addr, char *mac, size_t size)
{
    char ip_str[MAX_IP_ADDR_STR_LEN] = {0};

    snprintf(ip_str, MAX_IP_ADDR_STR_LEN, "%s", inet_ntoa(addr->sin_addr));
    if(get_mac_by_ip(ip_str, mac, size) < 0)
    {
        DMCLOG_D("get_mac_by_ip(%s) failed!\n", ip_str);
        return ESOCK_ARP;
    }

    return RET_SUCCESS;
}

/*
 * Desc: get local mac with specific network device name.
 * 
 * dev_name: input, network device name, like "eth0"
 * mac: output, 
 * Return: success on RET_SUCCESS, else on negate value.
 */
int get_local_mac_by_devname(const char *dev_name, uint8_t mac[6])
{
    struct ifreq ifr;
    int sockfd;
    
    if(dev_name == NULL || mac == NULL)
    {
        DMCLOG_D("invalid param!\n");
        return EINVAL_ARG;
    }
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd  < 0)
    {
        DMCLOG_D("create socket failed!\n");
        return ESOCK;
    }

    S_STRNCPY(ifr.ifr_name, dev_name, sizeof(ifr.ifr_name));
    if(ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0)
    {
        DMCLOG_D("ioctl get hwaddr error!\n");
        close(sockfd);
        return ESOCK_IOCTL;
    }

    memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);

    close(sockfd);
    return RET_SUCCESS;
}

/*
 * Desc: get local ip by specific network device name.
 *
 * dev_name: input, network device name, like "eth0"
 * addr: output, 
 * Return: success on RET_SUCCESS, 
 */
int get_local_ip_by_devname(const char *dev_name, struct sockaddr_in *addr)
{
    struct ifreq ifr;
    int sockfd;
    
    if(dev_name == NULL || addr == NULL)
    {
        DMCLOG_D("invalid param!\n");
        return EINVAL_ARG;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd  < 0)
    {
        DMCLOG_D("create socket failed!\n");
        return ESOCK;
    }

    S_STRNCPY(ifr.ifr_name, dev_name, sizeof(ifr.ifr_name));
    if(ioctl(sockfd, SIOCGIFADDR, &ifr) < 0)
    {
        DMCLOG_D("ioctl get ip error!\n");
        close(sockfd);
        return EIOCTL;
    }

    memcpy(addr, &ifr.ifr_addr, sizeof(struct sockaddr_in));    
    
    close(sockfd);
    return RET_SUCCESS;
}

/*
 * Desc: get peer mac by connected socket fd.
 *
 * sockfd: input, connected socket fd.
 * dev_name: input, specific network device name, like "eth0"
 * mac: output,
 * Return: success on RET_SUCCESS,
 */
int get_peer_mac_by_socket(int sockfd, const char *dev_name, uint8_t mac[6])
{
    struct sockaddr_in addr;
    struct arpreq arpreq;
    socklen_t len;
    
    if(sockfd < 0 || dev_name == NULL || mac == NULL)
    {
        DMCLOG_D("invalid param!\n");
        return EINVAL_ARG;
    }

    len = sizeof(struct sockaddr_in);
    memset(&addr, 0, sizeof(struct sockaddr_in));
    memset(&arpreq, 0, sizeof(struct arpreq));

    if(getpeername(sockfd, (struct sockaddr *)&addr, &len) < 0)
    {
        DMCLOG_D("getpeername failed!\n");
        return ESOCK;
    }

    S_STRNCPY(arpreq.arp_dev, dev_name, sizeof(arpreq.arp_dev));
    memcpy(&arpreq.arp_pa, &addr, sizeof(addr));
    arpreq.arp_pa.sa_family = AF_INET;
    arpreq.arp_ha.sa_family = AF_UNSPEC;
    if(ioctl(sockfd, SIOCGARP, &arpreq) < 0)
    {
        DMCLOG_D("ioctl get peer mac failed!\n");
        return ESOCK_IOCTL;
    }

    memcpy(mac, (uint8_t *)arpreq.arp_ha.sa_data, 6);

    return RET_SUCCESS;
}

/*
 * Desc: get peer mac by specific ip addr.
 *
 * addr: input, peer ip address.
 * dev_name: input, specific network device name.
 * mac: output,
 * Return:success on RET_SUCCESS, 
 */
int get_peer_mac_by_ip(struct sockaddr_in *addr, const char *dev_name, uint8_t mac[6])
{
    int sockfd;
    struct arpreq arpreq;
    
    if(addr == NULL || dev_name == NULL || mac == NULL)
    {
        DMCLOG_D("invalid param!\n");
        return EINVAL_ARG;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
    {
        DMCLOG_D("create socket failed!\n");
        return ESOCK;
    }

    memcpy(&arpreq.arp_pa, addr, sizeof(struct sockaddr_in));
    S_STRNCPY(arpreq.arp_dev, dev_name, sizeof(arpreq.arp_dev));
    arpreq.arp_ha.sa_family = AF_UNSPEC;
    if(ioctl(sockfd, SIOCGARP, &arpreq) < 0)
    {
        DMCLOG_D("ioctl get peer mac failed!\n");
        close(sockfd);
        return ESOCK_IOCTL;
    }

    memcpy(mac, (uint8_t *)arpreq.arp_ha.sa_data, 6);

    close(sockfd);
    return RET_SUCCESS;
}

// translate mac address into string.
void mac_to_str(uint8_t mac[6], char *buf, size_t size)
{
#if 0
    if(mac == NULL || buf == NULL || size < 18)
    {
        DMCLOG_D("invalid param!\n");
        return;
    }
#endif
    snprintf(buf, size, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2],\
             mac[3], mac[4], mac[5]);
    buf[MAX_MAC_ADDR_STR_LEN - 1] = '\0';
}

// translate ip address into string.
void ip_to_str(struct sockaddr_in *addr, char *buf, size_t size)
{
#if 0
    if(addr == NULL || buf == NULL)
    {
        DMCLOG_D("invalid param!\n");
        return;
    }
#endif
    memset(buf, 0, size);
    snprintf(buf, size, "%s", inet_ntoa(addr->sin_addr));
}

/*
 * Desc: get the flag of interface .
 *
 * dev_name: input, the name of network interface.
 * flag: output, save the flag.
 * Return:
 */
int get_inf_flag(const char *dev_name, short *flag)
{
    struct ifreq ifr;
    int sockfd;
    
    if(dev_name == NULL || flag == NULL)
    {
        DMCLOG_D("invalid param!\n");
        return EINVAL_ARG;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd  < 0)
    {
        DMCLOG_D("create socket failed!\n");
        return ESOCK;
    }

    S_STRNCPY(ifr.ifr_name, dev_name, sizeof(ifr.ifr_name));
    if(ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
    {
        DMCLOG_D("ioctl get interface flag failed!\n");
        close(sockfd);
        return EIOCTL;
    }

    *flag = ifr.ifr_flags ;  
    
    close(sockfd);
    return RET_SUCCESS;
}

// just like "ifconfig dev_name down"
int set_inf_down(const char *dev_name)
{
    struct ifreq ifr;
    int sockfd;

    if(dev_name == NULL)
    {
        DMCLOG_D("invalid param!\n");
        return EINVAL_ARG;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd  < 0)
    {
        DMCLOG_D("create socket failed!\n");
        return ESOCK;
    }

    S_STRNCPY(ifr.ifr_name, dev_name, sizeof(ifr.ifr_name));
    if(ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
    {
        DMCLOG_D("ioctl get interface flag failed!\n");
        close(sockfd);
        return EIOCTL;
    }

    if(ifr.ifr_flags & IFF_UP)
    {
        ifr.ifr_flags &= ~IFF_UP;
        if(ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0)
        {
            DMCLOG_D("ioctl failed\n");
            close(sockfd);
            return EIOCTL;
        }
    }

    close(sockfd);
    return RET_SUCCESS;
}

// just like "ifconfig dev_name up"
int set_inf_up(const char *dev_name)
{
    struct ifreq ifr;
    int sockfd;

    if(dev_name == NULL)
    {
        DMCLOG_D("invalid param!\n");
        return EINVAL_ARG;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd  < 0)
    {
        DMCLOG_D("create socket failed!\n");
        return ESOCK;
    }

    S_STRNCPY(ifr.ifr_name, dev_name, sizeof(ifr.ifr_name));
    if(ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
    {
        DMCLOG_D("ioctl get interface flag failed!\n");
        close(sockfd);
        return EIOCTL;
    }

    if(ifr.ifr_flags & IFF_UP)
    {
        // do nothing
    }
    else
    {
        ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
        if(ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0)
        {
            DMCLOG_D("ioctl failed\n");
            close(sockfd);
            return EIOCTL;
        }
    }

    close(sockfd);
    return RET_SUCCESS;
}


int get_inf_sub_netmask(const char *dev_name, char *netmask, size_t size)
{
    struct ifreq ifr;
    int sockfd;
    
    if(dev_name == NULL || netmask == NULL)
    {
        DMCLOG_D("invalid param!\n");
        return EINVAL_ARG;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd  < 0)
    {
        DMCLOG_D("create socket failed!\n");
        return ESOCK;
    }

    S_STRNCPY(ifr.ifr_name, dev_name, sizeof(ifr.ifr_name));
    if(ioctl(sockfd, SIOCGIFNETMASK, &ifr) < 0)
    {
        DMCLOG_D("ioctl get interface netmask failed!\n");
        close(sockfd);
        return EIOCTL;
    }

    snprintf(netmask, size, "%s", (char *)inet_ntoa(((struct sockaddr_in *)(&ifr.ifr_netmask))->sin_addr ) );  
    
    close(sockfd);
    return RET_SUCCESS;
}

int get_inf_broad_addr(const char *dev_name, char *broad_addr, size_t size)
{
    struct ifreq ifr;
    int sockfd;
    
    if(dev_name == NULL || broad_addr == NULL)
    {
        DMCLOG_D("invalid param!\n");
        return EINVAL_ARG;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd  < 0)
    {
        DMCLOG_D("create socket failed!\n");
        return ESOCK;
    }

    S_STRNCPY(ifr.ifr_name, dev_name, sizeof(ifr.ifr_name));
    if(ioctl(sockfd, SIOCGIFBRDADDR, &ifr) < 0)
    {
        DMCLOG_D("ioctl get interface netmask failed!\n");
        close(sockfd);
        return EIOCTL;
    }

    snprintf(broad_addr, size, "%s", (char *)inet_ntoa(((struct sockaddr_in *)(&ifr.ifr_broadaddr))->sin_addr ) );  
    
    close(sockfd);
    return RET_SUCCESS;
}

int get_inf_all_info(NetInfInfo *inf_info)
{
    struct ifreq ifr;
    int sockfd;
    struct sockaddr_in ip_addr;
    struct sockaddr_in netmask_addr;
    //struct sockaddr_in dest_addr;
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd  < 0)
    {
        DMCLOG_D("create socket failed!\n");
        return ESOCK;
    }
    
    S_STRNCPY(ifr.ifr_name, inf_info->inf_name, sizeof(ifr.ifr_name));
    // get ip
    if(ioctl(sockfd, SIOCGIFADDR, &ifr) < 0)
    {
        DMCLOG_D("ioctl get interface ip failed!\n");
        close(sockfd);
        return EIOCTL;
    }
    snprintf(inf_info->inf_ip, MAX_IP_ADDR_STR_LEN, "%s", (char *)inet_ntoa(((struct sockaddr_in *)(&ifr.ifr_addr))->sin_addr) );
    //snprintf(inf_info->inf_gateway, MAX_IP_ADDR_STR_LEN, "0.0.0.0");
    memcpy(&ip_addr, &ifr.ifr_addr, sizeof(struct sockaddr_in));

    // get sub net mask
    if(ioctl(sockfd, SIOCGIFNETMASK, &ifr) < 0)
    {
        DMCLOG_D("ioctl get interface sub_net_mask failed!\n");
        close(sockfd);
        return EIOCTL;
    }
    snprintf(inf_info->inf_netmask, MAX_IP_ADDR_STR_LEN, "%s", (char *)inet_ntoa(((struct sockaddr_in *)(&ifr.ifr_netmask))->sin_addr) );
    memcpy(&netmask_addr, &ifr.ifr_netmask, sizeof(struct sockaddr_in));


    // get broad addr
    if(ioctl(sockfd, SIOCGIFBRDADDR, &ifr) < 0)
    {
        DMCLOG_D("ioctl get interface broad_addr failed!\n");
        close(sockfd);
        return EIOCTL;
    }
    snprintf(inf_info->inf_broad_addr, MAX_IP_ADDR_STR_LEN, "%s", (char *)inet_ntoa(((struct sockaddr_in *)(&ifr.ifr_broadaddr))->sin_addr) );

    // get mac addr
    if(ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0)
    {
        DMCLOG_D("ioctl get interface mac_addr failed!\n");
        close(sockfd);
        return EIOCTL;
    }
    memcpy(inf_info->inf_mac, ifr.ifr_hwaddr.sa_data, 6);
    mac_to_str(inf_info->inf_mac, inf_info->mac_str, MAX_MAC_ADDR_STR_LEN);

    DMCLOG_D("%s: ip(%s), netmask(%s), broad_addr(%s), mac(%s)\n", inf_info->inf_name,
        inf_info->inf_ip, inf_info->inf_netmask, inf_info->inf_broad_addr, inf_info->mac_str);

    close(sockfd);

#if 0
    // get gateway.
    rt_match_cond_t condition;
    condition.type = DST_ADDR_INT;
    memcpy(&condition.dest.sockaddr, &ip_addr, sizeof(struct sockaddr_in));
    condition.dest.sockaddr.sin_addr.s_addr &= netmask_addr.sin_addr.s_addr;
    //snprintf(condition.dest.str_addr, sizeof(condition.dest), "%s", inf_info->inf_ip);
    if(get_route_gateway(&condition, inf_info->inf_gateway, MAX_IP_ADDR_STR_LEN) != RET_SUCCESS)
    {
        log_notice("get_route_gateway(%s) failed", inf_info->inf_ip);
        S_STRNCPY(inf_info->inf_gateway, "0.0.0.0", sizeof(inf_info->inf_gateway));
    }
    else
    {
        log_notice("get_route_gateway success, getway(%s)", inf_info->inf_gateway);
    }
#else
    // get default gateway.
    rt_match_cond_t condition;
    condition.type = DST_ADDR_INT;
    memcpy(&condition.dest.sockaddr, &ip_addr, sizeof(struct sockaddr_in));
    condition.dest.sockaddr.sin_addr.s_addr = 0;
    S_STRNCPY(condition.inf_name, inf_info->inf_name, sizeof(condition.inf_name));

#if 0
    if(get_route_gateway(&condition, inf_info->inf_gateway, MAX_IP_ADDR_STR_LEN) != RET_SUCCESS)
    {
        log_notice("get_route_gateway(%s) failed", inf_info->inf_ip);
    }
    else
    {
        log_notice("get_route_gateway success, getway(%s)", inf_info->inf_gateway);
    }
#endif

    
    if(get_route_gateway_ext(&condition, inf_info->inf_gateway, MAX_IP_ADDR_STR_LEN) != RET_SUCCESS)

    {
        log_notice("get_route_gateway(%s) failed", inf_info->inf_ip);
        //S_STRNCPY(inf_info->inf_gateway, "0.0.0.0", sizeof(inf_info->inf_gateway));
        inf_info->inf_gateway[0] = '\0';
        return ERT;
    }
    else
    {
        log_notice("get_route_gateway success, getway(%s)", inf_info->inf_gateway);
    }
    
#endif
    
    return RET_SUCCESS;
}

/*
 * Desc: get network interface info by name.
 *
 * inf_info: input/output,
 * nums: input, 
 * Return:
 */
void get_net_inf_info(NetInfInfo *inf_info, uint8_t nums)
{
    //struct sockaddr_in addr;
    uint8_t i;

    log_trace("start:");
    for(i = 0; i < nums; ++inf_info, ++i)
    {
        if(inf_info->inf_status == INF_INVALID || inf_info->inf_status == INF_DOWN)
        {
        #if 0
            if(get_local_ip_by_devname(inf_info->inf_name, &addr) != RET_SUCCESS)
            {
                DMCLOG_D("get_local_ip_by_devname(%s) failed!\n", inf_info->inf_name);
                continue;
            }
            ip_to_str(&addr, inf_info->inf_ip, sizeof(inf_info->inf_ip));
            DMCLOG_D("inf(%s): ip(%s)\n", inf_info->inf_name, inf_info->inf_ip);

            if(get_local_mac_by_devname(inf_info->inf_name, inf_info->inf_mac) < 0)
            {
                DMCLOG_D("get_local_mac_by_devname(%s) failed!\n", inf_info->inf_name);
                continue;
            }
        #endif
        
            if(get_inf_all_info(inf_info) != RET_SUCCESS)
            {
                DMCLOG_D("get_inf_all_info failed");
                continue;
            }

            inf_info->inf_status = INF_UP;
        }
    }
    log_trace("end");
}

/*
 * Desc: show all network interface info on this system.
 *
 */
int show_system_net_inf_info(void)
{
    struct ifreq buf[8];
    struct ifconf ifc;
    int sockfd;
    uint8_t inf_nums;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
    {
        DMCLOG_D("create socket failed!\n");
        return ESOCK;
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = (caddr_t)buf;
    if(ioctl(sockfd, SIOCGIFCONF, (char *)&ifc) < 0)
    {
        DMCLOG_D("ioctl get interface failed!\n");
        close(sockfd);
        return ESOCK_IOCTL;
    }

    inf_nums = ifc.ifc_len / sizeof(struct ifreq);
    DMCLOG_D("interface nums(%u)\n", inf_nums);
    if(inf_nums == 0)
    {
        close(sockfd);
        return RET_SUCCESS;
    }

    // to be continue!!!
    uint8_t i;
    for(i = 0; i < inf_nums; ++i)
    {
        DMCLOG_D("interface_name(%s)\n", buf[i].ifr_name);
    }
    
    close(sockfd);
    return RET_SUCCESS;
}

/*
 * Desc: get netlink status.
 *
 * dev_name: input, interface name, like eth2.1, ra0
 * status: output, 
 * Return: success on RET_SUCCESS.
 */
int get_netlink_status(const char *dev_name, int *status)
{
    int sockfd;
    struct ifreq ifr;
    struct ethtool_value edata;

    edata.cmd = ETHTOOL_GLINK;
    edata.data = 0;
    memset(&ifr, 0, sizeof(struct ifreq));
    S_STRNCPY(ifr.ifr_name, dev_name, sizeof(ifr.ifr_name));
    ifr.ifr_data = (char *)(&edata);

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        DMCLOG_D("create socket failed\n");
        return ESOCK;
    }

    if(ioctl(sockfd, SIOCETHTOOL, &ifr) < 0)
    {
        DMCLOG_D("ioctl failed\n");
        close(sockfd);
        return EIOCTL;
    }

    close(sockfd);
    *status = edata.data;
    return RET_SUCCESS;
}

/*
 *Desc: get line from buffer.
 *
 *Return: success return the bytes which is copyed to line.
          -1: means can not find '\n' flag;
          -2: means line_size < copyed bytes
 */
int get_line_from_buf(const char *buf, size_t buf_size, char *line, size_t line_size)
{
    size_t i = 0;
    int ret = -1;
    uint8_t flag = 0;
    
    for(i = 0; i < buf_size; ++i)
    {
        if(buf[i] == '\n')
        {
            flag = 1;
            break;
        }
    }

    if(flag)
    {
        if(i < line_size)
        {
            memset(line, 0, line_size);
            memcpy(line, buf, i);
            ret = (i+1);
        }
        else
        {
            ret = -2;
        }
    }

    return ret;
}



