/*
 * vnatd.c
 *
 * VNAT kernel daemon.
 *
 * Copyright (c) 2001-2002 Gong Su, All rights reserved.
 *
 * See the file "COPYRIGHT" for information on usage and redistribution
 * of this file.
 */

/*!
 * \defgroup kvnatd kvnatd
 *
 * \brief VNAT daemon
 *
 * \ingroup kvnatd
 *
 * VNAT daemon (kvnatd) is a kernel daemon process that implements the
 * VNAT Connection Migration Protocol (VCMP) for negotiating various
 * options (e.g., suspension time, helper, security, etc.) at suspending
 * time and for updating virtual-physical address mapping at resuming
 * time.
 *
 * Kvnatd is not started from a command line. It is created when the VNAT
 * module is loaded and killed when the VNAT module is unloaded.
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

#include <linux/signal.h>  /* sigset_t                        */
#include <linux/fcntl.h>   /* O_RDWR (must be after signal.h  */
#include <linux/net.h>     /* struct socket, net_proto_family */
#include <linux/in.h>      /* struct sockaddr_in              */
#include <linux/sched.h>   /* "current" macro                 */
#include <linux/smp_lock.h>
#include <linux/unistd.h>

#include <net/sock.h>      /* struct sock                     */

#include "vnat.h"
#include "vnatd.h"
#include "vnatmap.h"
#include "vnatvcmp.h"
#include "misc.h"

static void kvnatd_handle(struct socket *sock);
static void kvnatd_suspend(struct kthr_data *kvnatd_data);
static void kvnatd_resume(struct kthr_data *kvnatd_data);

static void
kvnatd_launcher(void *data)
{
    kthr_data_t *kthr = data;

    /*
     * Set the SIGCHLD flag so when the kernel thread exits, a SIGCHLD
     * signal will be sent to the parent (keventd in this case).
     */
    kernel_thread((int (*)(void *))kthr->func, (void *)kthr, SIGCHLD);
}

static void
init_kvnatd(kthr_data_t *data, char *name)
{
    /*
     * Lock the kernel. A new kernel thread starts without the big
     * kernel lock, regardless of the lock state of the creator
     * (the lock level is *not* inheritated).
     */
    lock_kernel();

    /*
     * Fill in thread structure.
     */
    data->thr = current;

    /*
     * Set signal mask to what we want to respond.
     */
    siginitsetinv(&current->blocked,
		  sigmask(SIGKILL)|
		  sigmask(SIGHUP)|
		  sigmask(SIGTSTP)|
		  sigmask(SIGCHLD));

    /* initialise wait queue */
    /*init_waitqueue_head(&data->que);*/

    /*
     * Initialise termination flag.
     */
    data->bye = 0;

    /*
     * Set name of this thread (max 15 chars + 0 !).
     */
    sprintf(current->comm, name);

    /*
     * Let others run.
     */
    unlock_kernel();

    /*
     * Tell the creator that we are ready and let him continue.
     */
    up(&data->sem);
}

static void
exit_kvnatd(struct kthr_data *data)
{
    /*
     * Lock the kernel, the exit will unlock it.
     */
    lock_kernel();
    data->thr = NULL;
    mb();

    /*
     * Notify the stop_kvnatd() routine that we are terminating.
     */
    up(&data->sem);

    /*
     * The kernel_thread that called clone() does a do_exit here.
     */

    /*
     * There is no race here between execution of the "killer" and
     * real termination of the thread (race window between up and
     * do_exit), since both the thread and the "killer" function are
     * running with the kernel lock held. The kernel lock will be
     * freed after the thread exited, so the code is really not
     * executed anymore as soon as the unload functions gets the
     * kernel lock back. The init process may not have made the
     * cleanup of the process here, but the cleanup can be done
     * safely with the module unloaded.
    */
}

void
start_kvnatd(void (*func)(kthr_data_t *data), kthr_data_t *data)
{
    PRINTK4(debug, ("[%s:%d|start_kvnatd]: { ...\n", \
		    current->comm, current->pid));

    init_MUTEX_LOCKED(&data->sem);

    /* store the function to be executed in the data passed to
       the launcher */
    data->func = func;

    /*
     * Create the new thread by running a task through keventd.
     */
    data->ktq.sync = 0;
    INIT_LIST_HEAD(&data->ktq.list);
    data->ktq.routine =  kvnatd_launcher;
    data->ktq.data = data;

    schedule_task(&data->ktq);

    /*
     * Wait for the thread to come up.
     */
    down(&data->sem);

    PRINTK4(debug, ("[%s:%d|start_kvnatd]: ... }\n", \
		    current->comm, current->pid));
}

void
stop_kvnatd(kthr_data_t *data)
{
#ifdef DEBUG
    char *comm = current->comm;
    pid_t pid = current->pid;
#endif

    if (data->thr == NULL)
    {
	PRINTK4(debug, ("[%s:%d|stop_kvnatd]: \
kernel thread already terminated\n", comm, pid));
	return;
    }

    PRINTK4(debug, ("[%s:%d|stop_kvnatd]: { ...\n", comm, pid));

    /*
     * This function needs to be protected with the big kernel lock
     * (lock_kernel()). The lock must be grabbed before changing the
     * terminate flag and released after the down() call.
     */
    lock_kernel();

    /*
     * Initialize the semaphore. We lock it here, the exit_kvnatd call
     * of the thread to be terminated will unlock it. As soon as we see
     * the semaphore unlocked, we know that the thread has exited.
     */
    init_MUTEX_LOCKED(&data->sem);

    /*
     * We need to do a memory barrier here to be sure that
     * the flags are visible on all CPUs.
     */
    mb();

    /*
     * Set flag to request thread termination.
     */
    data->bye = 1;

    /*
     * We need to do a memory barrier here to be sure that
     * the flags are visible on all CPUs.
     */
    mb();
    kill_proc(data->thr->pid, SIGKILL, 1);

    /*
     * Block till thread terminated.
     */
    down(&data->sem);

    /*
     * Release the big kernel lock.
     */
    unlock_kernel();

    /*
     * Now we are sure the thread is in zombie state. We notify keventd
     * to clean the process up.
     *
     * FIXME: this assumes that keventd is running on pid 2.
     * FIXED: set SIGCHLD flag when calling kernel_thread.
     */
    /*kill_proc(2, SIGCHLD, 1);*/

    PRINTK4(debug, ("[%s:%d|stop_kvnatd]: ... }\n", comm, pid));
}

void
kvnatd(kthr_data_t *kvnatd_data)
{
    struct socket *sock, *sock2;
    struct sockaddr_in sin;
    int err = 0;

    if ((err = sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &sock)) < 0)
	goto _exit0;

    sin.sin_family      = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port        = htons(KVNATD_PORT);

    if ((err = sock->ops->bind(sock, (struct sockaddr*)&sin, sizeof(sin))) < 0)
	goto _exit1;

    sock->sk->reuse = 1;

    if ((err = sock->ops->listen(sock, 5)) < 0) goto _exit1;

    /*
     * Note that we delay init_kvnatd until after we created the protocol
     * socket. This is to make sure that the protocol socket creation goes
     * through the normal socket calls rather than the intercepted ones
     * (once we call init_kvnatd, the interceptor will go ahead patching
     * the socket calls).
     */
    init_kvnatd(kvnatd_data, "kvnatd");

    for(;;)
    {
	if ((sock2 = sock_alloc()) == NULL)
	{
	    /*
	     * Wait for a while and try again.
	     */
	    static wait_queue_head_t wait_queue;

	    init_waitqueue_head(&wait_queue);
	    interruptible_sleep_on_timeout(&wait_queue, (5 * HZ) >> 1);

	    continue;
	}

	sock2->type = sock->type;
	sock2->ops  = sock->ops;

    _restart:
	if ((err = sock->ops->accept(sock, sock2, O_RDWR)) < 0)
	{
	    PRINTK4(debug, ("[%s:%d|kvnatd]: \
?sock->ops->accept[%p, %p, %d]=%d\n", current->comm, current->pid, \
			    sock, sock2, O_RDWR, err));
	}
	else
	{
	    PRINTK4(debug, ("[%s:%d|kvnatd]: \
!sock->ops->accept[%p, %p, %d]=%d\n", current->comm, current->pid, \
			    sock, sock2, O_RDWR, err));
	}

	mb();

	/*
	 * If the kvnat module is being unloaded, it will set this field.
	 */
	/*
	if (kvnatd_data->bye)
	{
	    PRINTK4(debug, ("[%s:%d|kvnatd]: \
terminating on SIGKILL [kvnat module unloading]\n", \
			    current->comm, current->pid));
	    sock_release(sock2);
	    break;
	}
	*/
	if (signal_pending(current))
	{
	    int kill = 0, cont = 0, stop = 0, chld = 0;

	    spin_lock_irq(&current->sigmask_lock);
	    if (sigismember(&current->pending.signal, SIGKILL))
	    {
		sigdelset(&current->pending.signal, SIGKILL);
		kill = 1;
	    }
            else if (sigismember(&current->pending.signal, SIGHUP))
            {
                sigdelset(&current->pending.signal, SIGHUP);
                cont = 1;
            }
            else if (sigismember(&current->pending.signal, SIGTSTP))
            {
                sigdelset(&current->pending.signal, SIGTSTP);
                stop = 1;
            }
            else if (sigismember(&current->pending.signal, SIGCHLD))
            {
                sigdelset(&current->pending.signal, SIGCHLD);
                chld = 1;
            }
	    recalc_sigpending(current);
	    spin_unlock_irq(&current->sigmask_lock);

	    /*
	     * We received a KILL signal, e.g., system shutdown or reboot.
	     */
	    if (kill)
	    {
		PRINTK4(debug, ("[%s:%d|kvnatd]: terminating on SIGKILL\n", \
				current->comm, current->pid));
		sock_release(sock2);
		kill_proc(2, SIGCHLD, 1);
		break;
	    }
	    /*
	     * We received a post-resuming HUP signal.
	     */
            else if (cont)
            {
                PRINTK4(debug, ("[%s:%d|kvnatd]: waking up on SIGHUP\n", \
				current->comm, current->pid));
		kvnatd_resume(kvnatd_data);
                goto _restart;
            }
	    /*
	     * We received a pre-suspending STOP signal.
	     */
            else if (stop)
            {
                PRINTK4(debug, ("[%s:%d|kvnatd]: waking up on SIGTSTP\n", \
				current->comm, current->pid));
		kvnatd_suspend(kvnatd_data);
                goto _restart;
            }
	    else if (chld)
	    {
                PRINTK4(debug, ("[%s:%d|kvnatd]: waking up on SIGCHLD\n", \
				current->comm, current->pid));
		waitpid(-1, 0, __WALL|WNOHANG);
		goto _restart;
	    }
	}

	/*
	 * Set the SIGCHLD flag so when the kernel thread exits, a SIGCHLD
	 * signal will be sent to the parent.
	 */
	kernel_thread((int (*)(void *))kvnatd_handle, (void *)sock2, SIGCHLD);
    }

 _exit1:
    sock_release(sock);

 _exit0:
    exit_kvnatd(kvnatd_data);
}

static void
kvnatd_handle(struct socket *sock)
{
    vcmp_msg_t msg;

    /*
     * Read and handle the message.
     *
     * msg.sock is needed by vcmp_pullpdu to lock and read daddr and dport.
     */
    msg.sock = sock;
    /*
    sin.sin_addr.s_addr = INADDR_ANY;
    msg.addr = (struct sockaddr *)&sin;
    */
    /*
     * Send reply if necessary.
     */
    while (vcmp_recvmsg(sock, &msg, VCMP_ALL, 0, timeo) > 0)
    {
	if (!msg.sock ||
	    vcmp_sendmsg(sock, &msg, ++msg.pdu.command,
			 MSG_NOSIGNAL, timeo) <= 0 ||
	    !msg.sock) break;
    }

    sock_release(sock);
}

static void
kvnatd_suspend(struct kthr_data *kvnatd_data)
{
    PRINTK4(debug, ("[%s:%d|kvnatd_suspend]: { ...\n", \
		    current->comm, current->pid));

    vnat_map_suspend(NULL);

    PRINTK4(debug, ("[%s:%d|kvnatd_suspend]: ... }\n", \
		    current->comm, current->pid));
}

static void
kvnatd_resume(struct kthr_data *kvnatd_data)
{
    PRINTK4(debug, ("[%s:%d|kvnatd_resume]: { ...\n", \
		    current->comm, current->pid));

    vnat_map_resume(NULL);

    PRINTK4(debug, ("[%s:%d|kvnatd_resume]: ... }\n", \
		    current->comm, current->pid));
}
