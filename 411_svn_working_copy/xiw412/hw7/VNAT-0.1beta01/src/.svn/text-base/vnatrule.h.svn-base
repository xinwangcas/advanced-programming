/*
 * vnatrule.h
 *
 * VNAT address translation rule data structures and routines.
 *
 * Copyright (c) 2001-2002 Gong Su, All rights reserved.
 *
 * See the file "COPYRIGHT" for information on usage and redistribution
 * of this file.
 */

#ifndef __VNATRULE_H__
#define __VNATRULE_H__

#define STRNMSIZ 16
#define IPADRSIZ 16
#define PTNUMSIZ 8

enum { INCOMING, OUTGOING };
enum { ADD, DEL };
enum { UP, DN };

extern int vnat_ifconfig(vnat_map_t *map, int act);
extern int vnat_iptables(vnat_map_t *map, int dir, int act);

#endif
