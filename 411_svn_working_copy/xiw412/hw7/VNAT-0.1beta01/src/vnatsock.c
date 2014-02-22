/*
 * vnatsock.c
 *
 * Kernel module which intercepts socket, bind, and connect calls.
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

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kmod.h>

#include <linux/netdevice.h> /* struct net_device                      */
#include <linux/net.h>       /* struct socket, struct net_proto_family */
#include <linux/inet.h>      /* in_aton                                */
#include <linux/sched.h>     /* "current" macro                        */
#include <linux/smp_lock.h>  /* spinlock_t                             */
#include <linux/unistd.h>    /* waitpid                                */

#include <net/sock.h>        /* struct sock  */
#include <net/tcp.h>         /* tcp_put_port */

#include "vnat.h"
#include "vnatd.h"
#include "vnatmap.h"
#include "vnatproc.h"
#include "vnatrule.h"
#include "vnatsock.h"
#include "vnatvcmp.h"
#include "vnatvif.h"
#include "misc.h"
#include "umodecall.h"

/*
 * Function pointer for the old and new "inet_create".
 */
static int (*save_inet_create)(struct socket *sock, int protocol);
static int vnat_inet_create(struct socket *sock, int protocol);
/*
 * Function pointer for the old and new "inet_release".
 */
static int (*save_inet_release)(struct socket *sock);
static int vnat_inet_release(struct socket *sock);
/*
 * Function pointer for saving the old "inet_bind".
 */
#ifdef DEBUG
static int (*save_inet_bind)(struct socket *sock,
                             struct sockaddr *uaddr, int addr_len);
static int vnat_inet_bind(struct socket *sock,
			  struct sockaddr *uaddr, int addr_len);
#endif
/*
 * Function pointer for the old and new "inet_accept".
 */
static int (*save_inet_accept)(struct socket *sock,
			       struct socket *newsock, int flags);
static int vnat_inet_accept(struct socket *sock,
			    struct socket *newsock, int flags);
/*
 * Function pointer for the old and new "inet_stream_connect".
 */
static int (*save_inet_stream_connect)(struct socket *sock,
				       struct sockaddr *uaddr, int addr_len,
				       int flags);
static int vnat_inet_stream_connect(struct socket *sock,
				    struct sockaddr *uaddr, int addr_len,
				    int flags);
/*
 * Function pointer for the old and new "inet_dgram_connect".
 */
static int (*save_inet_dgram_connect)(struct socket *sock,
				      struct sockaddr *uaddr, int addr_len,
				      int flags);
static int vnat_inet_dgram_connect(struct socket *sock,
				   struct sockaddr *uaddr, int addr_len,
				   int flags);
/*
 * Function pointer for the old and new "inet_getname".
 */
static int (*save_inet_getname)(struct socket *sock, struct sockaddr *uaddr,
				int *uaddr_len, int peer);
static int vnat_inet_getname(struct socket *sock, struct sockaddr *uaddr,
			     int *uaddr_len, int peer);

/*
 * Interception can be done either at the socket layer (sys_socket,
 * sys_bind, sys_connect) or at the transport layer (inet_create,
 * inet_bind, inet_stream_connect/inet_dgram_connect).
 *
 * If we do it at socket layer, we have to deal with userspace stuff,
 * e.g., socket descriptors, which requires routines that are not
 * exported. So we do it at transport layer.
 */

void
vnat_sock_init(void)
{
    extern struct net_proto_family inet_family_ops;
    extern struct proto_ops inet_stream_ops, inet_dgram_ops;

    PRINTK4(debug, ("[%s:%d|vnat_sock_init]: {", current->comm, current->pid));

    /*
     * Tap into "inet_getname".
     */
    save_inet_getname       = inet_stream_ops.getname;
    inet_stream_ops.getname = vnat_inet_getname;
    inet_dgram_ops.getname  = vnat_inet_getname;
    PRINTK4(debug, (" inet_getname"));

    /*
     * Tap into "inet_stream_connect".
     */
    save_inet_stream_connect = inet_stream_ops.connect;
    inet_stream_ops.connect  = vnat_inet_stream_connect;
    PRINTK4(debug, (" inet_stream_connect"));

    /*
     * Tap into "inet_dgram_connect".
     */
    save_inet_dgram_connect = inet_dgram_ops.connect;
    inet_dgram_ops.connect  = vnat_inet_dgram_connect;
    PRINTK4(debug, (" inet_dgram_connect"));

    /*
     * Tap into "inet_accept".
     */
    save_inet_accept       = inet_stream_ops.accept;
    inet_stream_ops.accept = vnat_inet_accept;
    PRINTK4(debug, (" inet_accept"));

    /*
     * Tap into "inet_bind".
     */
#ifdef DEBUG
    save_inet_bind       = inet_stream_ops.bind;
    inet_stream_ops.bind = vnat_inet_bind;
    inet_dgram_ops.bind  = vnat_inet_bind;
#endif

    /*
     * Tap into "inet_release".
     */
    save_inet_release       = inet_stream_ops.release;
    inet_stream_ops.release = vnat_inet_release;
    inet_dgram_ops.release  = vnat_inet_release;
    PRINTK4(debug, (" inet_release"));

    /*
     * Tap into "inet_create".
     *
     * Note that until this is done, all the other taps will simply
     * pass the call onto the original routines. This makes sure that
     * we are safe if someone calls these routines while we are half
     * way through the tapping process.
     */
    save_inet_create       = inet_family_ops.create;
    inet_family_ops.create = vnat_inet_create;
    PRINTK4(debug, (" inet_create"));

    PRINTK4(debug, (" }\n"));
}

void
vnat_sock_free(void)
{
    extern struct net_proto_family inet_family_ops;
    extern struct proto_ops inet_stream_ops, inet_dgram_ops;

    PRINTK4(debug, ("[%s:%d|vnat_sock_free]: {", current->comm, current->pid));

    /*
     * Restore "inet_create".
     */
    inet_family_ops.create = save_inet_create;
    PRINTK4(debug, (" inet_create"));

    /*
     * Restore "inet_release".
     */
    inet_stream_ops.release = save_inet_release;
    inet_dgram_ops.release  = save_inet_release;
    PRINTK4(debug, (" inet_release"));

    /*
     * Restore "inet_bind".
     */
#ifdef DEBUG
    inet_stream_ops.bind = save_inet_bind;
    inet_dgram_ops.bind  = save_inet_bind;
#endif

    /*
     * Restore "inet_accept".
     */
    inet_stream_ops.accept = save_inet_accept;
    PRINTK4(debug, (" inet_accept"));

    /*
     * Restore "inet_stream_connect" and "inet_dgram_connect".
     */
    inet_dgram_ops.connect  = save_inet_dgram_connect;
    PRINTK4(debug, (" inet_dgram_connect"));
    inet_stream_ops.connect = save_inet_stream_connect;
    PRINTK4(debug, (" inet_stream_connect"));

    /*
     * Restore "inet_getname".
     */
    inet_stream_ops.getname = save_inet_getname;
    inet_dgram_ops.getname  = save_inet_getname;
    PRINTK4(debug, (" inet_getname"));

    PRINTK4(debug, (" }\n"));
}

/*
 * Our own "inet_create".
 */
int
vnat_inet_create(struct socket *sock, int protocol)
{
    int   prot = protocol, err;

    vnat_map_t *map;
    vnat_parm_t parm;

    /*
     * We need to increment this module's use count. This is because,
     * once the real system call routine is called, the process may
     * decide to sleep in the kernel. Now if the module exit routine
     * happens (i.e. the module is unloaded), the sleeping process
     * will have a call stack back to this routine. The routine text
     * has been unloaded and can potentially filled with random bits.
     * This is a bad thing for stability!
     */
    MOD_INC_USE_COUNT;

    /*
     * Interception logic:
     *
     * If protocol == IPPROTO_VNAT, always intercept.
     * If protocol == -IPPROTO_VNAT, always bypass.
     * If protocol == other, check the list of commands (either inclusive
     *                or exclusive) to be intercepted, and the protocol
     *                mask.
     *
     * All interception does for "inet_create" is to mark the socket
     * with a flag so other calls will function appropriately.
     */
    if (protocol == 0 || protocol == IPPROTO_VNAT || protocol == -IPPROTO_VNAT)
    {
	prot =
	    (sock->type == SOCK_STREAM) ? IPPROTO_TCP :
	    (sock->type == SOCK_DGRAM   ? IPPROTO_UDP : protocol);
    }
    /*
     * Create the socket.
     */
    if ((err = save_inet_create(sock, prot)) < 0) goto _exit0;
    /*
     * Now decide if we want to virtualize it.
     */
    switch(protocol)
    {
    case  IPPROTO_VNAT:
	sock->flags |= O_VNAT;
	/*
	 * Use global fixup and forced address translation flag.
	 */
	vnat_cmd_parm(&parm);

	PRINTK1(debug, ("[%s:%d|vnat_inet_create]: +socket[%d, %d]=%p\n", \
			current->comm, current->pid, \
			sock->type, protocol, sock));
	break;

    case -IPPROTO_VNAT:
	PRINTK2(debug, ("[%s:%d|vnat_inet_create]: -socket[%d, %d]=%p\n", \
			current->comm, current->pid, \
			sock->type, protocol, sock));
	break;

    default:
	if (vnat_cmd_verdict(current->comm, prot, &parm))
	{
	    sock->flags |= O_VNAT;

	    PRINTK1(debug, ("[%s:%d|vnat_inet_create]: +socket[%d, %d]=%p\n", \
			    current->comm, current->pid, \
			    sock->type, protocol, sock));
	}
	else
	{
	    PRINTK2(debug, ("[%s:%d|vnat_inet_create]: -socket[%d, %d]=%p\n", \
			    current->comm, current->pid, \
			    sock->type, protocol, sock));
	}
	break;
    }

    /*
     * If we decide to virtualize the socket, let's allocated the map entry
     * now. This may be a little bit inefficient if the socket is never
     * connected. But it makes things much easier for storing per-command
     * settings for the socket.
     */
    if (sock->flags & O_VNAT)
    {
	if ((map = vnat_map_kmem_cache_alloc()) == NULL)
	{
	    PRINTK4(debug, ("[%s:%d|vnat_inet_create]: \
?socket[%p, %d]=NOMEM\n", current->comm, current->pid, sock, protocol));
	    goto _exit0;
	}
	memset(map, 0, sizeof(vnat_map_t));
	map->sock = sock;
	map->parm = parm;
	/*
	 * Must succeed!
	 */
	if (vnat_map_insert(sock, map, inst_by_sock))
	{
	    PRINTK4(debug, ("[%s:%d|vnat_inet_create]: \
?socket[%p, %d]=EXIST\n", current->comm, current->pid, sock, protocol));

	    /*
	     * Stop virtualizing this socket.
	     */
	    vnat_map_kmem_cache_free(map);
	    sock->flags &= ~O_VNAT;
	}
	else
	{
	    MOD_INC_USE_COUNT;
	}
    }

 _exit0:
    MOD_DEC_USE_COUNT;

    return err;
}

/*
 * Our own "inet_release".
 */
int
vnat_inet_release(struct socket *sock)
{
    vnat_map_t *hit = NULL, map = { NULL };

    int err;

#ifdef DEBUG
    int protocol = 0;

    if (sock->sk)
    {
	lock_sock(sock->sk);
	protocol = sock->sk->protocol;
	release_sock(sock->sk);
    }
#endif

    /*
     * Never touched this socket before. It's ok to use "sock" after
     * calling original "inet_release" since "inet_release" does not
     * destroy the socket. "sock_release" will do that on return from
     * "inet_release".
     */
    if (!(sock->flags & O_VNAT))
    {
	PRINTK2(debug, ("[%s:%d|vnat_inet_release]: -close[%d, %d]=%p\n", \
			current->comm, current->pid, \
			sock->type, protocol, sock));
	return save_inet_release(sock);
    }

    MOD_INC_USE_COUNT;

    /*
     * Perform the original close.
     */
    if ((err = save_inet_release(sock)) < 0) goto _exit0;

    /*
     * The socket was not bound to a VIF.
     * Not necessarily bad since there may be sockets we intercepted but
     * never been bound or connected, e.g., ioctl socket.
     *
     * With the new scheme where a map entry is created at socket creation
     * time, "hit" should NEVER be NULL.
     */
    map.sock = sock;
    if (!(hit = vnat_map_delete(sock, &map, dele_by_sock)))
    {
	PRINTK2(debug, ("[%s:%d|vnat_inet_release]: -CLOSE[%d, %d]=%p\n", \
			current->comm, current->pid, \
			sock->type, protocol, sock));
	goto _exit0;
    }

    /*
     * If this is a "connected" socket, clean up the corresponding
     * netfilter entries and corresponding conntrack states.
     *
     * Check is now redundant as a map is created only for connected sockets.
     */
    if (hit->state & VNAT_MAP_CONNECTED)
    {
	vnat_iptables(hit, INCOMING, DEL);
	vnat_iptables(hit, OUTGOING, DEL);
    }

    /*
     * For the case of both ends are the same, we might need to fool
     * vnat_ifconfig (when there is no real NIC yet and the sockets
     * are still using their original NIC, see vnat_map_resume).
     */
    if (hit->l_vnat_addr == hit->r_vnat_addr)
	hit->l_vnat_addr = htonl(INADDR_LOOPBACK);
    vnat_ifconfig(hit, DN);

    /*kfree(hit);*/
    vnat_map_kmem_cache_free(hit);

    MOD_DEC_USE_COUNT;

    PRINTK1(debug, ("[%s:%d|vnat_inet_release]: +close[%d, %d]=%p\n", \
		    current->comm, current->pid, sock->type, protocol, sock));

 _exit0:
    MOD_DEC_USE_COUNT;

    return err;
}

#ifdef DEBUG
static int
vnat_inet_bind(struct socket *sock, struct sockaddr *uaddr, int addr_len)
{
    int err;

    err = save_inet_bind(sock, uaddr, addr_len);

    PRINTK4(debug, ("[%s:%d|vnat_inet_bind]: \
inet_bind[%p, %d.%d.%d.%d:%d, %d]=%d\n", current->comm, current->pid, \
		    sock, \
		    NIPQUAD(((struct sockaddr_in *)uaddr)->sin_addr.s_addr), \
		    ntohs(((struct sockaddr_in *)uaddr)->sin_port), \
		    addr_len, err));

    return err;
}
#endif

/*
 * Our own "inet_accept". We have to intercept it because "inet_accept"
 * "graft" a new "connected" socket from the "listening" socket.
 */
int
vnat_inet_accept(struct socket *sock, struct socket *newsock, int flags)
{
    vnat_map_t map, *sock_map, *newsock_map;

    int err;

    /*
     * Look for our "magic" flag.
     */
    if (!(sock->flags & O_VNAT))
    {
	PRINTK2(debug, ("[%s:%d|vnat_inet_accept]: \
?accept[%p, %p, %08x]=BYPASS\n", current->comm, current->pid, \
			sock, newsock, flags));
	return save_inet_accept(sock, newsock, flags);
    }

    MOD_INC_USE_COUNT;

    if ((err = save_inet_accept(sock, newsock, flags)) < 0)
    {
	PRINTK4(debug, ("[%s:%d|vnat_inet_accept]: \
?inet_accept[%p, %p, %08x]=%d\n", current->comm, current->pid, \
			sock, newsock, flags, err));
	goto _exit0;
    }
    /*
     * Look for the map entry for the listening socket. We need to copy
     * over the per-command configuration settings.
     *
     * Note that this search can be optimized away if we modify the
     * "struct socket" to include a pointer to our map entry.
     */
    map.sock = sock;
    if ((sock_map = vnat_map_find(sock, &map, find_by_sock)) == NULL)
    {
	PRINTK4(debug, ("[%s:%d|vnat_inet_accept]: \
?inet_accept[%p, %p, %08x]=NOMAP for listening socket\n", \
			current->comm, current->pid, \
			sock, newsock, flags));
	goto _exit0;
    }
    /*
     * If we can't virtualize the connection, it will fall back to a
     * normal one.
     */
    if ((newsock_map = vnat_map_kmem_cache_alloc()) == NULL)
    {
	PRINTK4(debug, ("[%s:%d|vnat_inet_accept]: \
?accept[%p, %p, %08x]=NOMEM\n", current->comm, current->pid, \
			sock, newsock, flags));
	goto _exit0;
    }
    /*
     * Now we know our "virtual" connection has been created, let's extract
     * necessary info for potential migration later.
     *
     * Note that if "accept" succeeds, newsock->sk must be valid!
     */
    memset(newsock_map, 0, sizeof(vnat_map_t));

    newsock_map->parm = sock_map->parm;

    newsock_map->sock = newsock;
    newsock_map->owner = current;
    newsock_map->state |= (VNAT_MAP_CONNECTED/*|VNAT_MAP_GRAFTED*/);
    newsock_map->keepopen = VNAT_MAP_KEEPOPEN;

    lock_sock(newsock->sk);
    newsock_map->l_kern_addr = newsock_map->l_vnat_addr = newsock->sk->saddr;
    newsock_map->l_kern_port = newsock_map->l_vnat_port = newsock->sk->sport;
    newsock_map->r_kern_addr = newsock_map->r_vnat_addr = newsock->sk->daddr;
    newsock_map->r_kern_port = newsock_map->r_vnat_port = newsock->sk->dport;
    release_sock(newsock->sk);

    /*
     * If fflag is set, setting [l|r]_user_[addr|port] are deferred until
     * the first time a "getname" is called on the socket.
     */
    newsock->flags |= O_VNAT;
    /*
    if (sock->flags & O_FIXUP)
    {
	newsock->flags |= O_FIXUP;
    }
    else
    */
    if (!newsock_map->parm.fflag)
    {
	newsock_map->l_user_addr = newsock_map->l_kern_addr;
	newsock_map->l_user_port = newsock_map->l_kern_port;
	newsock_map->r_user_addr = newsock_map->r_kern_addr;
	newsock_map->r_user_port = newsock_map->r_kern_port;
    }

    /*
     * Will always succeed; otherwise "accept" would have failed.
     */
    if (vnat_map_insert(newsock, newsock_map, inst_by_sock))
    {
	PRINTK4(debug, ("[%s:%d|vnat_inet_accept]: \
?accept[%p, %p, %08x]=EXIST\n", current->comm, current->pid, \
			sock, newsock, flags));

	vnat_map_kmem_cache_free(newsock_map);
	newsock->flags &= ~O_VNAT;
    }
    else
    {
	MOD_INC_USE_COUNT;
    }

    PRINTK1(debug, ("[%s:%d|vnat_inet_accept]: \
+accept[%p, %p, %08x]={%d:%d:%d:%d:%d, %d:%d:%d:%d:%d}\n", \
		    current->comm, current->pid, sock, newsock, flags, \
		    NIPQUAD(newsock_map->l_kern_addr), \
		    ntohs(newsock_map->l_kern_port), \
		    NIPQUAD(newsock_map->r_kern_addr), \
		    ntohs(newsock_map->r_kern_port)));

 _exit0:
    MOD_DEC_USE_COUNT;

    return err;
}

/*
 * Our own "inet_stream_connect".
 */
int
vnat_inet_stream_connect(struct socket *sock,
			 struct sockaddr *uaddr, int addr_len, int flags)
{
    vnat_map_t map, *sock_map;

    int err;

    if (!(sock->flags & O_VNAT))
    {
	PRINTK2(debug, ("[%s:%d|vnat_inet_stream_connect]: \
?connect[%p, %d.%d.%d.%d:%d, %d, %08x]=BYPASS\n", \
			current->comm, current->pid, sock, \
			NIPQUAD(((struct sockaddr_in *)uaddr)->sin_addr.s_addr), \
			ntohs(((struct sockaddr_in *)uaddr)->sin_port), \
			addr_len, flags));
	return save_inet_stream_connect(sock, uaddr, addr_len, flags);
    }

    MOD_INC_USE_COUNT;

    /*
     * Initially, our virtual connection will be _exactly the same_ as
     * the physical connection, so just let the "connect" call go through!
     */
    if ((err = save_inet_stream_connect(sock, uaddr, addr_len, flags)) < 0 &&
	err != -EINPROGRESS && err != -EALREADY)
    {
	PRINTK4(debug, ("[%s:%d|vnat_inet_stream_connect]: \
?inet_stream_connect[%p, %d.%d.%d.%d:%d, %d, %08x]=%d\n", \
			current->comm, current->pid, sock, \
			NIPQUAD(((struct sockaddr_in *)uaddr)->sin_addr.s_addr), \
			ntohs(((struct sockaddr_in *)uaddr)->sin_port), \
			addr_len, flags, err));
	goto _exit0;
    }
    /*
     * Search for our map entry. Should NEVER fail!
     *
     * Again note that this search can be optimized away if we were allowed
     * to added a pointer inside the "struct socket".
     */
    map.sock = sock;
    if ((sock_map = vnat_map_find(sock, &map, find_by_sock)) == NULL)
    {
	PRINTK4(debug, ("[%s:%d|vnat_inet_stream_connect]: \
?connect[%p, %d.%d.%d.%d:%d, %d, %08x]=NOMAP\n", \
			current->comm, current->pid, sock, \
			NIPQUAD(((struct sockaddr_in *)uaddr)->sin_addr.s_addr), \
			ntohs(((struct sockaddr_in *)uaddr)->sin_port), \
			addr_len, flags));
	goto _exit0;
    }
    /*
     * If we can't virtualize the connection, it will fall back to a
     * normal one.
     */
    /*
    if ((map = vnat_map_kmem_cache_alloc()) == NULL)
    {
	PRINTK4(debug, ("[%s:%d|vnat_inet_stream_connect]: \
?connect[%p, %d.%d.%d.%d:%d, %d, %08x]=NOMEM\n", \
			current->comm, current->pid, sock, \
			NIPQUAD(((struct sockaddr_in *)uaddr)->sin_addr.s_addr), \
			ntohs(((struct sockaddr_in *)uaddr)->sin_port), \
			addr_len, flags));
	goto _exit0;
    }
    */
    /*
     * Now we know our "virtual" connection has been created, let's extract
     * necessary info for potential migration later.
     *
     * Note that if "connect" succeeds, sock->sk must be valid!
     */
    /*memset(map, 0, sizeof(vnat_map_t));*/

    sock_map->sock = sock;
    sock_map->owner = current;
    sock_map->state |= (VNAT_MAP_CONNECTED/*|VNAT_MAP_GRAFTED*/);
    sock_map->keepopen = VNAT_MAP_KEEPOPEN;

    lock_sock(sock->sk);
    sock_map->l_kern_addr = sock_map->l_vnat_addr = sock->sk->saddr;
    sock_map->l_kern_port = sock_map->l_vnat_port = sock->sk->sport;
    sock_map->r_kern_addr = sock_map->r_vnat_addr = sock->sk->daddr;
    sock_map->r_kern_port = sock_map->r_vnat_port = sock->sk->dport;
    release_sock(sock->sk);

    /*
     * If fflag is set, setting [l|r]_user_[addr|port] are deferred until
     * the first time a "getname" is called on the socket.
     */
    /*if (!(sock->flags & O_FIXUP))*/
    if (!sock_map->parm.fflag)
    {
	sock_map->l_user_addr = sock_map->l_kern_addr;
	sock_map->l_user_port = sock_map->l_kern_port;
	sock_map->r_user_addr = sock_map->r_kern_addr;
	sock_map->r_user_port = sock_map->r_kern_port;
    }

    /*
     * Will always succeed; otherwise "connect" would have failed.
     *
     * Nullified by the new scheme.
     */
    /*vnat_map_insert(sock, map, inst_by_sock);*/

    /*MOD_INC_USE_COUNT;*/

    PRINTK1(debug, ("[%s:%d|vnat_inet_stream_connect]: \
+connect[%p, %d.%d.%d.%d:%d, %d, %08x]={%d.%d.%d.%d:%d, %d.%d.%d.%d:%d}\n", \
		    current->comm, current->pid, sock, \
		    NIPQUAD(((struct sockaddr_in *)uaddr)->sin_addr.s_addr), \
		    ntohs(((struct sockaddr_in *)uaddr)->sin_port), \
		    addr_len, flags, \
		    NIPQUAD(sock_map->l_kern_addr), \
		    ntohs(sock_map->l_kern_port), \
		    NIPQUAD(sock_map->r_kern_addr), \
		    ntohs(sock_map->r_kern_port)));

 _exit0:
    MOD_DEC_USE_COUNT;

    return err;
}

/*
 * Our own "inet_dgram_connect".
 */
int
vnat_inet_dgram_connect(struct socket *sock,
			struct sockaddr *uaddr, int addr_len, int flags)
{
#ifdef DEBUG
    struct sockaddr_in *addr = (struct sockaddr_in *)uaddr/*, laddr*/;
#endif
    int err;

    MOD_INC_USE_COUNT;

    if (!(sock->flags & O_VNAT))
    {
	PRINTK2(debug, ("[%s:%d|vnat_inet_dgram_connect]: \
-connect[%p, %d.%d.%d.%d:%d]\n", current->comm, current->pid, sock, \
			NIPQUAD(addr->sin_addr.s_addr), \
			ntohs(addr->sin_port)));

	err = save_inet_dgram_connect(sock, uaddr, addr_len, flags);
	goto _exit0;
    }

    PRINTK1(debug, ("[%s:%d|vnat_inet_dgram_connect]: \
+connect[%p, %d.%d.%d.%d:%d]\n", current->comm, current->pid, sock, \
		    NIPQUAD(addr->sin_addr.s_addr), ntohs(addr->sin_port)));

    /*
    memset((char *)&laddr, 0, sizeof(laddr));
    laddr.sin_family = AF_INET;
    laddr.sin_addr.s_addr = htonl(INADDR_ANY);
    laddr.sin_port = htons(0);

    if ((err =
	 vnat_inet_bind(sock, (struct sockaddr *)&laddr, sizeof(laddr))) < 0)
    {
	PRINTK4(debug,
		("[vnat_inet_dgram_connect]: vnat_inet_bind failed, \
errno = %d\n", err));
	goto _exit0;
    }
    */

    err = save_inet_dgram_connect(sock, uaddr, addr_len, flags);

 _exit0:
    MOD_DEC_USE_COUNT;

    return err;
}

/*
 * Our own "inet_getname".
 */
int
vnat_inet_getname(struct socket *sock, struct sockaddr *uaddr, int *uaddr_len,
		  int peer)
{
    struct files_struct *files = current->files;
    struct sockaddr_in *sin = (struct sockaddr_in *)uaddr;

    vnat_map_t *hit, *hit2, map = { NULL };
    struct socket *master = NULL;

    int i, err;

    /*
     * Look for our magic flag.
     */
    if (!(sock->flags & O_VNAT))
    {
	PRINTK2(debug, ("[%s:%d|vnat_inet_getname]: \
?getname[%p, %p, %d, %d]=BYPASS\n", current->comm, current->pid, sock, uaddr, \
			*uaddr_len, peer));
	return save_inet_getname(sock, uaddr, uaddr_len, peer);
    }

    MOD_INC_USE_COUNT;

    if ((err = save_inet_getname(sock, uaddr, uaddr_len, peer)) < 0)
    {
	PRINTK4(debug, ("[%s:%d|vnat_int_getname]: \
?inet_getname[%p, %p, %d, %d]=%d\n", current->comm, current->pid, sock, \
			uaddr, *uaddr_len, peer, err));
	goto _exit0;
    }

    /*
     * If O_VNAT is set, NOMAP should NEVER happen!
     *
     * If the socket is not connected, we will return normally too.
     *
     * If the socket is connected but the fflag is not set, we will return
     * normally too.
     */
    map.sock = sock;
    if (((hit = vnat_map_find(sock, &map, find_by_sock)) == NULL) ||
	!(hit->state & VNAT_MAP_CONNECTED) ||
	!(hit->parm.fflag))
    {
	PRINTK2(debug, ("[%s:%d|vnat_inet_getname]: \
?getname[%p, %p, %d, %d]=NOMAP or DISCONNECTED or no-FIXUP\n", \
			current->comm, current->pid, sock, uaddr, \
			*uaddr_len, peer));
	goto _exit0;
    }
    /*
     * All the "fixup" logic is resolved at the first time when the
     * "getname" call is made on a socket.
     */
    /*if (!(sock->flags & O_FIXUP))*/
    /*
    if (!hit->parm.fflag)
    {
	sin->sin_port = peer ? hit->r_user_port : hit->l_user_port;
	sin->sin_addr.s_addr = peer ? hit->r_user_addr : hit->l_user_addr;

	goto _exit0;
    }
    */

    /*
     * If the values we need are already set, simply return them.
     *
     * Note that there is a bug in either sys_accept which calls inet_getname
     * with peer == 2, or inet_getname which explicitly checks peer == 1.
     */
    if ((peer == 0 &&
	 (sin->sin_port = hit->l_user_port) &&
	 (sin->sin_addr.s_addr = hit->l_user_addr)) ||
	(peer != 0 &&
	 (sin->sin_port = hit->r_user_port) &&
	 (sin->sin_addr.s_addr = hit->r_user_addr))) goto _exit0;

    /*
     * Search our owner's open file descriptors and find a connected socket
     * that is _not_ ourselves.
     *
     * Note that we are _not_ checking "files", it _must_ not be NULL since
     * we know at least we will find ourselves!
     */
    read_lock(&files->file_lock);
    for (i = 0; i < files->max_fds; i++)
    {
	struct file *file = files->fd[i];
	struct inode *inode = NULL;

	/*
	 * Skip ourselves.
	 */
	if (file == sock->file) continue;
	/*
	 * Chase down the inode pointer from the file pointer.
	 */
	if (file && file->f_dentry) inode = file->f_dentry->d_inode;
	/*
	 * Is it a connected INET socket?
	 */
	if (inode && (inode->i_mode & S_IFMT) == S_IFSOCK)
	{
	    master = &(inode->u.socket_i);

	    if (master->state == SS_CONNECTED && master->type == SOCK_STREAM)
		break;
	}
    }
    read_unlock(&files->file_lock);

    /*
     * We are the master!
     */
    if (i == files->max_fds)
    {
	hit->l_user_port = hit->l_kern_port;
	hit->r_user_port = hit->r_kern_port;
	hit->l_user_addr = hit->l_kern_addr;
	hit->r_user_addr = hit->r_kern_addr;

	sin->sin_port = peer ? hit->r_user_port : hit->l_user_port;
	sin->sin_addr.s_addr = peer ? hit->r_user_addr : hit->l_user_addr;

	PRINTK4(debug, ("[%s:%d|vnat_inet_getname]: \
+getname[%p, %p, %d, %d]=setting master socket (%p) user addresses and ports \
{%d.%d.%d.%d:%d, %d.%d.%d.%d:%d}\n", current->comm, current->pid, \
			sock, uaddr, *uaddr_len, peer, \
			sock, \
			NIPQUAD(hit->l_user_addr), ntohs(hit->l_user_port), \
			NIPQUAD(hit->r_user_addr), ntohs(hit->r_user_port)));
	goto _exit0;
    }
    /*
     * Find map for the master.
     */
    map.sock = master;
    if ((hit2 = vnat_map_find(master, &map, find_by_sock)) == NULL)
    {
	PRINTK4(debug, ("[%s:%d|vnat_inet_getname]: \
?getname[%p, %p, %d, %d]=NOMAP for master socket (%p)\n", \
			current->comm, current->pid, \
			sock, uaddr, *uaddr_len, peer, master));
	goto _exit0;
    }
    /*
     * Copy from master.
     */
    hit->l_user_port = hit2->l_user_port;
    hit->r_user_port = hit2->r_user_port;
    hit->l_user_addr = hit2->l_user_addr;
    hit->r_user_addr = hit2->r_user_addr;

    sin->sin_port = peer ? hit->r_user_port : hit->l_user_port;
    sin->sin_addr.s_addr = peer ? hit->r_user_addr : hit->l_user_addr;

    PRINTK1(debug, ("[%s:%d|vnat_inet_getname]: \
+getname[%p, %p, %d, %d]=copying master socket (%p) user addresses and ports \
{%d.%d.%d.%d:%d, %d.%d.%d.%d:%d}\n", current->comm, current->pid, \
		    sock, uaddr, *uaddr_len, peer, \
		    master, \
		    NIPQUAD(hit->l_user_addr), ntohs(hit->l_user_port), \
		    NIPQUAD(hit->r_user_addr), ntohs(hit->r_user_port)));

 _exit0:
    PRINTK1(debug, ("[%s:%d|vnat_inet_getname]: \
+getname[%p, %p, %d, %d]=%d.%d.%d.%d:%d\n", current->comm, current->pid, \
		    sock, uaddr, *uaddr_len, peer, \
		    NIPQUAD(sin->sin_addr.s_addr), ntohs(sin->sin_port)));

    MOD_DEC_USE_COUNT;

    return err;
}
