/*
 * misc.h
 *
 * Export support routines used by VNAT system.
 *
 * Copyright (c) 2001-2002 Gong Su, All rights reserved.
 *
 * See the file "COPYRIGHT" for information on usage and redistribution
 * of this file.
 */
#ifndef __MISC_H__
#define __MISC_H__

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifdef DEBUG

#define PRINTK1(lvl, fmt) \
do { \
    if (lvl & 1) printk fmt; \
} while(0)

#define PRINTK2(lvl, fmt) \
do { \
    if (lvl & 2) printk fmt; \
} while(0)

#define PRINTK4(lvl, fmt) \
do { \
    if (lvl & 4) printk fmt; \
} while(0)

/*
 * "vec" must be in the form of foo[i].
 */
#define VEC_PRINTK4(fmt) \
do { \
    int i = 0; \
    while (fmt) \
    { \
	PRINTK4(debug, (" %s", fmt)); \
	i++; \
    } \
} while (0)

#else

#define PRINTK1(lvl, fmt) do {} while(0)
#define PRINTK2(lvl, fmt) do {} while(0)
#define PRINTK4(lvl, fmt) do {} while(0)
#define VEC_PRINTK4(fmt)  do {} while(0)

#endif

extern void  list_free(void **chain);
extern void *list_find(void **chain, void *item,
		       int (*cmp)(void *p1, void *p2));
extern void *list_insert(void **chain, void *item,
			 int (*cmp)(void *p1, void *p2));
extern void *list_delete(void **chain, void *item,
			 int (*cmp)(void *p1, void *p2));

extern struct task_struct *sock_owner(struct socket *sock);

#endif
