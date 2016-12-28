/*
 * This file is part of mdns for shairport.
 *
 * fastmdns is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.
 * 
 * fastmdns is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
 * Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with avahi; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 */

#ifndef __FASTMDNS_H__
#define __FASTMDNS_H__

#include <stdint.h>
#include <arpa/inet.h>

#include "mcast.h"

#define MDNS_GROUP	"224.0.0.251"
#define MDNS_PORT	5353

/**
 * struct fastmDNSrrNode -
 */
typedef struct {
	void	*rr;
	int	len;
	int	ttloffset;
} mDNSrr_t;

/**
 * struct fastmDNS -
 */
struct fastmDNS {
	mCast_t		*mc;

	pthread_t	pthread;
	int		delaySecond;

#define MAX_RECV_PKG	1536
	void		*recvbuf;
	/* for pthread notify pipe */
	int		msg_pipe[2];

	mDNSrr_t	*rr_list;
	void		**ttl_points;

	char		*queryMD5;

	void		*response;
	int		res_len;
};

/**
 * struct mDNSConfig -
 */
struct mDNSConfig {
	char		*serviceName;
	char		*domainName;
	char		**txt;

	int		servPort;
	int		ttl;

	/* INET IPs, use INET big-endian */
	uint32_t	IPv4;
	int		has_ipv4;
	struct in6_addr	IPv6;
	int		has_ipv6;

	/* coordinate for message compression */
	uint16_t	base_coor;
	uint16_t	sname_coor;
	uint16_t	raop_coor;
	uint16_t	tcp_coor;
	uint16_t	local_coor;
	uint16_t	dname_coor;
};

#define RR_NAMES_LIMIT		5
#define RR_NAME_INDEX(coor)	((uint16_t)(0xC << 12) | ((uint16_t)(coor) & 0xFFF))

/**
 * fastmdns_config_init -
 */
static inline void fastmdns_config_init(struct mDNSConfig *config)
{
	memset(config, 0, sizeof(struct mDNSConfig));
	/* default Time to Live 75 minutes.
	 * based on IETF RFC6762 10th section.
	 */
	config->ttl = 75 * 60;
}

/**
 * fastmdns_Init -
 */
extern struct fastmDNS *fastmdns_Init(struct mDNSConfig *config);

/**
 * fastmDNS *fastmdns_uninit -
 */
extern void fastmdns_uninit(struct fastmDNS *mdns);

/**
 * fastmdns_mcast_poll -
 */
extern int fastmdns_mcast_thread_poll(struct fastmDNS *mdns, int delaytime);

/**
 * fastmdns_mcast_poll_quit -
 */
extern void fastmdns_mcast_poll_quit(struct fastmDNS *mdns);

#endif /* __FASTMDNS_H__ */
