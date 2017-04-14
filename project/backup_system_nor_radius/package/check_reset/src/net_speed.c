#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "net_speed.h"

#if 0
static struct net_stat netstats[16]={0};
struct net_stat *get_net_stat(const char *dev)
{
    unsigned int i=0;
    if (!dev) {
        return NULL;
    }
    /* find interface stat */
    for (i = 0; i < 16; i++) {
        if (netstats[i].dev && strcmp(netstats[i].dev, dev) == 0) {
            return &netstats[i];
        }
    }
    /* wasn't found? add it */
    if (i == 16) {
        for (i = 0; i < 16; i++) {
            if (netstats[i].dev == 0) {
                netstats[i].dev = strndup(dev, MAX_BUFF_SIZE);
                return &netstats[i];
            }
        }
    }
    printf("too many interfaces used (limit is 16)");
    return NULL;
}
void clear_net_stats(void)
{
    memset(netstats, 0, sizeof(netstats));
}
int update_net_stats(const char* dev, double delta)
{
    FILE *net_dev_fp;
    // FIXME: arbitrary size chosen to keep code simple.
    int i=0;
    char buf[256]={0};
    /* open file and ignore first two lines */
    if (!(net_dev_fp = fopen("/proc/net/dev", "r"))) {
        fprintf(stderr, "fopen failed.\n");
        clear_net_stats();
        return -1;
    }
    fgets(buf, 255, net_dev_fp);    /* garbage */
    fgets(buf, 255, net_dev_fp);    /* garbage (field names) */
    /* read each interface */
    for (i = 0; i < 16; i++) {
        struct net_stat *ns=NULL;
        unsigned char *s=NULL, *p=NULL;
        unsigned long long r=0, t=0;
        unsigned long long last_recv=0, last_trans=0;
        if (fgets(buf, 255, net_dev_fp) == NULL) {
            //File EOF
            break;
        }
        //Skip Space
        p = buf;
        while (isspace((int) *p)) {
            p++;
        }
        /*s: network interface name*/
        s = p;
        //Skip Network Interface Name
        while (*p && *p != ':') {
            p++;
        }
        if (*p == '\0') {
            continue;
        }
        *p = '\0';
        /*p: reveive bytes*/
        p++;
        //Judge Network Interface or Not?
        if(strcmp(s, dev) != 0)
            continue;
        //Get struct net_stat
        ns = get_net_stat(s);
        ns->up = 1;
        last_recv = ns->recv;
        last_trans = ns->trans;
        /* bytes packets errs drop fifo frame compressed multicast|bytes ... */
        sscanf(p, "%lld  %*d     %*d  %*d  %*d  %*d   %*d        %*d       %lld",
            &r, &t);
        /* if recv or trans is less than last time, an overflow happened */
        if (r < ns->last_read_recv) {
            last_recv = 0;
        } else {
            ns->recv += (r - ns->last_read_recv);
        }
        ns->last_read_recv = r;
        if (t < ns->last_read_trans) {
            last_trans = 0;
        } else {
            ns->trans += (t - ns->last_read_trans);
        }
        ns->last_read_trans = t;
        /* calculate speeds */
        if(last_recv == 0)
            ns->recv_speed = 0;
        else
            ns->recv_speed = (ns->recv - last_recv) / delta;
        if(last_trans == 0)
            ns->trans_speed = 0;
        else
            ns->trans_speed = (ns->trans - last_trans) / delta;
        //Find Network Interface, And Work Over
        break;
    }
    fclose(net_dev_fp);
	return 0;
}

int main()
{
    int i=0;
    unsigned int sleep_time = 0;
    struct net_stat* ns = NULL;
    time_t current_time=0, last_time=0, delta_time=0;
    srand(time(NULL));
    for(i=0; i<10000; i++)
    {
    
        last_time = current_time;
        current_time = time(NULL);
        delta_time = current_time-last_time;
        //printf("Delta Time: %lld Seconds\n", delta_time);
        update_net_stats("eth0", delta_time);
        ns = get_net_stat("eth0");
        printf("Recv Speed: %f KB/s\n", ns->recv_speed/1024);
        printf("Send Speed: %f KB/s\n", ns->trans_speed/1024);
        sleep_time = 1+random()%5;
        printf("Sleep %d Second...\n", sleep_time);
        sleep(sleep_time);
    }
    return 0;
}

#endif

int get_net_stat_for_dev(struct net_stat *dev_net_stat, int delta)
{
	FILE *net_dev_fp;
    // FIXME: arbitrary size chosen to keep code simple.
    int i=0;
    char buf[256]={0};
    /* open file and ignore first two lines */
    if (!(net_dev_fp = fopen("/proc/net/dev", "r"))) {
        fprintf(stderr, "fopen failed.\n");
        //clear_net_stats();
        return -1;
    }
    fgets(buf, 255, net_dev_fp);    /* garbage */
    fgets(buf, 255, net_dev_fp);    /* garbage (field names) */
	
	for (i = 0; i < MAX_INTERFACE; i++) {
			//struct net_stat *ns=NULL;
			unsigned char *s=NULL, *p=NULL;
			unsigned long long r=0, t=0;
			unsigned long long last_recv=0, last_trans=0;
			if (fgets(buf, 255, net_dev_fp) == NULL) {
				//File EOF
				break;
			}
			//Skip Space
			p = buf;
			while (isspace((int) *p)) {
				p++;
			}
			/*s: network interface name*/
			s = p;
			//Skip Network Interface Name
			while (*p && *p != ':') {
				p++;
			}
			if (*p == '\0') {
				continue;
			}
			*p = '\0';
			/*p: reveive bytes*/
			p++;
			//Judge Network Interface or Not?
			if(strcmp(s, dev_net_stat->dev) != 0)
				continue;
			//Get struct net_stat
			//ns = get_net_stat(s);
			//ns->up = 1;
			last_recv = dev_net_stat->recv;
			last_trans = dev_net_stat->trans;
			/* bytes packets errs drop fifo frame compressed multicast|bytes ... */
			sscanf(p, "%lld  %*d	 %*d  %*d  %*d	%*d   %*d		 %*d	   %lld",
				&r, &t);
			/* if recv or trans is less than last time, an overflow happened */
			if (r < dev_net_stat->last_read_recv) {
				last_recv = 0;
			} else {
				dev_net_stat->recv += (r - dev_net_stat->last_read_recv);
			}
			dev_net_stat->last_read_recv = r;
			if (t < dev_net_stat->last_read_trans) {
				last_trans = 0;
			} else {
				dev_net_stat->trans += (t - dev_net_stat->last_read_trans);
			}
			dev_net_stat->last_read_trans = t;
			/* calculate speeds */
			if(last_recv == 0)
				dev_net_stat->recv_speed = 0;
			else
				dev_net_stat->recv_speed = (dev_net_stat->recv - last_recv) / delta;
			if(last_trans == 0)
				dev_net_stat->trans_speed = 0;
			else
				dev_net_stat->trans_speed = (dev_net_stat->trans - last_trans) / delta;
			//Find Network Interface, And Work Over
			break;
		}
		fclose(net_dev_fp);
		return 0;
}

#if 0
int main()
{
    int i=0;
	int ret = 0;
    unsigned int sleep_time = 0;
    //struct net_stat* ns = NULL;
    time_t current_time=0, last_time=0, delta_time=0;
    srand(time(NULL));
	struct net_stat eth0_net_stat;
	memset(&eth0_net_stat, 0, sizeof(struct net_stat));
	strcpy(eth0_net_stat.dev, "eth0");
	for(i=0; i<10000; i++)
    {
    
        last_time = current_time;
        current_time = time(NULL);
        delta_time = current_time-last_time;
        //printf("Delta Time: %lld Seconds\n", delta_time);
        //update_net_stats("eth0", delta_time);
        //ns = get_net_stat("eth0");
        ret = get_net_stat_for_dev(&eth0_net_stat, delta_time);
        printf("Recv Speed: %f KB/s\n", eth0_net_stat.recv_speed/1024);
        printf("Send Speed: %f KB/s\n", eth0_net_stat.trans_speed/1024);
        sleep_time = 1+random()%5;
        printf("Sleep %d Second...\n", sleep_time);
        sleep(sleep_time);
    }
    return 0;
}
#endif



