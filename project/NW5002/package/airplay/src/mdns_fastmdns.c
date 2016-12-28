/*
 * mDNS registration handler. This file is part of Shairport.
 * Copyright (c) Paul Lietar 2013
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <errno.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <net/if.h>

#include "common.h"
#include "mdns.h"

#include "fastmdns.h"

static struct fastmDNS *mdns = NULL;

static int fastmdns_register(char *apname, int port)
{
    struct mDNSConfig mdnsCfg;
    struct ifaddrs *ifa;
    struct ifaddrs *ifanode;
    char *txt[] = {MDNS_RECORD, NULL};
    char hostName[128] = {0};
    char *ifaName = NULL;
    char hwAddr[MAC_ADDR_LENGTH + 1] = {0};
    int err;
    int i = 0;
    fastmdns_config_init(&mdnsCfg);

    err = getifaddrs(&ifa);
    if (err < 0) {
        warn("Get host ip: %s", strerror);
        return -1;
    }

    for (ifanode = ifa; ifanode != NULL; ifanode = ifanode->ifa_next) {
		
        if (ifanode->ifa_flags & IFF_LOOPBACK ||
            !ifanode->ifa_addr)
            continue;printf("iiiiiiiiiiii=%d\n",i);i++;

        if (!ifaName) {
            ifaName = strdup(ifanode->ifa_name);
	    if (!ifaName) {
                warn("ifaName strdup: %s", strerror(errno));
                break;
	    }
	} else {
            if (strcmp(ifaName, ifanode->ifa_name))
                continue;
        }

        if (ifanode->ifa_addr->sa_family == AF_INET) {
            debug(1, "Get ipv4: %s\n", inet_ntoa(((struct sockaddr_in *)ifanode->ifa_addr)->sin_addr));
            mdnsCfg.IPv4 = ((struct sockaddr_in *)ifanode->ifa_addr)->sin_addr.s_addr;
            mdnsCfg.has_ipv4 = 1;
        }

        if (ifanode->ifa_addr->sa_family == AF_INET6) {
            char ipv6[INET6_ADDRSTRLEN] = {0};
            inet_ntop(AF_INET6, &((struct sockaddr_in6 *)ifanode->ifa_addr)->sin6_addr,
                      ipv6, INET6_ADDRSTRLEN);
            debug(1, "Get ipv6: %s\n", ipv6);
            memcpy(&mdnsCfg.IPv6,
                   &((struct sockaddr_in6 *)ifanode->ifa_addr)->sin6_addr,
                   sizeof(struct in6_addr));
            mdnsCfg.has_ipv6 = 1;
        }

	if (!mdnsCfg.has_ipv4 && !mdnsCfg.has_ipv6) {
            free(ifaName);
            ifaName = NULL;
        }
    }

    /* Release if addrs */
    freeifaddrs(ifa);

    if (!ifaName) {
        warn("fastmdns: no non-loopback ipv4 or ipv6 interface found");
        return -1;
    }

    /* Get network HWaddress */
    err = get_hwaddr(ifaName, hwAddr);
    free(ifaName);
    if (err < 0)
        return -1;

    /* Host name + '-' + MAC address */
    gethostname(hostName, sizeof(hostName) - (sizeof(hwAddr) - 4) - 1);
    strcat(hostName, "-");
    strcat(hostName, hwAddr + 4);

    debug(1, "Host name: %s\n", hostName);

    mdnsCfg.serviceName	= apname;
    mdnsCfg.servPort	= port;
    mdnsCfg.domainName	= hostName;
    mdnsCfg.txt		= txt;
    mdnsCfg.ttl		= 30 * 60; /* 30 Minutes */

    mdns = fastmdns_Init(&mdnsCfg);
    if (!mdns)
        return -1;

    err = fastmdns_mcast_thread_poll(mdns, 5);
    if (err < 0) {
        fastmdns_uninit(mdns);
        mdns = NULL;
        return -1;
    }

    return 0;
}

static void fastmdns_unregister(void)
{
    if (mdns) {
        /* Quit fast mdns thread */
        fastmdns_mcast_poll_quit(mdns);
        /* fast mdns uninit */
        fastmdns_uninit(mdns);

        mdns = NULL;
    }
}

mdns_backend mdns_fastmdns = {
    .name		= "fastmdns",
    .mdns_register	= fastmdns_register,
    .mdns_unregister	= fastmdns_unregister,
};
