/*
 * kvnatd.h
 *
 * Data structure for kvnatd.
 *
 * Copyright (c) 2001-2002 Gong Su, All rights reserved.
 *
 * See the file "COPYRIGHT" for information on usage and redistribution
 * of this file.
 */

#ifndef __KVNATD_H__
#define __KVNATD_H__

#include <linux/tqueue.h>  /* struct tq_struct */
#include <asm/semaphore.h> /* struct semaphore */

/*
 * Port on which kvnatd listens.
 */
#define KVNATD_PORT 2031

struct kthr_data
{
    struct task_struct *thr;
    struct tq_struct    ktq;
    struct semaphore    sem;
    /*
    wait_queue_head_t   que;
    */

    void              (*func)(struct kthr_data *data);
    int                 bye;
};
typedef struct kthr_data kthr_data_t;

extern void kvnatd(kthr_data_t *data);

extern void start_kvnatd(void (*func)(kthr_data_t *data), kthr_data_t *data);
extern void stop_kvnatd (kthr_data_t *data);

#endif
