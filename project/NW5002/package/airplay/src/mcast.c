/*
 * mcast.c
 * This file is part of mdns for shairport.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.
 * 
 * This is distributed in the hope that it will be useful, but WITHOUT
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "common.h"
#include "mcast.h"

/**
 * mcast_create_socket -
 * @mc_group:
 * @mc_port:
 * @loopback:	Local loopback. 1: loop on; 0: loop off
 */
mCast_t *mcast_create_socket(char *mc_group, uint16_t mc_port, int loopback)
{
	mCast_t *mc;
	int on = 1;
	int ttl;
	int err = 0;

	mc = malloc(sizeof(mCast_t));
	if (!mc) {
		warn("%s. malloc mCast_t: %s", __func__, strerror(errno));
		return NULL;
	}

	/* Protocol family */
	mc->sock_fd = socket(PF_INET, SOCK_DGRAM, 0);
	if (mc->sock_fd < 0) {
		warn("%s. multicast socket: %s", __func__, strerror(errno));
		goto err_socket;
	}

	err = setsockopt(mc->sock_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if (err < 0) {
		warn("%s. set SO_REUSEADDR: %s", __func__, strerror(errno));
		goto err_set;
	}

	/* Bind to an address */
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(mc_port);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); /* receive multicast */

	err = bind(mc->sock_fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (err < 0) {
		warn("%s. bind server address: %s", __func__, strerror(errno));
		goto err_set;
	}

	/* Add membership to receiving socket */
	struct ip_mreq mreq;
	memset(&mreq, 0, sizeof(struct ip_mreq));
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	mreq.imr_multiaddr.s_addr = inet_addr(mc_group);

	err = setsockopt(mc->sock_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq));
	if (err < 0) {
		warn("%s. Add multicast group: %s", __func__, strerror(errno));
		goto err_set;
	}

	ttl = 255;
	err = setsockopt(mc->sock_fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
	if (err < 0) {
		warn("%s. Set ip multicast ttl: %s", strerror(errno));
		goto err_set;
	}

	/* Loopback in case someone else needs the data */
	on = loopback;
	err = setsockopt(mc->sock_fd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&on, sizeof(on));
	if (err < 0) {
		warn("%s. Enable multicast loop: %s", __func__, strerror(errno));
		goto err_set;
	}

	strcpy(mc->mc_group, mc_group);
	mc->mc_port	= mc_port;

	return mc;

err_set:
	close(mc->sock_fd);

err_socket:
	free(mc);

	return NULL;
}

/**
 * mcast_destroy_socket -
 * @mc:
 */
void mcast_destroy_socket(mCast_t *mc)
{
	if (mc) {
		close(mc->sock_fd);
		free(mc);
	}
}

/**
 * mcast_send -
 * @mc:
 * @data:
 * @len:
 */
ssize_t mcast_send(mCast_t *mc, char *data, int len)
{
	struct sockaddr_in toaddr;

	memset(&toaddr, 0, sizeof(struct sockaddr_in));

	toaddr.sin_family	= AF_INET;
	toaddr.sin_port		= htons(mc->mc_port);
	toaddr.sin_addr.s_addr	= inet_addr(mc->mc_group);

	return sendto(mc->sock_fd, data, len, 0,
			(struct sockaddr *)&toaddr, sizeof(struct sockaddr_in));
}

/**
 * mcast_receive -
 * @mc:
 * @data:
 * @len:
 */
ssize_t mcast_receive(mCast_t *mc, char *data, int len)
{
	struct sockaddr_in peer;
	socklen_t peerlen;
	return recvfrom(mc->sock_fd, data, len, 0,
			(struct sockaddr *)&peer, &peerlen);
}
