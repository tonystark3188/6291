/*
 * This file is part of mdns for shairport.
 *
 * fastmdns is base on libavahi-core; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include <sys/select.h>

#include <arpa/inet.h>

#include "common.h"
#include "mcast.h"
#include "fastmdns.h"

/**
 * struct mDNSHeadEntry -
 */
#pragma pack(2) /* align to half word */
struct mDNSHeadEntry {
	uint16_t	id;		/* dns.id */

	/* Flags */
	uint16_t	flags;		/* dns.flags */
	uint16_t	queries;	/* dns.count.queries */
	uint16_t	answers;	/* dns.count.answers */
	uint16_t	auth_rr;	/* dns.count.auth_rr */
	uint16_t	add_rr;		/* dns.count.add_rr */
};
#pragma pack() /* align to default */

/**
 * enum dns.resp.type
 */
enum rr_type {
	TYPE_TXT	= 0x0010,
	TYPE_PTR	= 0x000C,
	TYPE_SRV	= 0x0021,
	TYPE_AAAA	= 0x001C,
	TYPE_A		= 0x0001,
	TYPE_ANY	= 0x00FF,
	TYPE_NSEC	= 0x002F,
};

/**
 * struct mDNSrrEntry -
 */
#pragma pack(2) /* align to half word */
struct mDNSrrEntry {
	uint16_t	type;		/* dns.resp.type */
	uint16_t	fflush_class;	/* dns.resp.cache_flush and dns.resp.class */
	uint32_t	timeToLive;	/* dns.resp.ttl */
	uint16_t	length;		/* dns.resp.len */
};
#pragma pack() /* align to default */

enum {
	RR_TXT = 0,
	RR_RAOP,
	RR_SRV,
	LOCAL_AAAA,
	LOCAL_A,
	RR_DNS_SD,
	RR_COUNT,
};

/**
 * mdns_valid_rrs -
 */
static int mdns_valid_rrs(mDNSrr_t *rr_list)
{
	int count;
	int i;

	for (i = 0, count = 0; i < RR_COUNT; i++) {
		if (rr_list[i].rr)
			count++;
	}

	return count;
}

/**
 * mdns_clear_rrs_ttl -
 */
static void mdns_clear_rrs_ttl(void **ttl)
{
	int i;
	for (i = 0; i < RR_COUNT; i++) {
		if (ttl[i])
			*((uint32_t *)ttl[i]) = htonl(0);
	}
}

/**
 * mdns_memcpy -
 */
static void *mdns_memcpy(void *dest, void *src, size_t n)
{
	void *ret;
#pragma pack(2) /* align to half word */
	ret = memcpy(dest, src, n);
#pragma pack() /* align to default */
	return ret;
}

/**
 * mdns_get_textsize -
 */
static uint16_t mdns_get_textsize(char **txtArray, int add)
{
	uint16_t count = 0;
	int i;

	for (i = 0; txtArray[i] != NULL; i++)
		count += (uint16_t)strlen(txtArray[i]) + 1;

	if (add)
		count += add;

	return count;
}

/**
 * mdns_fill_texts -
 */
static void *mdns_fill_texts(char *t, char **txtArray, int endsign)
{
	int i;

	for (i = 0; txtArray[i] != NULL; i++) {
		*(uint8_t *)t = strlen(txtArray[i]);
		t++;
		mdns_memcpy(t, txtArray[i], strlen(txtArray[i]));
		t += strlen(txtArray[i]);
	}

	if (endsign) {
		*(uint8_t *)t = 0x00;
		t++;
	}

	return t;
}

/**
 * mdns_new_rr_txt -
 */
static int mdns_new_rr_txt(mDNSrr_t *rr, struct mDNSConfig *config)
{
	struct mDNSrrEntry *entry;
	int name_size, txt_size;
	char *p;

	char *host[] = {
		NULL,
		"_raop",
		"_tcp",
		"local",
		NULL,
	};

	host[0]		= config->serviceName;
	name_size	= mdns_get_textsize(host, 1);
	txt_size	= mdns_get_textsize(config->txt, 0);

	rr->len = sizeof(struct mDNSrrEntry) + name_size + txt_size;
	rr->rr = malloc(rr->len);
	if (!rr->rr) {
		debug(1, "Alloc mdns rr txt: %s\n", strerror(errno));
		rr->len = 0;
		return -1;
	}

	p = rr->rr;

	/* Fill response answers.rr_txt */
	/* resp.rr.name */
	p = mdns_fill_texts(p, host, 1);

	/* resp.rr.flags */
	entry = (struct mDNSrrEntry *)p;
	entry->type		= htons(TYPE_TXT);
	entry->fflush_class	= htons(0x8001);
	entry->timeToLive	= htonl(config->ttl);
	entry->length		= htons((uint16_t)(txt_size & 0xFFFF));
	p = (char *)(entry + 1);

	/* resp.rr.txt */
	p = mdns_fill_texts(p, config->txt, 0);

	/* Mark rr.ttl offset */
	rr->ttloffset = (uint32_t)&entry->timeToLive - (uint32_t)rr->rr;

	config->sname_coor	= config->base_coor;
	config->raop_coor	= config->sname_coor + (strlen(host[0]) + 1);
	config->tcp_coor	= config->raop_coor + (strlen(host[1]) + 1);
	config->local_coor	= config->tcp_coor + (strlen(host[2]) + 1);

	return 0;
}

/**
 * mdns_new_rr_raop -
 */
static int mdns_new_rr_raop(mDNSrr_t *rr, struct mDNSConfig *config)
{
	struct mDNSrrEntry *entry;
	char *p;

	rr->len = sizeof(struct mDNSrrEntry) + 4;
	rr->rr = malloc(rr->len);
	if (!rr->rr) {
		debug(1, "Alloc mdns rr ptr: %s\n", strerror(errno));
		rr->len = 0;
		return -1;
	}

	p = rr->rr;

	/* Fill response answers.rr_raop */
	/* resp.rr.name */
	*(uint16_t *)p = htons(RR_NAME_INDEX(config->raop_coor));
	p += sizeof(uint16_t);

	/* resp.rr.flags */
	entry = (struct mDNSrrEntry *)p;
	entry->type		= htons(TYPE_PTR);
	entry->fflush_class	= htons(0x0001);
	entry->timeToLive	= htonl(config->ttl);
	entry->length		= htons(2);
	p = (char *)(entry + 1);

	/* resp.rr.domain */
	*(uint16_t *)p = htons(RR_NAME_INDEX(config->sname_coor));

	/* Mark rr.ttl offset */
	rr->ttloffset = (uint32_t)&entry->timeToLive - (uint32_t)rr->rr;

	return 0;
}

/**
 * struct mDNSrrSrvData -
 */
#pragma pack(2) /* align to half word */
struct mDNSrrSrvData {
	uint16_t	priority;
	uint16_t	weight;
	uint16_t	port;
};
#pragma pack() /* align to default */

/**
 * mdns_new_rr_srv -
 */
static int mdns_new_rr_srv(mDNSrr_t *rr, struct mDNSConfig *config)
{
	struct mDNSrrEntry *entry;
	struct mDNSrrSrvData *srv;
	int domain_size;
	char *p;

	char *domain[] = {NULL,	NULL};

	domain[0]	= config->domainName;
	domain_size	= mdns_get_textsize(domain, 2);

	rr->len = sizeof(struct mDNSrrEntry) + sizeof(struct mDNSrrSrvData) +
		  domain_size + 2;
	rr->rr = malloc(rr->len);
	if (!rr->rr) {
		debug(1, "Alloc mdns rr srv: %s\n", strerror(errno));
		rr->len = 0;
		return -1;
	}

	p = rr->rr;

	/* Fill response answers.rr_srv */
	/* resp.rr.name */
	*(uint16_t *)p = htons(RR_NAME_INDEX(config->sname_coor));
	p += sizeof(uint16_t);

	/* resp.rr.flag */
	entry = (struct mDNSrrEntry *)p;
	entry->type		= htons(TYPE_SRV);
	entry->fflush_class	= htons(0x8001);
	entry->timeToLive	= htonl(120); /* host's ttl 2 minutes */
	entry->length		= htons(sizeof(struct mDNSrrSrvData) +
					(uint16_t)(domain_size & 0xFFFF));
	p = (char *)(entry + 1);

	/* resp.rr.data */
	srv = (struct mDNSrrSrvData *)p;
	srv->priority	= htons(0x0000);
	srv->weight	= htons(0x0000);
	srv->port	= htons(config->servPort);
	p = (char *)(srv + 1);

	p = mdns_fill_texts(p, domain, 0);
	*(uint16_t *)p = htons(RR_NAME_INDEX(config->local_coor));

	/* Mark rr.ttl offset */
	rr->ttloffset = -1;

	config->dname_coor = config->base_coor + (rr->len - domain_size);

	return 0;
}

/**
 * mdns_new_rr_dnssd -
 */
static int mdns_new_rr_dnssd(mDNSrr_t *rr, struct mDNSConfig *config)
{
	struct mDNSrrEntry *entry;
	int serv_size;
	char *p;

	char *service[] = {
		"_service",
		"_dns-sd",
		"_udp",
		NULL,
	};

	serv_size = mdns_get_textsize(service, 2);

	rr->len = sizeof(struct mDNSrrEntry) + serv_size + 2;
	rr->rr = malloc(rr->len);
	if (!rr->rr) {
		debug(1, "Alloc mdns rr dns-sd: %s\n", strerror(errno));
		rr->len = 0;
		return -1;
	}

	p = rr->rr;

	/* Fill response answers.rr_dns-sd */
	/* resp.rr.name */
	p = mdns_fill_texts(p, service, 0);
	*(uint16_t *)p = htons(RR_NAME_INDEX(config->local_coor));
	p += sizeof(uint16_t);

	/* resp.rr.flag */
	entry = (struct mDNSrrEntry *)p;
	entry->type		= htons(TYPE_PTR);
	entry->fflush_class	= htons(0x0001);
	entry->timeToLive	= htonl(config->ttl);
	entry->length		= htons(2);
	p = (char *)(entry + 1);

	/* resp.rr.data */
	*(uint16_t *)p = htons(RR_NAME_INDEX(config->raop_coor));

	/* Mark rr.ttl offset */
	rr->ttloffset = (uint32_t)&entry->timeToLive - (uint32_t)rr->rr;

	return 0;
}

/**
 * mdns_new_local_aaaa -
 */
static int mdns_new_local_aaaa(mDNSrr_t *rr, struct mDNSConfig *config)
{
	struct mDNSrrEntry *entry;
	char *p;

	rr->len = sizeof(struct mDNSrrEntry) + sizeof(struct in6_addr) + 2;
	rr->rr = malloc(rr->len);
	if (!rr->rr) {
		debug(1, "Alloc mdns local AAAA: %s\n", strerror(errno));
		rr->len = 0;
		return -1;
	}

	p = rr->rr;

	/* Fill response answers.local.aaaa */
	/* resp.rr.name */
	*(uint16_t *)p = htons(RR_NAME_INDEX(config->dname_coor));
	p += sizeof(uint16_t);

	/* resp.rr.flag */
	entry = (struct mDNSrrEntry *)p;
	entry->type		= htons(TYPE_AAAA);
	entry->fflush_class	= htons(0x8001);
	entry->timeToLive	= htonl(120); /* host's ttl 2 minutes */
	entry->length		= htons(sizeof(struct in6_addr));
	p = (char *)(entry + 1);

	/* resp.rr.ipaddr */
	mdns_memcpy(p, &config->IPv6, sizeof(struct in6_addr));

	/* Mark rr.ttl offset */
	rr->ttloffset = -1;

	return 0;
}

/**
 * mdns_new_local_a -
 */
static int mdns_new_local_a(mDNSrr_t *rr, struct mDNSConfig *config)
{
	struct mDNSrrEntry *entry;
	char *p;

	rr->len = sizeof(struct mDNSrrEntry) + 6;
	rr->rr = malloc(rr->len);
	if (!rr->rr) {
		debug(1, "Alloc mdns local A: %s\n", strerror(errno));
		rr->len = 0;
		return -1;
	}

	p = rr->rr;

	/* Fill response answers.local.a */
	/* resp.rr.name */
	*(uint16_t *)p = htons(RR_NAME_INDEX(config->dname_coor));
	p += sizeof(uint16_t);

	/* resp.rr.flag */
	entry = (struct mDNSrrEntry *)p;
	entry->type		= htons(TYPE_A);
	entry->fflush_class	= htons(0x8001);
	entry->timeToLive	= htonl(120); /* host's ttl 2 minutes */
	entry->length		= htons(4);
	p = (char *)(entry + 1);

	/* resp.rr.ipaddr */
	*(uint32_t *)p = config->IPv4;

	/* Mark rr.ttl offset */
	rr->ttloffset = -1;

	return 0;
}

/**
 * mdns_release_rr_list -
 */
static void mdns_release_rr_list(mDNSrr_t *rr_list)
{
	int i;

	if (!rr_list)
		return;

	for (i = 0; i < RR_COUNT; i++) {
		if (rr_list[i].rr) {
			free(rr_list[i].rr);
			rr_list[i].rr = NULL;
		}
	}

	free(rr_list);
}

/**
 * mdns_create_rr_list -
 */
static mDNSrr_t *mdns_create_rr_list(struct mDNSConfig *config)
{
	mDNSrr_t *rr_list;
	int err;

	rr_list = malloc(sizeof(mDNSrr_t) * RR_COUNT);
	if (!rr_list) {
		warn("Alloc rr list: %s", strerror(errno));
		return NULL;
	}

	config->base_coor = sizeof(struct mDNSHeadEntry);

	/* rr TXT */
	err = mdns_new_rr_txt(rr_list + RR_TXT, config);
	if (err < 0) {
		warn("Can NOT new mDNS rr txt.");
		goto err_out;
	}
	config->base_coor += rr_list[RR_TXT].len;

	/* rr PTR */
	err = mdns_new_rr_raop(rr_list + RR_RAOP, config);
	if (err < 0) {
		warn("Can NOT new mDNS rr ptr.");
		goto err_out;
	}
	config->base_coor += rr_list[RR_RAOP].len;

	/* rr SRV */
	err = mdns_new_rr_srv(rr_list + RR_SRV, config);
	if (err < 0) {
		warn("Can NOT new mDNS rr srv.");
		goto err_out;
	}

	/* rr DNS-SD */
	err = mdns_new_rr_dnssd(rr_list + RR_DNS_SD, config);
	if (err < 0) {
		warn("Can NOT new mDNS rr dns-sd.");
		goto err_out;
	}

	/* local AAAA */
	/* if no ipv6 address, rr.local AAAA set none. */
	if (config->has_ipv6) {
		err = mdns_new_local_aaaa(rr_list + LOCAL_AAAA, config);
		if (err < 0) {
			warn("Can NOT new mDNS rr IPv6 local.");
			goto err_out;
		}
	} else {
		rr_list[LOCAL_AAAA].rr	= NULL;
		rr_list[LOCAL_AAAA].len	= 0;
		rr_list[LOCAL_AAAA].ttloffset = -1;
	}

	/* local A */
	/* if no ipv4 address, rr.local A set none. */
	if (config->has_ipv4) {
		err = mdns_new_local_a(rr_list + LOCAL_A, config);
		if (err < 0) {
			warn("Can NOT new mDNS rr IPv4 local.");
			goto err_out;
		}
	} else {
		rr_list[LOCAL_A].rr	= NULL;
		rr_list[LOCAL_A].len	= 0;
		rr_list[LOCAL_A].ttloffset = -1;
	}

	return rr_list;

err_out:
	mdns_release_rr_list(rr_list);

	return NULL;
}

/**
 * mdns_received_check_query -
 */
static int mdns_received_check_query(void *recv, ssize_t rByte, char *md5)
{
	struct mDNSHeadEntry *entry = (struct mDNSHeadEntry *)recv;
	int result = 0;
	uint16_t rr_num;
	char *RRs;
	int names, next;
	int i;

	if (!rByte) {
		return 0;
	}

	/* Check mDNS id */
	if (ntohs(entry->id) != 0x0000) {
		/* This is not mdns package */
		return 0;
	}
	/* Check flags is query */
	if (ntohs(entry->flags) & 0x8000) {
		/* This is a response mdns package */
		return 0;
	}

	rr_num = ntohs(entry->queries);

	for (i = 0, RRs = (char *)(entry + 1); i < rr_num; i++) {
		/* RR names has compression. but need this name included
		 * before. so only need to check FQDN
		 */
		/* Check SRV name MD5 first */
		if (*(RRs + *RRs + 1) == 0x05 && !strncmp(RRs + *RRs + 2, "_raop", 5)) {
			if (*RRs > 13 && !strncmp(RRs + 1, md5, 12))
				result = 1;
			break;
		}

		/* Check raop name */
		if (*RRs == 0x05 && !strncmp(RRs + 1, "_raop", 5)) {
			/* qurey match */
			result = 1;
			break;
		}

		/* goto next */
		next = 0;
		names = 0;
		/* Calculate name area */
		while (names < RR_NAMES_LIMIT) {
			/* Domain name has message compression at the
			 * end of every names. it used 2bytes, just pass
			 * it.
			 */
			if (!RRs[next]) {
				next += 1;
				break;
			} else if ((RRs[next] & 0xC0) == 0xC0) {
				debug(1, "mDNS. %d. compressed name\n", __LINE__);
				next += 2;
				break;
			}

			next += RRs[next] + 1;
			names++;
		}

		if (names >= RR_NAMES_LIMIT) {
			result = 0;
			debug(1, "Not correct RAOP RRs\n");
			break;
		}

		RRs += next + 4;
	}

	debug(1, "query check out %d\n", result);

	return result;
}

/**
 * fastmdns_create_response -
 */
static int fastmdns_create_response(struct fastmDNS *mdns, struct mDNSConfig *config)
{
	mDNSrr_t *rr_list = mdns->rr_list;
	uint16_t rr_count = mdns_valid_rrs(rr_list);
	struct mDNSHeadEntry *entry;
	char *p;
	int i;

	mdns->res_len = sizeof(struct mDNSHeadEntry) +
			rr_list[RR_TXT].len +
			rr_list[RR_RAOP].len +
			rr_list[RR_SRV].len +
			rr_list[LOCAL_AAAA].len +
			rr_list[LOCAL_A].len +
			rr_list[RR_DNS_SD].len;

	mdns->response = malloc(mdns->res_len);
	if (!mdns->response) {
		warn("Create fast mdns response: %s", strerror(errno));
		mdns->res_len = 0;
		return -1;
	}

	mdns->ttl_points = malloc(sizeof(void **) * RR_COUNT);
	if (!mdns->ttl_points) {
		warn("Alloc fast mdns ttl_points: %s", strerror(errno));
		free(mdns->response);
		mdns->response	= NULL;
		mdns->res_len	= 0;
		return -1;
	}
	memset(mdns->ttl_points, 0, sizeof(void **) * RR_COUNT);

	/* Fill response head */
	entry = (struct mDNSHeadEntry *)mdns->response;
	entry->id	= htons(0x0000);
	entry->flags	= htons(0x8400);
	entry->queries	= htons(0);
	entry->answers	= htons(rr_count);
	entry->auth_rr	= htons(0);
	entry->add_rr	= htons(0);
	p = (char *)(entry + 1);

	/* Set query md5 point */
	mdns->queryMD5 = p + 1;

	/* Fill RRs */
	for (i = 0; i < RR_COUNT; i++) {
		if (rr_list[i].rr) {
			mdns_memcpy(p, rr_list[i].rr, rr_list[i].len);
			if (rr_list[i].ttloffset != -1)
				mdns->ttl_points[i] = p + rr_list[i].ttloffset;
			p += rr_list[i].len;
		}
	}

	return 0;
}

/**
 * fastmdns_destory_response -
 */
static void fastmdns_destory_response(struct fastmDNS *mdns)
{
	if (mdns->response)
		free(mdns->response);
	if (mdns->ttl_points)
		free(mdns->ttl_points);
}

/**
 * fastmdns_Init -
 */
struct fastmDNS *fastmdns_Init(struct mDNSConfig *config)
{
	struct fastmDNS *mdns;
	int err;

	printf("mDNS. publish %s\n", config->serviceName);

	if (!config->has_ipv4 && config->has_ipv6) {
		warn("no IPv4 or IPv6 host address");
		return NULL;
	}

	mdns = malloc(sizeof(struct fastmDNS) + MAX_RECV_PKG);
	if (!mdns) {
		warn("Alloc fast mDNS: %s", strerror(errno));
		return NULL;
	}

	/* recv buffer address */
	mdns->recvbuf = (void *)(mdns + 1);

	/* Need not loopback multicast */
	mdns->mc = mcast_create_socket(MDNS_GROUP, MDNS_PORT, 0);
	if (!mdns->mc)
		goto err_mcast_create;

	/* create tempfile rr list.
	 * When finish product rr response, we release this list.
	 */
	mdns->rr_list = mdns_create_rr_list(config);
	if (!mdns->rr_list)
		goto err_rr_list_create;

	err = fastmdns_create_response(mdns, config);
	if (err < 0)
		goto err_response_create;

	/* New a pipe for pthread quit message send */
	err = pipe(mdns->msg_pipe);
	if (err < 0) {
		warn("New pipe: %s", strerror(errno));
		goto err_pipe;
	}

	mdns_release_rr_list(mdns->rr_list);
	mdns->rr_list = NULL;

	return mdns;

err_pipe:
	fastmdns_destory_response(mdns);

err_response_create:
	mdns_release_rr_list(mdns->rr_list);
	mdns->rr_list = NULL;

err_rr_list_create:
	mcast_destroy_socket(mdns->mc);

err_mcast_create:
	free(mdns);

	return NULL;
}

/**
 * fastmDNS *fastmdns_uninit -
 */
void fastmdns_uninit(struct fastmDNS *mdns)
{
	/* Release mDNS response */
	fastmdns_destory_response(mdns);
	/* Release multicast */
	mcast_destroy_socket(mdns->mc);
	/* Close message pipe */
	close(mdns->msg_pipe[0]);
	close(mdns->msg_pipe[1]);
	/* Release fastmdns struct */
	free(mdns);
}

/**
 * fastmdns_poll_func -
 */
static void *fastmdns_poll_func(void *arg)
{
	struct fastmDNS *mdns = (struct fastmDNS *)arg;
	ssize_t rByte, sByte;
	fd_set mdns_fds;
	int n_send;
	int maxfd;
	int retval;

	/* Init select timeout time */
	struct timeval tv = {
		.tv_sec		= mdns->delaySecond,
		.tv_usec	= 0,
	};

	maxfd = mdns->mc->sock_fd > mdns->msg_pipe[0] ?
		mdns->mc->sock_fd + 1 : mdns->msg_pipe[0] + 1;

	/* Send one time response at mdns poll start  */
	sByte = mcast_send(mdns->mc, mdns->response, mdns->res_len);
	if (sByte < 0) {
		warn("mDNS send response: %s", strerror(errno));
	}

	for (;;) {
		/* Config select fds */
		FD_ZERO(&mdns_fds);
		/* Multicast Socket fd */
		FD_SET(mdns->mc->sock_fd, &mdns_fds);
		/* Message pipe for quit pthread */
		FD_SET(mdns->msg_pipe[0], &mdns_fds);

		/* Timeout wait fds select */
		retval = select(maxfd, &mdns_fds, NULL, NULL, &tv);
		if (retval < 0) {
			warn("%s. select: %s", __func__, strerror(errno));
			return NULL;
		}

		if (retval) {
			if (FD_ISSET(mdns->msg_pipe[0], &mdns_fds)) {
				char msg;
				read(mdns->msg_pipe[0], &msg, sizeof(msg));
				if (msg == 'Q') {
					debug(1, "fast mDNS normal terminate\n");
				}

				/* mDNS RFC6762. mDNS byebye package is same to response,
				 * Only TimeToLive value is ZERO.
				 */
				mdns_clear_rrs_ttl(mdns->ttl_points);
				/* Send GoodByte */
				sByte = mcast_send(mdns->mc, mdns->response, mdns->res_len);
				if (sByte < 0) {
					warn("mDNS send byebye: %s", strerror(errno));
				}
				break;
			} else if (FD_ISSET(mdns->mc->sock_fd, &mdns_fds)) {
				int ret;
				rByte = mcast_receive(mdns->mc, mdns->recvbuf, MAX_RECV_PKG);
				if (rByte < 0) {
					warn("mDNS receive message: %s", strerror(errno));
					return NULL;
				}

				/* check query. Not valid query, then continue */
				ret = mdns_received_check_query(mdns->recvbuf, rByte, mdns->queryMD5);
				if (!ret){
					/* Not wanted query, do NOT reset timeout time */
					continue;
				}
			}
		}

		if (!retval) {
			debug(1, "mDNS regular multicast\n");
			n_send = 2;
		} else {
			debug(1, "mDNS match query\n");
			n_send = 1;
		}

		/* Reset select timeout time */
		tv.tv_sec	= mdns->delaySecond;
		tv.tv_usec	= 0;

		/* if retval EQUAL 0, means select timeout.
		 * we send a response.
		 */

		while (n_send > 0) {
			sByte = mcast_send(mdns->mc, mdns->response, mdns->res_len);
			if (sByte < 0) {
				warn("mDNS send response: %s", strerror(errno));
				break;
			}

			n_send--;
			usleep(200 * 1000); /* 200 ms */
		}
	}

	return NULL;
}

/**
 * fastmdns_mcast_poll -
 */
int fastmdns_mcast_thread_poll(struct fastmDNS *mdns, int delaytime)
{
	int err = 0;

	mdns->delaySecond = delaytime;

	err = pthread_create(&mdns->pthread, NULL, fastmdns_poll_func, (void *)mdns);
	if (err) {
		warn("Create fast mDNS poll thread: %s", strerror(errno));
		return err;
	}

	return 0;
}

/**
 * fastmdns_mcast_poll_quit -
 */
void fastmdns_mcast_poll_quit(struct fastmDNS *mdns)
{
	char msg = 'Q';

	/* Send poll quit message to mdns pthread */
	write(mdns->msg_pipe[1], &msg, sizeof(msg));
	/* Resave pthread source */
	pthread_join(mdns->pthread, NULL);
}
