/*
 * vnatsock.h
 *
 * VNAT socket virtualization system call interception points.
 *
 * Copyright (c) 2001-2002 Gong Su, All rights reserved.
 *
 * See the file "COPYRIGHT" for information on usage and redistribution
 * of this file.
 */

#ifndef __VNATSOCK_H__
#define __VNATSOCK_H__

#include <linux/net.h> /* struct socket
			* #include <linux/socket.h>: struct sockaddr
			*/

extern void vnat_sock_init(void);
extern void vnat_sock_free(void);

#endif
