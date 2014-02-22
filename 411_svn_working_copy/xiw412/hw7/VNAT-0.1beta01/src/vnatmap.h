/*
 * vnatmap.h
 *
 * VNAT physical-virtual address mapping module data structure and routines.
 *
 * Copyright (c) 2001-2002 Gong Su, All rights reserved.
 *
 * See the file "COPYRIGHT" for information on usage and redistribution
 * of this file.
 */

#ifndef __VNATMAP_H__
#define __VNATMAP_H__

#include <linux/net.h>       /* struct socket     */
#include <linux/netdevice.h> /* struct net_device */
#include <linux/types.h>     /* pid_t, u32, s16   */

#include "vnat.h"            /* vnat_parm_t       */

/*
 * Note the funny ordering of fields?
 */
struct vnat_map
{
    struct vnat_map *next;

    struct socket *sock;    /* socket intercepted */

    struct net_device *vif; /* VIF bound to       */

    struct task_struct *owner; /* process that owns the socket */

    /*
     * map entry state:
     * client/server, listening/connected, normal/suspended
     */
#define VNAT_MAP_CONNECTED 0x01
#define VNAT_MAP_SUSPENDED 0x02
#define VNAT_MAP_GRAFTED   0x04
    int    state;

    /*
     * Configuration settings passed to us from command line at module
     * loading time or from /proc entry at run time.
     */
    vnat_parm_t parm;

    u32    suspend_time;   /* When were we suspended?                */
    u32    keepalive_time; /* How long is the TCP keepalive time?    */
#define VNAT_MAP_KEEPOPEN  0xFF
    u8     keepopen;       /* Is TCP keepalive turned on?            */
    u8     active_timer;   /* Does the process have an active timer? */

    s16 l_user_port; /* local user supplied port number  */
    u32 l_user_addr; /* local user supplied IP address   */

    u32 l_kern_addr; /* local kernel seen IP address     */
    s16 l_kern_port; /* local kernel seen port number    */

    s16 l_vnat_port; /* local VNAT-ed port number        */
    u32 l_vnat_addr; /* local VNAT-ed IP address         */

    /*
     * Mapping from kernel to user address and port is a local matter
     * that we shouldn't be caring about.
     *
     * Take that back. These are for fixing up applications like FTP,
     * which insist on having their own idea of what a "connection" is.
     */
    u32 r_user_addr; /* remote user supplied IP address  */
    s16 r_user_port; /* remote user supplied port number */

    s16 r_kern_port; /* remote kernel seen port number   */
    u32 r_kern_addr; /* remote kernel seen IP address    */

    u32 r_vnat_addr; /* remote VNAT-ed IP address        */
    s16 r_vnat_port; /* remote VNAT-ed port number       */
};
typedef struct vnat_map vnat_map_t;

extern int         vnat_map_init(void);
extern void        vnat_map_free(void);
extern void        vnat_map_suspend(struct task_struct *task);
extern void        vnat_map_resume(struct task_struct *task);
extern int         vnat_map_dump(char *buffer, int length);
extern void        vnat_map_spin_lock(void);
extern void        vnat_map_spin_unlock(void);
extern vnat_map_t *vnat_map_kmem_cache_alloc(void);
extern void        vnat_map_kmem_cache_free(vnat_map_t *map);
extern vnat_map_t *vnat_map_find_all(vnat_map_t *map,
				     int (*cmp)(vnat_map_t *p1, vnat_map_t *p2));
extern vnat_map_t *vnat_map_find(struct socket *sock, vnat_map_t *map,
				 int (*cmp)(vnat_map_t *p1, vnat_map_t *p2));
extern vnat_map_t *vnat_map_insert(struct socket *sock, vnat_map_t *map,
				   int (*cmp)(vnat_map_t *p1, vnat_map_t *p2));
extern vnat_map_t *vnat_map_delete(struct socket *sock, vnat_map_t *map,
				   int (*cmp)(vnat_map_t *p1, vnat_map_t *p2));
extern int find_by_sock(vnat_map_t *p1, vnat_map_t *p2);
extern int inst_by_sock(vnat_map_t *p1, vnat_map_t *p2);
extern int dele_by_sock(vnat_map_t *p1, vnat_map_t *p2);
extern int dele_by_rkap(vnat_map_t *p1, vnat_map_t *p2);

#endif
