/*
 * vnatvif.h
 *
 * VNAT virtual ethernet device driver access functions.
 *
 * Copyright (c) 2001-2002 Gong Su, All rights reserved.
 *
 * See the file "COPYRIGHT" for information on usage and redistribution
 * of this file.
 */

#ifndef __VNATVIF_H__
#define __VNATVIF_H__

#include <linux/netdevice.h> /* struct net_device */

extern int  vnatvif_probe(struct net_device *dev);
extern int  vnatvif_get_refcnt(struct net_device *dev);
extern int  vnatvif_dec_and_test_refcnt(struct net_device *dev);
extern void vnatvif_inc_refcnt(struct net_device *dev);
extern void vnatvif_dec_refcnt(struct net_device *dev);

#endif
