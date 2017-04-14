/*
 * =============================================================================
 *
 *       Filename:  net_util.h
 *
 *    Description:  network util module
 *
 *        Version:  1.0
 *        Created:  2014/9/5 13:53:04
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ye Wenhao (), wenhaoye@126.com
 *   Organization:  
 *
 * =============================================================================
 */
 #ifndef _NETWORK_UTIL_H_
 #define _NETWORK_UTIL_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <linux/sockios.h> // define SIOCETHTOOL
#include <linux/ethtool.h> 



#ifdef __cplusplus
extern "C"{
#endif


#ifdef NAS_BIG_ENDIAN
#define Htons(v) (v)
#define Htonl(v) (v)
#define Ntohs(v) (v)
#define Ntohl(v) (v)
#else 
#define Htons(v) ( (((v) & 0xff) << 8) | (((v) & 0xff00) >> 8) )
#define Htonl(v) ( (((v) & 0xff) << 24) | (((v) & 0xff00) << 8) | \
                   (((v) & 0xff0000) >> 8) | (((v) & 0xff000000) >> 24) )
#define Ntohs(v) ( (((v) & 0xff) << 8) | (((v) & 0xff00) >> 8) )
#define Ntohl(v) ( (((v) & 0xff) << 24) | (((v) & 0xff00) << 8) | \
                   (((v) & 0xff0000) >> 8) | (((v) & 0xff000000) >> 24) )
#endif



#define MAX_MAC_ADDR_STR_LEN 18
#define MAX_IP_ADDR_STR_LEN 16
#define MAX_NET_DEVICE_NAME_LEN 8

typedef enum
{
    INF_INVALID = 0,
    INF_DOWN,
    INF_UP,
    INF_RUNNING
}NetInfStatus;

typedef struct
{
    char inf_name[MAX_NET_DEVICE_NAME_LEN];
    char inf_ip[MAX_IP_ADDR_STR_LEN];
    char inf_gateway[MAX_IP_ADDR_STR_LEN];
    char inf_netmask[MAX_IP_ADDR_STR_LEN];
    char inf_broad_addr[MAX_IP_ADDR_STR_LEN];
    NetInfStatus inf_status;
    uint8_t inf_mac[6];
    char mac_str[MAX_MAC_ADDR_STR_LEN];
    
}NetInfInfo;


int query_arp(const char *ip, char *mac, size_t size);
int get_mac_by_ip(const char *ip, char *mac, size_t size);
int get_mac_by_ip_ext(struct sockaddr_in *addr, char *mac, size_t size);


int get_local_mac_by_devname(const char *dev_name, uint8_t mac[6]);
int get_local_ip_by_devname(const char *dev_name, struct sockaddr_in *addr);
int get_peer_mac_by_socket(int sockfd, const char *dev_name, uint8_t mac[6]);
int get_peer_mac_by_ip(struct sockaddr_in *addr, const char *dev_name, uint8_t mac[6]);

void mac_to_str(uint8_t mac[6], char *buf, size_t size);
void ip_to_str(struct sockaddr_in *addr, char *buf, size_t size);

int get_inf_flag(const char *dev_name, short *flag);
void get_net_inf_info(NetInfInfo *inf_info, uint8_t nums);
int show_system_net_inf_info(void);

int set_inf_down(const char *dev_name);
int set_inf_up(const char *dev_name);
int get_netlink_status(const char *dev_name, int *status);


int get_line_from_buf(const char *buf, size_t buf_size, char *line, size_t line_size);


#ifdef __cplusplus
}
#endif


 #endif

