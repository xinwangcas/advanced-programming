/*
 * vnatvif.c
 *
 * VNAT Kernel virtual ethernet device driver
 *
 * Copyright (c) 2000-2002 Gong Su, All rights reserved.
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

#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/types.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <linux/if_packet.h>

#include "misc.h"

static int vnatvif_open(struct net_device *dev);
static int vnatvif_start_xmit(struct sk_buff *skb, struct net_device *dev);
static int vnatvif_close(struct net_device *dev);
static struct net_device_stats *vnatvif_get_stats(struct net_device *dev);

struct vnatvif_priv
{
    /*
     * The "refcnt" in "net_device" structure is used by routing code.
     */
    /*__u32  refcnt;*/
    atomic_t refcnt;

    struct net_device_stats stats;
};

#ifdef DEBUG
static int debug = 0;
#endif

int
vnatvif_get_refcnt(struct net_device *dev)
{
    return atomic_read(&((struct vnatvif_priv *)dev->priv)->refcnt);
}

int
vnatvif_dec_and_test_refcnt(struct net_device *dev)
{
    return atomic_dec_and_test(&((struct vnatvif_priv *)dev->priv)->refcnt);
}

void
vnatvif_inc_refcnt(struct net_device *dev)
{
    atomic_inc(&((struct vnatvif_priv *)dev->priv)->refcnt);
}

void
vnatvif_dec_refcnt(struct net_device *dev)
{
    atomic_dec(&((struct vnatvif_priv *)dev->priv)->refcnt);
}

int
vnatvif_probe(struct net_device *dev)
{
    /*
     * Fetch debugging level from "irq" and reset "irq" to 0.
     * This way we don't have to use a global variable so we will be
     * self-contained.
     */
#ifdef DEBUG
    debug = dev->irq;
    dev->irq = 0;
#endif

    PRINTK4(debug, ("[%s:%d|vnatvif_probe]: { %s }\n", \
		    current->comm, current->pid, dev->name));

    memcpy(dev->dev_addr, "\xFE\xFE\x00\x00\x00\x00", 6);

    /*
     * Initialize the device structure.
     */
    if ((dev->priv = kmalloc(sizeof(struct vnatvif_priv), GFP_KERNEL))
	== NULL) return -ENOMEM;
    memset(dev->priv, 0, sizeof(struct vnatvif_priv));

    /*
     * Setup the generic properties
     */
    ether_setup(dev);

    /*
     * The rdr specific entries in the device structure.
     */
    dev->open = vnatvif_open;
    dev->hard_start_xmit = vnatvif_start_xmit;
    dev->stop = vnatvif_close;
    dev->get_stats = vnatvif_get_stats;

    dev->flags |= (IFF_NOARP | IFF_POINTOPOINT);
    dev->tx_queue_len = 0;

    return 0;
}

static int
vnatvif_open(struct net_device *dev)
{
    PRINTK4(debug, ("[%s:%d|vnatvif_open:]: { %s }\n", \
		    current->comm, current->pid, dev->name));

#if (LINUX_VERSION_CODE >= 0x02032a)
    /*netif_start_queue(dev);*/
#else
    /*
    dev->interrupt = 0;
    dev->tbusy = 0;
    dev->start = 1;
    */
#endif

    MOD_INC_USE_COUNT;
    return 0;
}

static int
vnatvif_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    /*
    struct vnatvif_priv *kvnatif = (struct vnatvif_priv *)dev->priv;
    struct net_device_stats *stats = &kvnatif->stats;
    */

    /*
     * This route should have NEVER been called to acctually send out
     * packets. When a process tries to send out packets via a socket
     * that is bound to us locally and (presumably) to another VIF
     * remotely, the NF_IP_LOCAL_OUT hook should have already mangled
     * the destination address appropriately so the packets are routed
     * to the correct PIF.
     */

    PRINTK4(debug, ("[%s:%d|vnatvif_start_xmit]: \
hmm...%s dropping packet...\n", current->comm, current->pid, dev->name));

    dev_kfree_skb(skb);
    return 0;
}

static int
vnatvif_close(struct net_device *dev)
{
    PRINTK4(debug, ("[%s:%d|vnatvif_close]: { %s }\n", \
		    current->comm, current->pid, dev->name));

#if (LINUX_VERSION_CODE >= 0x02032a)
    /*netif_stop_queue(dev);*/
#else
    /*
    dev->tbusy = 1;
    dev->start = 0;
    */
#endif

    MOD_DEC_USE_COUNT;
    return 0;
}

static struct net_device_stats *
vnatvif_get_stats(struct net_device *dev)
{
    return &((struct vnatvif_priv *)dev->priv)->stats;
}

MODULE_DESCRIPTION("Kernel Virtual Interface (VIF) driver");
MODULE_AUTHOR("Gong Su, (C) 2001, GPLv2 or later");
#if (LINUX_VERSION_CODE >= 0x02040e)
MODULE_LICENSE("GPL");
#endif

int __init init_vnatvif(void)
{
    return 0;
}

void __exit exit_vnatvif(void)
{
    return;
}

module_init(init_vnatvif);
module_exit(exit_vnatvif);

EXPORT_SYMBOL(vnatvif_probe);
EXPORT_SYMBOL(vnatvif_get_refcnt);
EXPORT_SYMBOL(vnatvif_dec_and_test_refcnt);
EXPORT_SYMBOL(vnatvif_inc_refcnt);
EXPORT_SYMBOL(vnatvif_dec_refcnt);
