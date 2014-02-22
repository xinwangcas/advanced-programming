/*
 * vcmp.c
 *
 * VNAT Connection Migration Protocol (VCMP) routines.
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

#include <linux/kernel.h>

#include <linux/socket.h> /* struct sockaddr            */
#include <linux/in.h>     /* struct sockaddr_in         */
#include <linux/timer.h>
#include <net/sock.h>     /* struct sock, (un)lock_sock */
#include <net/tcp.h>
#include <asm/uaccess.h>  /* KERNEL_DS, get_fs          */

#include "vnat.h"
#include "vnatd.h"
#include "vnatmap.h"
#include "vnatrule.h"
#include "vnatvcmp.h"
#include "misc.h"

#include "vnatsyms.h"

#define SET_SNDTIMEO(sock, timeout) \
do { \
    lock_sock((sock)->sk); \
    (sock)->sk->sndtimeo = (timeout); \
    release_sock((sock)->sk); \
} while(0)

#define SET_RCVTIMEO(sock, timeout) \
do { \
    lock_sock((sock)->sk); \
    (sock)->sk->rcvtimeo = (timeout); \
    release_sock((sock)->sk); \
} while(0)

static int  find_by_lvap_rvap(vnat_map_t *p1, vnat_map_t *p2);

static void vcmp_inithdr(struct vcmp_msg *msg, int flags);
static void vcmp_fillpdu(struct vcmp_msg *msg, char command);
static int  vcmp_pullpdu(struct vcmp_msg *msg, char command);

static void
vcmp_inithdr(vcmp_msg_t *msg, int flags)
{
    msg->hdr.msg_name       = NULL;
    msg->hdr.msg_namelen    = 0;
    msg->hdr.msg_iov        = &msg->iov;
    msg->hdr.msg_iovlen     = 1;
    msg->hdr.msg_control    = NULL;
    msg->hdr.msg_controllen = 0;
    msg->hdr.msg_flags      = flags;

    msg->hdr.msg_iov->iov_base = &msg->pdu;
    msg->hdr.msg_iov->iov_len  = sizeof(msg->pdu);
}

static int
find_by_lvap_rvap(vnat_map_t *p1, vnat_map_t *p2)
{
    return (p1->l_vnat_addr == p2->l_vnat_addr &&
	    p1->l_vnat_port == p2->l_vnat_port &&
	    p1->r_vnat_addr == p2->r_vnat_addr &&
	    p1->r_vnat_port == p2->r_vnat_port);
}

static void
vcmp_fillpdu(vcmp_msg_t *msg, char command)
{
    msg->pdu.version = VCMP_VER;
    msg->pdu.command = command;

    switch(command)
    {
    case VCMP_UPD: /* Client sending */
    case VCMP_UPD_R_ACK:
	/*
	 * How do we identify a virtualized connection?
	 *
	 * Although theoretically the following virtual address and
	 * port pairs are allowed on a single physical node, we can
	 * not let this happen since incoming packets are impossible
	 * to demultiplex, as shown in the diagram below.
	 *
	 *     vaddr1:vport vaddr2:vport vaddr3:vport
	 *            \           |          /
	 *              \         |        /
	 *                \       |      /
	 *                  vaddr:vport
	 *
	 * In order to be able to demultiplex incoming packets, the
	 * mapping has to be:
	 *
	 *     vaddr1:vport1 vaddr2:vport2 vaddr3:vport3
	 *            \            |            /
	 *              \          |          /
	 *                \        |        /
	 *                    vaddr:vport1
	 *                    vaddr:vport2
	 *                    vaddr:vport3
	 *
	 * In other words, a different virtual port must be used with
	 * each different virtual address since these different virtual
	 * addresses may eventually be mapped to the same physical
	 * address and thus becoming indistinguishable.
	 */

	/*
	 * Remote half to be matched.
	 */
	msg->pdu.r_korv_port = msg->map->r_vnat_port;
	msg->pdu.r_korv_addr = msg->map->r_vnat_addr;
	/*
	 * Local half to be matched.
	 */
	msg->pdu.l_korv_addr = msg->map->l_vnat_addr;
	msg->pdu.l_korv_port = msg->map->l_vnat_port;
	/*
	 * These need to be updated. Placehold for now. Server will
	 * get these from protocol stack.
	 *
	 * l_vnat_addr is overloaded with epoch.
	 */
	msg->pdu.l_vnat_addr = htonl(jiffies);
	msg->pdu.l_vnat_port = msg->map->l_vnat_port;

	PRINTK4(debug, ("ver=%d cmd=%d r_korv_port=%d \
r_korv_addr=%d.%d.%d.%d l_korv_addr=%d.%d.%d.%d l_korv_port=%d \
l_vnat_addr=%d (epoch) l_vnat_port=%d", \
			msg->pdu.version, msg->pdu.command, \
			ntohs(msg->pdu.r_korv_port), \
			NIPQUAD(msg->pdu.r_korv_addr), \
			NIPQUAD(msg->pdu.l_korv_addr), \
			ntohs(msg->pdu.l_korv_port), \
			ntohl(msg->pdu.l_vnat_addr), \
			ntohs(msg->pdu.l_vnat_port)));
	break;

    case VCMP_UPD_R: /* Server sending */
	/*
	 * For client verification.
	 */
	msg->pdu.r_korv_port = msg->map->r_vnat_port;
	msg->pdu.r_korv_addr = msg->map->r_vnat_addr;
	msg->pdu.l_korv_addr = msg->map->l_vnat_addr;
	msg->pdu.l_korv_port = msg->map->l_vnat_port;
	/*
	 * Server sets it to client address.
	 */
	msg->pdu.l_vnat_addr = msg->map->r_vnat_addr;
	msg->pdu.l_vnat_port = msg->map->r_vnat_port;

	PRINTK4(debug, ("ver=%d cmd=%d r_korv_port=%d \
r_korv_addr=%d.%d.%d.%d l_korv_addr=%d.%d.%d.%d l_korv_port=%d \
l_vnat_addr=%d.%d.%d.%d l_vnat_port=%d", \
			msg->pdu.version, msg->pdu.command, \
			ntohs(msg->pdu.r_korv_port), \
			NIPQUAD(msg->pdu.r_korv_addr), \
			NIPQUAD(msg->pdu.l_korv_addr), \
			ntohs(msg->pdu.l_korv_port), \
			NIPQUAD(msg->pdu.l_vnat_addr), \
			ntohs(msg->pdu.l_vnat_port)));
	break;

    case VCMP_SUSP: /* Client sending */
	/*
	 * Remote half to be matched.
	 */
	msg->pdu.r_korv_port = msg->map->r_vnat_port;
	msg->pdu.r_korv_addr = msg->map->r_vnat_addr;
	/*
	 * Local half to be matched.
	 */
	msg->pdu.l_korv_addr = msg->map->l_vnat_addr;
	msg->pdu.l_korv_port = msg->map->l_vnat_port;
	/*
	 * l_vnat_addr is overloaded for keepalive time.
	 */
	msg->pdu.l_vnat_addr = htonl(msg->map->parm.tcpka);

	PRINTK4(debug, ("ver=%d cmd=%d r_korv_port=%d \
r_korv_addr=%d.%d.%d.%d l_korv_addr=%d.%d.%d.%d l_korv_port=%d \
l_vnat_addr=%d (keepalive_time)", \
			msg->pdu.version, msg->pdu.command, \
			ntohs(msg->pdu.r_korv_port), \
			NIPQUAD(msg->pdu.r_korv_addr), \
			NIPQUAD(msg->pdu.l_korv_addr), \
			ntohs(msg->pdu.l_korv_port), \
			ntohl(msg->pdu.l_vnat_addr)));
	break;

    default:
	PRINTK4(debug, ("unknown message type"));
	break;
    }
}

static int
vcmp_pullpdu(vcmp_msg_t *msg, char command)
{
    vnat_map_t map /*= { NULL, NULL, msg->sock }*/;

    u32 l_prot_addr;
    u32 r_prot_addr;
    u16 r_prot_port;

    u32 rcv_tsval;

    if (msg->pdu.command != command && command != VCMP_ALL)
	return -EPROTONOSUPPORT;

    lock_sock(msg->sock->sk);
    l_prot_addr = msg->sock->sk->rcv_saddr;
    r_prot_addr = msg->sock->sk->daddr;
    r_prot_port = msg->sock->sk->dport;
    release_sock(msg->sock->sk);

    switch(msg->pdu.command)
    {
    case VCMP_UPD: /* Server receiving */
	PRINTK4(debug, ("ver=%d cmd=%d r_korv_port=%d \
r_korv_addr=%d.%d.%d.%d l_korv_addr=%d.%d.%d.%d l_korv_port=%d \
l_vnat_addr=%d (epoch) l_vnat_port=%d\n", \
			msg->pdu.version, msg->pdu.command, \
			ntohs(msg->pdu.r_korv_port), \
			NIPQUAD(msg->pdu.r_korv_addr), \
			NIPQUAD(msg->pdu.l_korv_addr), \
			ntohs(msg->pdu.l_korv_port), \
			ntohl(msg->pdu.l_vnat_addr), \
			ntohs(msg->pdu.l_vnat_port)));

	/*
	 * Note that we are NOT updating [l|r]_user_addr and [l|r]_user_port.
	 */
	map.l_vnat_port = msg->pdu.r_korv_port;
	map.l_vnat_addr = msg->pdu.r_korv_addr;
	map.r_vnat_port = msg->pdu.l_korv_port;
	map.r_vnat_addr = msg->pdu.l_korv_addr;
	if ((msg->map = vnat_map_find_all(&map, find_by_lvap_rvap)) == NULL)
	    return -EADDRNOTAVAIL;

	/*
	 * Restore TCP keepalive timer if it was negotiated.
	 *
	 * The check is unnecessary.
	 */
	if (msg->map->sock && msg->map->sock->sk)
	{
	    struct socket *sock = msg->map->sock;
	    struct sock *sk = sock->sk;
	    struct tcp_opt *tp = &(sk->tp_pinfo.af_tcp);

	    lock_sock(sk);

	    if (msg->map->keepopen != VNAT_MAP_KEEPOPEN)
	    {
		tp->keepalive_time = msg->map->keepalive_time;
		/*
		 * If the TCP keepalive timer used to be enabled, restore it
		 * regardless of whether the timer is enabled or not now.
		 *
		 * FIXME: if the original timer was enabled with a default
		 * system wide value (tp->keepalive_time == 0), we need
		 * access to sysctl_tcp_keepalive_time here.
		 */
		PRINTK4(debug, ("resuming...socket[%p]: keepalive=", sock));

		if (msg->map->keepopen)
		{
		    PRINTK4(debug, ("%d\n", tp->keepalive_time));

		    sk->keepopen = msg->map->keepopen;
#if defined(vnat_sysctl_tcp_keepalive_time)
		    tcp_reset_keepalive_timer(sk, tp->keepalive_time ? :
					      *((int *)vnat_sysctl_tcp_keepalive_time));
#else
#error "Cannot resolve symbol sysctl_tcp_keepalive_time"
#endif
		}
		/*
		 * If the TCP keepalive timer used to be disabled but we
		 * enabled it during suspension, disable it.
		 */
		else if (sk->keepopen)
		{
		    PRINTK4(debug, ("disabled\n"));

		    sk->keepopen = 0;
		    tcp_delete_keepalive_timer(sk);
		}
		else
		{
		    PRINTK4(debug, ("nochange\n"));
		}
	    }

	    /*
	     * Hack for PAWS/RTTM.
	     */
	    rcv_tsval = ntohl(msg->pdu.l_vnat_addr);
	    if (tp->ts_recent >= rcv_tsval)
	    {
		long delta;

		PRINTK4(debug, ("resuming...socket[%p]: ts_recent=%d->", \
				sock, tp->ts_recent));
		/*
		 * Roll the peer's PAWS stamp back based on what _our_
		 * elapsed time (and implicitly assume that the peer is
		 * ticking at the same rate).
		 *
		 * The idea is: if I see a peer stamp of rcv_tsval _now_
		 * and I know delta jiffies have elapsed since the last
		 * time I saw rcv_tsval, then the last rcv_tsval I saw
		 * must have been (rcv_tsval - delta), assuming the peer
		 * is ticking at the same rate as mine.
		 */
		delta = jiffies - (tp->ts_recent_stamp > (ULONG_MAX/HZ) ?
				   ULONG_MAX : tp->ts_recent_stamp*HZ);
		tp->ts_recent =	delta > 0 ? rcv_tsval - delta : 0;

		PRINTK4(debug, ("%d\n", tp->ts_recent));
	    }

	    release_sock(sk);

	    /*
	     * Before updating our address mapping, delete the old mapping.
	     */
	    vnat_iptables(msg->map, INCOMING, DEL);
	    vnat_iptables(msg->map, OUTGOING, DEL);

	    /*
	     * Ok, let's update the address mapping entry.
	     *
	     * Remember that this is an "out-of-band" protocol connection,
	     * not a virtualized connection. We use the socket layer info
	     * only to find the client's new physical address and inform
	     * client (ping-pong).
	     */
	    msg->map->r_vnat_addr = r_prot_addr;
	    msg->map->r_vnat_port = msg->pdu.l_vnat_port;
	    /*
	     * A just waken up server _can_ initiate traffic! Example, SSH
	     * rekey request. So now we will always add rules in pairs!
	     */
	    if (vnat_iptables(msg->map, INCOMING, ADD) < 0)
		return -ENETUNREACH;
	    if (vnat_iptables(msg->map, OUTGOING, ADD) < 0)
	    {
		vnat_iptables(msg->map, INCOMING, DEL);
		return -ENETUNREACH;
	    }
	    break;

	case VCMP_UPD_R_ACK: /* Server receiving */
	    PRINTK4(debug, ("ver=%d cmd=%d r_korv_port=%d \
r_korv_addr=%d.%d.%d.%d l_korv_addr=%d.%d.%d.%d l_korv_port=%d \
l_vnat_addr=%d (epoch) l_vnat_port=%d\n", \
			    msg->pdu.version, msg->pdu.command, \
			    ntohs(msg->pdu.r_korv_port), \
			    NIPQUAD(msg->pdu.r_korv_addr), \
			    NIPQUAD(msg->pdu.l_korv_addr), \
			    ntohs(msg->pdu.l_korv_port), \
			    ntohl(msg->pdu.l_vnat_addr), \
			    ntohs(msg->pdu.l_vnat_port)));

	    map.l_vnat_port = msg->pdu.r_korv_port;
	    map.l_vnat_addr = msg->pdu.r_korv_addr;
	    map.r_vnat_port = msg->pdu.l_korv_port;
	    map.r_vnat_addr = msg->pdu.l_korv_addr;
	    if ((msg->map = vnat_map_find_all(&map, find_by_lvap_rvap)) == NULL)
		return -EADDRNOTAVAIL;

	    /*
	     * Finally, let's wake up the stopped process.
	     *
	     * FIXME: we might want to delay this. Since the peer will not
	     * be ready until it receives the reply from our kvnatd.
	     * Hopefully the half-waken-up connection will be kept alive
	     * by TCP for a while until our peer installs vnat rules and
	     * becomes ready.
	     */
	    if (msg->map->owner != NULL)
	    {
		struct task_struct *t = msg->map->owner;
#ifdef DEBUG
		pid_t pid = t->pid;
#endif

		PRINTK4(debug, ("resuming...socket:proc[%p:%d]: \
epoch=%ld, suspend_time=%d, expires=%ld, ", sock, pid, jiffies, \
				msg->map->suspend_time, \
				t->real_timer.expires));

		/*
		 * If the timer had already fired off, the "expires" would be
		 * useless.
		 */
		if (t->real_timer.expires - msg->map->suspend_time > 0)
		{
		    t->real_timer.expires += jiffies - msg->map->suspend_time;
		    /*
		     * Put the timer back if it was active at the time of
		     * suspension. Shift the expiration back properly.
		     */
		    if (msg->map->active_timer)
		    {
			add_timer(&t->real_timer);

			PRINTK4(debug, ("modified expires=%ld, active\n", \
					t->real_timer.expires));
		    }
		    else
		    {
			PRINTK4(debug, ("modified expires=%ld, inactive\n", \
					t->real_timer.expires));
		    }
		}

		/*if (msg->map->state & VNAT_MAP_GRAFTED)*/
		{
		    PRINTK4(debug, ("resuming...waking up [%s:%d]\n", \
				    t->comm, t->pid));
		    wake_up_process(t);
		}
	    }
	    else
	    {
		PRINTK4(debug, ("resuming...socket:proc[%p:%p]: \
no process stopped\n", sock, msg->map->owner));
	    }
	}

	msg->sock = NULL;
	break;

    case VCMP_UPD_R: /* Client receiving */
	PRINTK4(debug, ("ver=%d cmd=%d r_korv_port=%d \
r_korv_addr=%d.%d.%d.%d l_korv_addr=%d.%d.%d.%d l_korv_port=%d \
l_vnat_addr=%d.%d.%d.%d l_vnat_port=%d\n", \
			msg->pdu.version, msg->pdu.command, \
			ntohs(msg->pdu.r_korv_port), \
			NIPQUAD(msg->pdu.r_korv_addr), \
			NIPQUAD(msg->pdu.l_korv_addr), \
			ntohs(msg->pdu.l_korv_port), \
			NIPQUAD(msg->pdu.l_vnat_addr), \
			ntohs(msg->pdu.l_vnat_port)));

	/*
	 * Before updating our address mapping, delete the old mapping.
	 */
	vnat_iptables(msg->map, OUTGOING, DEL);
	vnat_iptables(msg->map, INCOMING, DEL);

	/*
	 * Update our address from what server tells us (ping-pong).
	 *
	 * Note that we are NOT updating [l|r]_user_addr and [l|r]_user_port.
	 */
	msg->map->l_vnat_addr = msg->pdu.l_vnat_addr;
	msg->map->l_vnat_port = msg->pdu.l_vnat_port;

	if (msg->pdu.r_korv_port != msg->map->l_vnat_port ||
	    msg->pdu.r_korv_addr != msg->map->l_vnat_addr ||
	    msg->pdu.l_korv_addr != msg->map->r_vnat_addr ||
	    msg->pdu.l_korv_port != msg->map->r_vnat_port) return -EREMCHG;

	if (vnat_ifconfig(msg->map, UP) < 0) return -NODEV;

	/*
	 * Ok, let's update the address mapping entry.
	 */
	if (vnat_iptables(msg->map, OUTGOING, ADD) < 0)
	{
	    vnat_ifconfig(msg->map, DN);
	    return -ENETUNREACH;
	}
	if (vnat_iptables(msg->map, INCOMING, ADD) < 0)
	{
	    vnat_iptables(msg->map, OUTGOING, DEL);
	    vnat_ifconfig(msg->map, DN);
	    return -ENETUNREACH;
	}

	break;

    case VCMP_SUSP: /* Server receiving */
	PRINTK4(debug, ("ver=%d cmd=%d r_korv_port=%d \
r_korv_addr=%d.%d.%d.%d l_korv_addr=%d.%d.%d.%d l_korv_port=%d \
l_vnat_addr=%d (keepalive_time)\n", \
			msg->pdu.version, msg->pdu.command, \
			ntohs(msg->pdu.r_korv_port), \
			NIPQUAD(msg->pdu.r_korv_addr), \
			NIPQUAD(msg->pdu.l_korv_addr), \
			ntohs(msg->pdu.l_korv_port), \
			ntohl(msg->pdu.l_vnat_addr)));

	map.l_vnat_port = msg->pdu.r_korv_port;
	map.l_vnat_addr = msg->pdu.r_korv_addr;
	map.r_vnat_port = msg->pdu.l_korv_port;
	map.r_vnat_addr = msg->pdu.l_korv_addr;
	if ((msg->map = vnat_map_find_all(&map, find_by_lvap_rvap)) == NULL)
	    return -EADDRNOTAVAIL;

	/*
	 * Stop the process handling this connection.
	 * We update the real owner process of the socket at this point.
	 */
	if (msg->map->sock &&
	    ((msg->map->owner = sock_owner(msg->map->sock)) != NULL))
	{
	    struct task_struct *t = msg->map->owner;

	    /*if (msg->map->state & VNAT_MAP_GRAFTED)*/
	    {
		PRINTK4(debug, ("suspending...stopping [%s:%d]\n", \
				t->comm, t->pid));
		send_sig(SIGSTOP, t, 1);
	    }

	    /*
	     * Save the epoch.
	     */
	    msg->map->suspend_time = jiffies;

	    PRINTK4(debug, ("suspending...socket:proc[%p:%d]: \
epoch=%d, expires=%ld, ", msg->map->sock, t->pid, \
			    msg->map->suspend_time, t->real_timer.expires));
	    /*
	     * If the process has an active timer, deactivate it.
	     */
	    if (timer_pending(&t->real_timer))
	    {
		del_timer(&t->real_timer);
		msg->map->active_timer = 1;

		PRINTK4(debug, ("deactivated\n"));
	    }
	    else
	    {
		PRINTK4(debug, ("inactive\n"));
	    }
	}

	/*
	 * Set TCP keepalive timer.
	 */
	if (msg->map->sock && msg->map->sock->sk)
	{
	    struct socket *sock = msg->map->sock;
	    struct sock *sk = sock->sk;
	    struct tcp_opt *tp = &(sk->tp_pinfo.af_tcp);

	    lock_sock(sk);
	    /*
	     * Save the TCP keepalive timer.
	     */
	    msg->map->keepopen = sk->keepopen;
	    msg->map->keepalive_time = tp->keepalive_time;
	    /*
	     * If the suspending peer supplied a timer value, use it.
	     * Note we don't care if the timer was already enabled or
	     * not, or it has already been idle for a while. We count
	     * everything starting from NOW!
	     */
	    PRINTK4(debug, ("suspending...socket[%p]: keepalive=", \
			    sock));

	    if ((tp->keepalive_time = ntohl(msg->pdu.l_vnat_addr) * HZ))
	    {
                PRINTK4(debug, ("%d\n", tp->keepalive_time));

		sk->keepopen = 1;
		tcp_reset_keepalive_timer(sk, tp->keepalive_time);
	    }
	    /*
	     * If the suspending peer didn't supply a timer value, but
	     * the timer is currently enabled, disable it.
	     */
	    else if (sk->keepopen)
	    {
                PRINTK4(debug, ("disabled\n"));

		sk->keepopen = 0;
		tcp_delete_keepalive_timer(sk);
	    }
            else
	    {
		PRINTK4(debug, ("nochange\n"));
	    }
	     release_sock(sk);
	}

	/*
	 * Signal that we don't need to reply for this message.
	 */
	msg->sock = NULL;
	break;

    default:
	PRINTK4(debug, ("unknown message type\n"));
	return -EPROTONOSUPPORT;
    }

    return 0;
}

int
vcmp_socket(unsigned int addr, unsigned short port, struct socket **sock,
	    long timeout)
{
    struct sockaddr_in laddr;

    int err;

    PRINTK4(debug, ("[%s:%d|vcmp_socket]: {", current->comm, current->pid));

    PRINTK4(debug, ("%d.%d.%d.%d:%d, %ld", \
		    NIPQUAD(addr), ntohs(port), timeout));

    if ((err = sock_create(PF_INET, SOCK_STREAM, -IPPROTO_VNAT, sock)) < 0)
	goto _exit0;

    memset((char *)&laddr, 0, sizeof(laddr));
    laddr.sin_family = AF_INET;
    laddr.sin_addr.s_addr = addr;
    laddr.sin_port = port;

    if (timeout) SET_SNDTIMEO(*sock, timeout*HZ);
    if ((err = (*sock)->ops->connect(*sock, (struct sockaddr *)&laddr,
				     sizeof(laddr), O_RDWR)) < 0) goto _exit1;
    if (timeout) SET_SNDTIMEO(*sock, MAX_SCHEDULE_TIMEOUT);

    goto _exit0;

 _exit1:
    sock_release(*sock);

 _exit0:
    PRINTK4(debug, ("}=%d\n", err));
    return err;
}

int
vcmp_sendmsg(struct socket *sock, vcmp_msg_t *vcmp, char command, int flags,
	     long timeout)
{
    mm_segment_t oldfs;
    int          len;

    vcmp_inithdr(vcmp, flags);

    PRINTK4(debug, ("[%s:%d|vcmp_sendmsg]: {", current->comm, current->pid));

    vcmp_fillpdu(vcmp, command);

    oldfs = get_fs();
    set_fs(KERNEL_DS);
    if (timeout) SET_SNDTIMEO(sock, timeout*HZ);
    len = sock_sendmsg(sock, &vcmp->hdr, sizeof(vcmp->pdu));
    if (timeout) SET_SNDTIMEO(sock, MAX_SCHEDULE_TIMEOUT);
    set_fs(oldfs);

    PRINTK4(debug, ("}=%d\n", len));

    return len;
}

int
vcmp_recvmsg(struct socket *sock, vcmp_msg_t *vcmp, char command, int flags,
	     long timeout)
{
    mm_segment_t oldfs;
    int          len, err;

    vcmp_inithdr(vcmp, flags);

    PRINTK4(debug, ("[%s:%d|vcmp_recvmsg]: { ...\n", \
		    current->comm, current->pid));

    oldfs = get_fs();
    set_fs(KERNEL_DS);
    if (timeout) SET_RCVTIMEO(sock, timeout*HZ);
    len = sock_recvmsg(sock, &vcmp->hdr, sizeof(vcmp->pdu), 0);
    if (timeout) SET_SNDTIMEO(sock, MAX_SCHEDULE_TIMEOUT);
    set_fs(oldfs);

    if (len <= 0) PRINTK4(debug, ("transport error\n"));
    /*
     * Note we only reset "len" if there is an error from vcmp_pullpdu.
     */
    else if ((err = vcmp_pullpdu(vcmp, command)) < 0) len = err;

    PRINTK4(debug, ("[%s:%d|vcmp_recvmsg]: ... }=%d\n", \
		    current->comm, current->pid, len));

    return len;
}
