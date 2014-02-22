/*
 * vnatmap.c
 *
 * VNAT physical-virtual address mapping table handling module.
 *
 * Copyright (c) 2001-2002 Gong Su, All rights reserved.
 *
 * See the file "COPYRIGHT" for information on usage and redistribution
 * of this file.
 */

#if defined(MODVERSIONS)
#include <linux/modversions.h>
#endif

#define __KERNEL_SYSCALLS__

#include <net/ip.h>      /* needed by route.h */
#include <net/route.h>   /* struct rt_key     */
#include <linux/net.h>   /* needed by in.h    */
#include <linux/in.h>    /* INADDR_LOOPBACK   */
#include <linux/slab.h>  /* kmem_cache_...    */
#include <linux/time.h>  /* do_gettimeofday   */

#include "vnat.h"
#include "vnatd.h"
#include "vnatmap.h"
#include "vnatrule.h"
#include "vnatvcmp.h"
#include "misc.h"

#define VNAT_MAP_BASE_NR 16

/*
 * Hash key is computed by concatenating pid with port then modulo hash
 * table size.
 */
static vnat_map_t *vnat_map_base[VNAT_MAP_BASE_NR];
static spinlock_t  vnat_map_lock[VNAT_MAP_BASE_NR];         /* entry lock */
static spinlock_t  vnat_map_base_lock = SPIN_LOCK_UNLOCKED; /* table lock */

static kmem_cache_t *vnat_map_cachep;

/*
 * Initialize address mapping hash table pointers.
 */
int
vnat_map_init(void)
{
    int i;

    PRINTK4(debug, ("[%s:%d|vnat_map_init]: initializing mapping table { ", \
		    current->comm, current->pid));

    if ((vnat_map_cachep = kmem_cache_create("vnat_map",
					     sizeof(vnat_map_t), 0,
					     SLAB_HWCACHE_ALIGN, NULL, NULL))
	== NULL) return -ENOMEM;

    for (i = 0; i < VNAT_MAP_BASE_NR; i++)
    {
	PRINTK4(debug, ("."));
	vnat_map_base[i] = NULL;
	vnat_map_lock[i] = SPIN_LOCK_UNLOCKED;
    }

    PRINTK4(debug, (" }\n"));

    return 0;
}

/*
 * Free a list of map entries.
 */
void
vnat_map_free(void)
{
    int i;

    /*
     * We really have nothing to do there. This function is called from
     * kvnat_exit when the kvnat module is about to be unloaded. But
     * kvnat_exit will only be called when the module's usage count has
     * dropped to 0, which means that all virtualized connections are
     * properly closed and there should be no mapping entry in the table.
     *
     * But now we've changed the design. A map entry is created when the
     * socket is created, rather than when the socket is connected. We
     * have two choices:
     *
     * 1. increment the module usage count whenever a map entry is created.
     *    This will prevent us from being unloaded until all virtualized
     *    sockets (connected or not) are closed.
     * 2. increment the module usage count whenever a virtualized socket
     *    socket is connected. This is how it's done currently. But with
     *    the design change, it means that we could potentially be unloaded
     *    while still having virtualized sockets (those that are not
     *    connected). So if we want to stick to this choice, we need to
     *    clean up the map entries for these sockets.
     */
    PRINTK4(debug, ("[%s:%d|vnat_map_free]: freeing mapping table { ", \
		    current->comm, current->pid));

    for (i = 0; i < VNAT_MAP_BASE_NR; i++)
    {
	if (vnat_map_base[i])
	{
	    vnat_map_t **chain = &vnat_map_base[i], *ptr;

	    /*
	     * We can't use list_free since list_free uses "kfree", not
	     * "kmem_cache_free".
	     */
	    PRINTK4(debug, ("+"));
	    while (*chain)
	    {
		ptr = *chain;
		*chain = (*chain)->next;

		ptr->sock->flags &= ~O_VNAT;
		kmem_cache_free(vnat_map_cachep, ptr);
	    }
	}
	else
	{
	    PRINTK4(debug, ("."));
	}
    }

    /*
     * We are called when the module is about to be unloaded. That happens
     * only when the usage counter of the module has reached 0, which means
     * that our cache _must_ be clean.
     */
    kmem_cache_destroy(vnat_map_cachep);

    PRINTK4(debug, (" }\n"));
}

void
vnat_map_suspend(struct task_struct *task)
{
    int i;

#ifdef PROFILE
    struct timeval tv1, tv2;
#endif

    /*
     * For now, we only handle suspending all virtualized connections.
     */
    PRINTK4(debug, ("[%s:%d|vnat_map_suspend]: \
suspending sockets for %s:%d { ", current->comm, current->pid, \
		    task == NULL ? "all" : task->comm, \
		    task == NULL ? -1 : task->pid));

#ifdef PROFILE
    do_gettimeofday(&tv1);
#endif

    spin_lock(&vnat_map_base_lock);
    for (i = 0; i < VNAT_MAP_BASE_NR; i++)
    {
	struct vnat_map *ptr = vnat_map_base[i];

	while (ptr)
	{
	    /*
	     * Only suspend sockets in connected state!.
	     */
	    if (ptr->state & VNAT_MAP_CONNECTED)
	    {
		vcmp_msg_t msg;
		struct socket *sock = NULL;

		if (vcmp_socket(ptr->r_vnat_addr,
				htons(KVNATD_PORT), &sock, timeo) == 0)
		{
		    msg.map  = ptr;
		    vcmp_sendmsg(sock, &msg, VCMP_SUSP, MSG_NOSIGNAL, timeo);
		    sock_release(sock);
		}
		/*
		 * Since the VCMP_SUSP message is entirely optional,
		 * we don't care if the VCMP_SUSP message is sent
		 * successfully or not; and we don't wait for a reply.
		 */
		ptr->state |= VNAT_MAP_SUSPENDED;
		/*
		 * Clean up the netfilter rules.
		 */
		vnat_iptables(ptr, INCOMING, DEL);
		vnat_iptables(ptr, OUTGOING, DEL);
		/*
		 * Bring down the virtual interface.
		 */
		vnat_ifconfig(ptr, DN);
	    }

	    ptr = ptr->next;
	}
    }
    spin_unlock(&vnat_map_base_lock);

#ifdef PROFILE
    do_gettimeofday(&tv2);

    PRINTK4(debug, ("[%s:%d|vnat_map_suspend]: \
elapsed time %ld microseconds\n", current->comm, current->pid, \
		    (tv2.tv_sec-tv1.tv_sec)*1000000+(tv2.tv_usec-tv1.tv_usec)));
#endif

    PRINTK4(debug, (" }\n"));
}

void
vnat_map_resume(struct task_struct *task)
{
    int i, j;

#ifdef PROFILE
    struct timeval tv1, tv2;
#endif

    /*
     * Update all virtualized connections marked as suspended.
     */
    PRINTK4(debug, ("[%s:%d|vnat_map_resume]: \
resuming sockets for %s:%d { ...\n", current->comm, current->pid, \
		    task == NULL ? "all" : task->comm, \
		    task == NULL ? -1 : task->pid));

#ifdef PROFILE
    do_gettimeofday(&tv1);
#endif

    spin_lock(&vnat_map_base_lock);
    for (i = 0; i < VNAT_MAP_BASE_NR; i++)
    {
	struct vnat_map *ptr = vnat_map_base[i];

	while (ptr)
	{
	    if ((ptr->state & VNAT_MAP_SUSPENDED) &&
		((task == NULL) || (task == ptr->owner)))
	    {
		vcmp_msg_t msg;
		struct socket *sock = NULL;

		/*
		 * Ok, we will handle some special cases first.
		 */
		if (ptr->l_vnat_addr == INADDR_LOOPBACK &&
		    ptr->r_vnat_addr == INADDR_LOOPBACK)
		{
		    PRINTK4(debug, ("resuming...socket[%p]: \
loopback<->loopback, ignored\n", ptr->sock));

		    if (ptr->owner /*&& (ptr->state & VNAT_MAP_GRAFTED)*/)
		    {
			PRINTK4(debug, ("resuming...waking up [%s:%d]\n", \
					ptr->owner->comm, ptr->owner->pid));
			wake_up_process(ptr->owner);
		    }
		}
		else if (ptr->l_vnat_addr == ptr->r_vnat_addr)
		{
		    struct rtable *rt;
		    struct rt_key key;

		    PRINTK4(debug, ("resuming...socket[%p]: \
local<->local ", ptr->sock));

		    /*
		     * If we don't have a real NIC yet, migrated
		     * socket gets to keep its original NIC address
		     * as [l|r]_vnat_addr (which will be the same
		     * as [l|r]_kern_addr). Otherwise, we update
		     * [l|r]_vnat_addr with the real NIC address.
		     *
		     * And we will simply create a virtual NIC with
		     * its original NIC address. NO rules need to be
		     * created. This is because both ends of the
		     * connection are the same so IP is smart enough
		     * not to actually send packets to the NIC driver.
		     */
		    memset(&key, 0, sizeof(key));
		    key.dst = ptr->r_vnat_addr;
		    if ((j = ip_route_output_key(&rt, &key)) < 0)
			/*
			 * Temporarily make l_vnat_addr different from
			 * l_kern_addr; otherwise vnat_ifconfig will
			 * refuse to bring up the virtual NIC.
			 */
			ptr->l_vnat_addr = htonl(INADDR_LOOPBACK);
		    else
			ptr->l_vnat_addr = ptr->r_vnat_addr = rt->rt_src;
		    /*
		     * Let's hope this will succeed.
		     */
		    vnat_ifconfig(ptr, UP);
		    if (j < 0) ptr->l_vnat_addr = ptr->r_vnat_addr;

		    if (ptr->owner /*&& (ptr->state & VNAT_MAP_GRAFTED)*/)
		    {
			PRINTK4(debug, ("resuming...waking up [%s:%d]\n", \
					ptr->owner->comm, ptr->owner->pid));
			wake_up_process(ptr->owner);
		    }
		}
		else
		{
		    if (vcmp_socket(ptr->r_vnat_addr,
				    htons(KVNATD_PORT), &sock, timeo) == 0)
		    {
			msg.map = ptr;
			if (vcmp_sendmsg(sock, &msg, VCMP_UPD, MSG_NOSIGNAL,
					 timeo) > 0)
			{
			    /*
			     * Read the response.
			     *
			     * msg.sock is needed by vcmp_pullpdu to lock
			     * and read daddr and dport from transport
			     * protocol.
			     */
			    msg.sock = sock;
			    if (vcmp_recvmsg(sock, &msg, VCMP_UPD_R, 0,
					     timeo) > 0)
			    {
				/*
				 * This message tells the peer we are now
				 * fully ready so it can wake up the stopped
				 * process.
				 */
				if (vcmp_sendmsg(sock, &msg, VCMP_UPD_R_ACK,
						 MSG_NOSIGNAL, timeo) > 0)
				    ptr->state &= ~VNAT_MAP_SUSPENDED;
			    }
			}

			sock_release(sock);
		    }
		}
	    }

	    ptr = ptr->next;
	}
    }
    spin_unlock(&vnat_map_base_lock);

#ifdef PROFILE
    do_gettimeofday(&tv2);

    PRINTK4(debug, ("[%s:%d|vnat_map_resume]: \
elapsed time %ld microseconds\n", current->comm, current->pid, \
		    (tv2.tv_sec-tv1.tv_sec)*1000000+(tv2.tv_usec-tv1.tv_usec)));
#endif

    PRINTK4(debug, ("... }\n"));
}

#define VNAT_MAP_DUMP_FT "%08x %08x %-16s %-5d %02x %03d.%03d.%03d.%03d:%-5d %03d.%03d.%03d.%03d:%-5d %03d.%03d.%03d.%03d:%-5d %03d.%03d.%03d.%03d:%-5d %03d.%03d.%03d.%03d:%-5d %03d.%03d.%03d.%03d:%-5d\n"
#define VNAT_MAP_DUMP_ST " socket |  map   |vif             | pid |st|lu1.lu2.lu3.lu4:luprt|lk1.lk2.lk3.lk4:lkprt|lv1.lv2.lv3.lv4:lvprt|ru1.ru2.ru3.ru4:ruprt|rk1.rk2.rk3.rk4:rkprt|rv1.rv2.rv3.rv4:rvprt"

int
vnat_map_dump(char *buffer, int length)
{
    int   i, len = 0;

    vnat_map_t *ptr;

    if (length < sizeof(VNAT_MAP_DUMP_ST+2)) return 0;

    len += sprintf(buffer+len, "%s\n", VNAT_MAP_DUMP_ST);

    spin_lock(&vnat_map_base_lock);
    for (i = 0; i < VNAT_MAP_BASE_NR; i++)
    {
	ptr = vnat_map_base[i];
	/*
	 * Make sure we have enough space left for at least one entry.
	 */
	while(ptr && (length-len) >= (sizeof(VNAT_MAP_DUMP_ST)+2))
	{
	    len += sprintf(buffer+len, VNAT_MAP_DUMP_FT,
			   (u32)ptr->sock, (u32)ptr,
			   ptr->vif ? ptr->vif->name:"null",
			   ptr->owner ? ptr->owner->pid:-1, ptr->state,
			   NIPQUAD(ptr->l_user_addr), ntohs(ptr->l_user_port),
			   NIPQUAD(ptr->l_kern_addr), ntohs(ptr->l_kern_port),
			   NIPQUAD(ptr->l_vnat_addr), ntohs(ptr->l_vnat_port),
			   NIPQUAD(ptr->r_user_addr), ntohs(ptr->r_user_port),
			   NIPQUAD(ptr->r_kern_addr), ntohs(ptr->r_kern_port),
			   NIPQUAD(ptr->r_vnat_addr), ntohs(ptr->r_vnat_port));

	    sock_owner(ptr->sock);

	    ptr = ptr->next;
	}

	/*
	 * Buffer filled up before finishing traversing the list.
	 */
	if (ptr) break;
    }
    spin_unlock(&vnat_map_base_lock);

    return len;
}


void
vnat_map_spin_lock(void)
{
    spin_lock(&vnat_map_base_lock);
}

void
vnat_map_spin_unlock(void)
{
    spin_unlock(&vnat_map_base_lock);
}

vnat_map_t *
vnat_map_kmem_cache_alloc(void)
{
    return kmem_cache_alloc(vnat_map_cachep, GFP_KERNEL);
}

void
vnat_map_kmem_cache_free(vnat_map_t *map)
{
    kmem_cache_free(vnat_map_cachep, map);
}

vnat_map_t *
vnat_map_find_all(vnat_map_t *map, int (*cmp)(vnat_map_t *p1, vnat_map_t *p2))
{
    int i;
    vnat_map_t *ptr = NULL;

    spin_lock(&vnat_map_base_lock);
    for (i = 0; i < VNAT_MAP_BASE_NR; i++)
    {
	if ((ptr = list_find((void **)&vnat_map_base[i], (void *)map,
			     (int (*)(void *, void *))cmp))) break;
    }
    spin_unlock(&vnat_map_base_lock);

    return ptr;
}

vnat_map_t *
vnat_map_find(struct socket *sock, vnat_map_t *map,
	      int (*cmp)(vnat_map_t *p1, vnat_map_t *p2))
{
    int i = ((u32)sock) % VNAT_MAP_BASE_NR;
    vnat_map_t *hit;

    spin_lock(&vnat_map_lock[i]);
    hit = list_find((void **)&vnat_map_base[i],
		    (void *)map, (int (*)(void *, void *))cmp);
    spin_unlock(&vnat_map_lock[i]);

    return hit;
}

vnat_map_t *
vnat_map_insert(struct socket *sock, vnat_map_t *map,
		int (*cmp)(vnat_map_t *p1, vnat_map_t *p2))
{
    int i = ((u32)sock) % VNAT_MAP_BASE_NR;
    vnat_map_t *hit;

    spin_lock(&vnat_map_lock[i]);
    hit = list_insert((void **)&vnat_map_base[i],
		      (void *)map, (int (*)(void *, void *))cmp);
    spin_unlock(&vnat_map_lock[i]);

    return hit;
}

vnat_map_t *
vnat_map_delete(struct socket *sock, vnat_map_t *map,
	      int (*cmp)(vnat_map_t *p1, vnat_map_t *p2))
{
    int i = ((u32)sock) % VNAT_MAP_BASE_NR;
    vnat_map_t *hit;

    spin_lock(&vnat_map_lock[i]);
    hit = list_delete((void **)&vnat_map_base[i],
		      (void *)map, (int (*)(void *, void *))cmp);
    spin_unlock(&vnat_map_lock[i]);

    return hit;
}

int
find_by_sock(vnat_map_t *p1, vnat_map_t *p2)
{
    return (p1->sock == p2->sock);
}

int
inst_by_sock(vnat_map_t *p1, vnat_map_t *p2)
{
    return (p1->sock == p2->sock);
}

int
dele_by_sock(vnat_map_t *p1, vnat_map_t *p2)
{
    return (p1->sock == p2->sock);
}

int
dele_by_rkap(vnat_map_t *p1, vnat_map_t *p2)
{
    return (p1->r_kern_addr == p2->r_kern_addr &&
	    p1->r_kern_port == p2->r_kern_port);
}
