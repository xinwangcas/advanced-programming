/*
 * vnatrule.c
 *
 * VNAT address translation rule setup (via netfilter) module.
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

#include <linux/net.h> /* needed by in.h */
#include <linux/in.h>  /* INADDR_ANY     */
#include <linux/if.h>  /* IFNAMSIZ       */
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/rtnetlink.h>

#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ip_conntrack_tuple.h>
#include <linux/netfilter_ipv4/ip_conntrack_core.h>

#include "vnat.h"
#include "vnatmap.h"
#include "vnatrule.h"
#include "vnatvif.h"
#include "misc.h"
#include "umodecall.h"

static int vnat_inet_set_ifa(struct net_device *dev, u32 s_addr);
static int vnat_conntrack_del(vnat_map_t *map);

int
vnat_ifconfig(vnat_map_t *map, int act)
{
    char vifname[IFNAMSIZ];
    struct net_device *dev;
    int err;

    /*
     * Only create the VIF if there is no NIC with the same IP address.
     */
    if (map->l_kern_addr == map->l_vnat_addr) return 0;

    switch (act)
    {
    case UP:
	/*
	 * Map entry already bound to a VIF.
	 */
	if (map->vif) return -EFAULT;

	/*
	 * Create the VIF if it's not already there.
	 */
	sprintf(vifname, "vif%d", current->pid);
	if ((dev = dev_get_by_name(vifname)) == NULL)
	{
	    if ((dev = (struct net_device *)kmalloc(sizeof(struct net_device),
						    GFP_KERNEL)) == NULL)
	    {
		return -ENOMEM;
	    }
	    memset(dev, 0, sizeof(struct net_device));

	    /*
	     * Pass in debugging level via "irq".
	     */
#ifdef DEBUG
	    dev->irq = debug;
#endif
	    dev->init = vnatvif_probe;
	    memcpy(dev->name, vifname, IFNAMSIZ);
	    if ((err = register_netdev(dev)) < 0)
	    {
		kfree(dev);
		return err;
	    }
	    PRINTK4(debug, ("[%s:%d|vnat_ifconfig]: registered %s[%d]\n", \
			    current->comm, current->pid, dev->name, \
			    vnatvif_get_refcnt(dev)));

	    rtnl_lock();
	    err = vnat_inet_set_ifa(dev, map->l_kern_addr);
	    rtnl_unlock();

	    /*
	     * Failed to bring up the VIF.
	     */
	    if (err < 0)
	    {
		PRINTK4(debug, ("%s:%d|vnat_ifconfig]: failed to create %s\n", \
				current->comm, current->pid, dev->name));
		unregister_netdev(dev);
		kfree(dev->priv);
		kfree(dev);
		return err;
	    }
	}
	else
	{
	    /*
	     * dev_get_by_name does dev_hold automatically. Put it back.
	     */
	    dev_put(dev);
	}

	/*
	 * Bind the map entry and increment VIF reference counter.
	 */
	map->vif = dev;
	vnatvif_inc_refcnt(dev);

	PRINTK4(debug, ("[%s:%d|vnat_ifconfig]: opened %s[%d]=%d.%d.%d.%d\n", \
			current->comm, current->pid, dev->name, \
			vnatvif_get_refcnt(dev), NIPQUAD(map->l_kern_addr)));

	break;

    case DN:
	/*
	 * Map entry not bound to a VIF.
	 */
	if (!map->vif) return -EFAULT;

	/*
	 * vnatvif_dec_and_test_refcnt returns TRUE if the counter is 0.
	 */
	dev = map->vif;
	PRINTK4(debug, ("[%s:%d|vnat_ifconfig]: closing %s[%d]=%d.%d.%d.%d\n", \
			current->comm, current->pid, dev->name, \
			vnatvif_get_refcnt(dev), NIPQUAD(map->l_kern_addr)));
	if (vnatvif_dec_and_test_refcnt(dev))
	{
	    PRINTK4(debug, ("[%s:%d|vnat_ifconfig]: unregistering %s[%d]\n", \
			    current->comm, current->pid, \
			    dev->name, vnatvif_get_refcnt(dev)));
	    unregister_netdev(dev);
	    kfree(dev->priv);
	    kfree(dev);
	}
	map->vif = NULL;

	break;

    default:
	return -EINVAL;
    }

    return 0;
}

static int
vnat_inet_set_ifa(struct net_device *dev, u32 s_addr)
{
    struct in_device *in_dev = __in_dev_get(dev);
    struct in_ifaddr *ifa;

    /*
     * Later changing the address of a virtual interface is prohibited.
     */
    if (in_dev != NULL) return -EACCES;

    if ((ifa = kmalloc(sizeof(*ifa), GFP_KERNEL)) == NULL) return -ENOBUFS;
    memset(ifa, 0, sizeof(*ifa));

    if ((in_dev = kmalloc(sizeof(*in_dev), GFP_KERNEL))	== NULL)
    {
	kfree(ifa);
	return -ENOBUFS;
    }
    memset(in_dev, 0, sizeof(*in_dev));

    /*
     * Short cut taken from devinet_ioctl(), case SIOCSIFADDR.
     */
    memcpy(ifa->ifa_label, dev->name, IFNAMSIZ);
    ifa->ifa_address = ifa->ifa_local = s_addr;
    ifa->ifa_prefixlen = 32;
    ifa->ifa_mask = inet_make_mask(32);

    /*
     * Short cut taken from inetdev_init().
     */
    in_dev->lock = RW_LOCK_UNLOCKED;
    in_dev->dev = dev;
    dev_hold(dev);

    write_lock_bh(&inetdev_lock);
    dev->ip_ptr = in_dev;
    in_dev_hold(in_dev);
    write_unlock_bh(&inetdev_lock);

    in_dev_hold(in_dev);
    ifa->ifa_dev = in_dev;

    /*
     * Short cut taken from inet_insert_ifa().
     */
    write_lock_bh(&in_dev->lock);
    in_dev->ifa_list = ifa;
    write_unlock_bh(&in_dev->lock);

    /*
     * Short cut taken from devinet_ioctl(), case SIOCSIFFLAGS.
     */
    dev_open(dev);

    return 0;
}

int
vnat_iptables(vnat_map_t *map, int dir, int act)
{
    char iptables_path[] = "/sbin/iptables";
    char action[STRNMSIZ], chain[STRNMSIZ], proto[STRNMSIZ], target[STRNMSIZ];
    char src[IPADRSIZ], dst[IPADRSIZ], sport[PTNUMSIZ], dport[PTNUMSIZ];
    char to[IPADRSIZ+PTNUMSIZ];
    char *argv_nat[] = { iptables_path,
			 "-t", "nat", action, chain,
			 "-p", proto,
			 "--src", src, "--sport", sport,
			 "--dst", dst, "--dport", dport,
			 "-j", target, "--to", to, NULL };
    char *envp[] = { "HOME=/", "TERM=linux",
		     "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };

    int   err = 0;

    /*
     * Now we have all the information we need to setup the NAT.
     */
    sprintf(action, act == ADD ? "-A" : "-D");
    sprintf(proto, map->sock->type == SOCK_STREAM ? "tcp" : "udp");

    /*
     * Setup DNAT rule. Do NAT only when necessary.
     * We never setup rules when both ends of a connection are the same.
     */
    if (map->parm.trans ||
	((map->l_vnat_addr != map->r_vnat_addr) &&
	 ((dir == INCOMING && map->l_vnat_addr != map->l_kern_addr) ||
	  (dir == OUTGOING && map->r_kern_addr != map->r_vnat_addr))))
    {
	sprintf(chain, dir==INCOMING ? "PREROUTING" : "OUTPUT");

	sprintf(target, "DNAT");

	sprintf(src, "%d.%d.%d.%d",
		NIPQUAD((dir==INCOMING ? map->r_vnat_addr:map->l_kern_addr)));

	sprintf(dst, "%d.%d.%d.%d",
		NIPQUAD((dir==INCOMING ? map->l_vnat_addr:map->r_kern_addr)));

	sprintf(to, "%d.%d.%d.%d:%d",
		NIPQUAD((dir==INCOMING ? map->l_kern_addr:map->r_vnat_addr)),
		ntohs((dir==INCOMING ? map->l_kern_port:map->r_vnat_port)));

	sprintf(sport, "%d",
		ntohs((dir==INCOMING ? map->r_vnat_port:map->l_kern_port)));

	sprintf(dport, "%d",
		ntohs((dir==INCOMING ? map->l_vnat_port:map->r_kern_port)));

	err = call_usermodehelper2(iptables_path, argv_nat, envp, &umhc);

	PRINTK4(debug, ("[%s:%d|vnat_iptables]:", \
			current->comm, current->pid));
	VEC_PRINTK4(argv_nat[i]);
	PRINTK4(debug, (" ... errno=%d\n", err));

	if (err < 0) return err;
    }

    /*
     * Setup SNAT rule.
     * We never setup rules when both ends of a connection are the same.
     */
    if (map->parm.trans ||
	((map->l_vnat_addr != map->r_vnat_addr) &&
	 ((dir == INCOMING && map->r_vnat_addr != map->r_kern_addr) ||
	  (dir == OUTGOING && map->l_kern_addr != map->l_vnat_addr))))
    {
	sprintf(chain, dir==INCOMING ? "INPUT" : "POSTROUTING");
	sprintf(target, "SNAT");


	sprintf(src, "%d.%d.%d.%d",
		NIPQUAD((dir==INCOMING ? map->r_vnat_addr:map->l_kern_addr)));

	sprintf(dst, "%d.%d.%d.%d",
		NIPQUAD((dir==INCOMING ? map->l_kern_addr:map->r_vnat_addr)));

	sprintf(to, "%d.%d.%d.%d:%d",
		NIPQUAD((dir==INCOMING ? map->r_kern_addr:map->l_vnat_addr)),
		ntohs((dir==INCOMING ? map->r_kern_port:map->l_vnat_port)));

	sprintf(sport, "%d",
		ntohs((dir==INCOMING ? map->r_vnat_port:map->l_kern_port)));

	sprintf(dport, "%d",
		ntohs((dir==INCOMING ? map->l_kern_port:map->r_vnat_port)));

	err = call_usermodehelper2(iptables_path, argv_nat, envp, &umhc);

	PRINTK4(debug, ("[%s:%d|vnat_iptables]:", \
			current->comm, current->pid));
	VEC_PRINTK4(argv_nat[i]);
	PRINTK4(debug, (" ... errno=%d\n", err));

	if (err < 0)
	{
	    /*
	     * FIXME: need to clean up the DNAT rule.
	     */
	    return err;
	}
    }

    /*
     * Note that even though initially the virtual connection and the
     * physical connection are the _same_ so there are _no_ rules created
     * for initial translation, connection tracking are still created by
     * netfilter.
     * As a result, for the first migration, even though there will be no
     * old rules to delete, we still have to delete the connection tracking.
     */
    if (act == DEL) vnat_conntrack_del(map);

    return err;
}

static int
vnat_conntrack_del(vnat_map_t *map)
{
    struct ip_conntrack_tuple_hash *h;
    struct ip_conntrack_tuple tuple;

    tuple.src.ip = map->l_kern_addr;
    tuple.dst.ip = map->r_kern_addr;

    switch(map->sock->type)
    {
    case SOCK_STREAM:
	tuple.dst.protonum = IPPROTO_TCP;
	tuple.src.u.tcp.port = map->l_kern_port;
	tuple.dst.u.tcp.port = map->r_kern_port;
	break;

    case SOCK_DGRAM:
	tuple.dst.protonum = IPPROTO_UDP;
	tuple.src.u.udp.port = map->l_kern_port;
	tuple.dst.u.udp.port = map->r_kern_port;
	break;

    default:
	tuple.dst.protonum = IPPROTO_IP;
	tuple.src.u.all = 0;
	tuple.dst.u.all = 0;
	break;
    }

    if ((h = ip_conntrack_find_get(&tuple, NULL)))
    {
	PRINTK4(debug, ("[%s:%d|vnat_conntrack]: [%d.%d.%d.%d:%d, \
%d.%d.%d.%d:%d, %d]=%d\n", current->comm, current->pid, \
			NIPQUAD(map->l_kern_addr), ntohs(map->l_kern_port), \
			NIPQUAD(map->r_kern_addr), ntohs(map->r_kern_port), \
			tuple.dst.protonum, \
			atomic_read(&h->ctrack->ct_general.use)));

	/*
	 * Drop the counter grabbed by "ip_conntrack_find_get".
	 */
	atomic_dec(&h->ctrack->ct_general.use);

	/*
	 * Take it off the hash table. This will also destroy the conntrack
	 * tuple entry if "ct_general.use" drops to 0.
	 */
	if (del_timer(&h->ctrack->timeout))
	{
	    h->ctrack->timeout.function((unsigned long)h->ctrack);
	}
    }

    return 0;
}
