/*
 * mcast.h
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

#ifndef __MCAST_H__
#define __MCAST_H__

#include <stdint.h>
#include <stddef.h>

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * mCast_t - structure for multicast socket
 */
typedef struct {
	int		sock_fd;

#define IPV4_ADDR_LENGTH	16
	char		mc_group[IPV4_ADDR_LENGTH];
	uint16_t	mc_port;
} mCast_t;

/**
 * mcast_create_socket -
 */
extern mCast_t *mcast_create_socket(char *mc_group, uint16_t mc_port, int loopback);

/**
 * mcast_destroy_socket -
 */
extern void mcast_destroy_socket(mCast_t *mc);

/**
 * mcast_send -
 */
extern ssize_t mcast_send(mCast_t *mc, char *data, int len);

/**
 * mcast_receive -
 */
extern ssize_t mcast_receive(mCast_t *mc, char *data, int len);

#ifdef __cplusplus
}
#endif

#endif /* __MCAST_H__ */
