/*
 * umodecall.c
 *
 * Call a userspace program from within the kernel.
 *
 * This code is originally written by Andrew Morton <andrewm@uow.edu.au>.
 * Modified to work with VNAT.
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

#include <linux/sched.h>
#include <linux/unistd.h>
#include <linux/smp_lock.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/interrupt.h>

#include <asm/semaphore.h>
#include <asm/uaccess.h>

#include "umodecall.h"

/*
 * Hack. do_exit is not exported. Use complete_and_exit (>=2.4.10)
 * or up_and_exit (<2.4.10).
 */
#if (LINUX_VERSION_CODE >= 0x02040a)
#define DO_EXIT(x) \
{ \
      complete_and_exit(NULL, x); \
}
#else
#define DO_EXIT(x) \
{ \
      DECLARE_MUTEX_LOCKED(sem); \
      up_and_exit(&sem, x); \
}
#endif

struct subprocess_info
{
    struct tq_struct task_queue;
    char *path;
    char **argv;
    char **envp;
    struct semaphore *sem;  /* If this is non-zero we must wake the parent */
    struct umh_control umh_c;
    int *exit_code;
};

struct umh_control umhc = { 1 };

extern void *kmallocz(size_t size, int gfp_flags);
extern char *kstrdup(const char *orig, int gfp_flags);
extern char **kstrdup_vec(char **orig, int gfp_flags);
extern void kstrfree_vec(char **vec);

static int errno;

/*
 * kmallocz - allocate some memory with kmalloc and zero it
 * @size: number of bytes to allocate
 * @gfp_flags: allocation mode for kmalloc()
 *
 * Returns a pointer to the newly allocated and cleared memory, or %NULL
 * if the allocation failed. The memory may be released with kfree().
 */
void *kmallocz(size_t size, int gfp_flags)
{
    void *ret = kmalloc(size, gfp_flags);
    if (ret)
	memset(ret, 0, size);
    return ret;
}
EXPORT_SYMBOL(kmallocz);

/*
 * kstrdup - duplicate a string in kmalloced memory
 * @orig: the string to duplicate
 * @gfp_flags: allocation mode for kmalloc()
 *
 * Returns the copy, or %NULL on allocation failure. The new string
 * may be released with kfree().
 */
char *kstrdup(const char *orig, int gfp_flags)
{
    char *ret = kmalloc(strlen(orig) + 1, gfp_flags);
    if (ret)
	strcpy(ret, orig);
    return ret;
}
EXPORT_SYMBOL(kstrdup);

/*
 * kstrdup_vec - copy a vector of strings
 * @orig: null-terminated array of pointers to strings
 * @gfp_flags: allocation mode for kmalloc()
 *
 * Returns a duplicated copy of the passed vector of strings. Returns %NULL
 * on allocation error. The returned vector is null-terminated and may be
 * released with kstrfree_vec().
 */
char **kstrdup_vec(char **orig, int gfp_flags)
{
    unsigned int nstrings, idx;

    char **ret;

    for (nstrings = 0; orig[nstrings] != 0; nstrings++)
	;

    ret = kmallocz((nstrings + 1) * sizeof(*ret), gfp_flags);
    if (ret)
    {
	for (idx = 0; idx < nstrings; idx++)
	{
	    ret[idx] = kstrdup(orig[idx], gfp_flags);
	    if (ret[idx] == 0)
	    {
		kstrfree_vec(ret);
		ret = 0;
		goto out;
	    }
	}
    }

 out:
    return ret;
}
EXPORT_SYMBOL(kstrdup_vec);


/*
 * kstrfree_vec - release a vector of strings which was allocated with
 * kstrdup_vec()
 * @vec: address of the null-terminated vector
 *
 * Releases the strings via kfree() and then releases the containing vector
 * itself.
 * If @vec is %NULL no action is taken.
 */
void kstrfree_vec(char **vec)
{
    unsigned int idx = 0;

    if (vec)
    {
	while (vec[idx])
	     kfree(vec[idx++]);
	kfree(vec);
    }
}
EXPORT_SYMBOL(kstrfree_vec);

/*
 * Notify userspace, unblock the caller of call_usermodehelper
 * and clean up.
 */
static void free_subprocess_info(struct subprocess_info *sub_info,
				 int exit_code)
{

    if (sub_info->umh_c.completion)
    {
	sub_info->umh_c.completion(sub_info->umh_c.completion_arg, exit_code);

	if (sub_info->umh_c.owner)
	    __MOD_DEC_USE_COUNT(sub_info->umh_c.owner);
    }

    if (sub_info->sem)
    {
	*sub_info->exit_code = exit_code;
	up(sub_info->sem);
    }

    kstrfree_vec(sub_info->envp);
    kstrfree_vec(sub_info->argv);
    kfree(sub_info->path);
    kfree(sub_info);
}

/*
 * This thread becomes the usermode application.
 */
static int ______call_usermodehelper(void *data)
{
    struct subprocess_info *sub_info = data;
    int retval = -EPERM;

    if (current->fs->root)
	retval = exec_usermodehelper(sub_info->path,
				     sub_info->argv, sub_info->envp);

    DO_EXIT(retval);
}

/*
 * This thread parents the usermode application.
 */
static int ____call_usermodehelper(void *data)
{
    struct subprocess_info *sub_info = data;
    struct task_struct *curtask = current;
    pid_t pid, pid2;
    int ret = 0;

    /*
     * CLONE_VFORK: wait until the usermode helper has execve'd successfully.
     * We need the data structures to stay around until that is done.
     * We really only need to do this for the asynchronous case.
     */
    if ((pid = kernel_thread(______call_usermodehelper, sub_info,
			     CLONE_VFORK | SIGCHLD)) < 0)
    {
	printk(KERN_ERR "call_usermodehelper(%s): vfork failed: %d\n",
	       sub_info->path, pid);
	ret = pid;
	goto out;
    }

    if (sub_info->sem || sub_info->umh_c.completion)
    {
	mm_segment_t fs = get_fs();

	/*
	 * Block everything but SIGKILL/SIGSTOP
	 */
	spin_lock_irq(&curtask->sigmask_lock);
	siginitsetinv(&curtask->blocked, sigmask(SIGKILL) | sigmask(SIGSTOP));
	recalc_sigpending(curtask);
	spin_unlock_irq(&curtask->sigmask_lock);
	set_fs(KERNEL_DS);
	pid2 = waitpid(pid, &ret, __WALL);
	set_fs(fs);
	if (pid2 != pid)
	{
	    printk(KERN_ERR "call_usermodehelper(%s): waitpid failed: %d\n",
		   sub_info->path, pid2);
	    ret = (pid2 < 0) ? -errno : -1;
	}
    }

 out:
    free_subprocess_info(sub_info, ret);
    DO_EXIT(0);
}

/*
 * This is run by keventd.
 */
static void __call_usermodehelper(void *data)
{
    struct subprocess_info *sub_info = data;
    pid_t pid;

    if ((pid = kernel_thread(____call_usermodehelper, sub_info, SIGCHLD)) < 0)
    {
	printk(KERN_ERR "call_usermodehelper(%s): fork failed: %d\n",
	       sub_info->path, pid);
	free_subprocess_info(sub_info, pid);
    }
}

/*
 * call_usermodehelper - start a usermode application
 * @path: pathname for the application
 * @argv: null-terminated argument list
 * @envp: null-terminated environment list
 * @umh_c: an &umh_control which controls call_usermodehelper()'s behaviour
 *
 * Runs a user-space application. The application runs as a child of keventd.
 * It runs with full root capabilities. If @umh_c->synchronous is true,
 * call_usermodehelper() must be called from process context, and it will
 * block the caller until the usermode application has exitted, or an error
 * has occurred.
 *
 * If &umh_c is %NULL then the usermode helper is called asynchronously and
 * no completion is performed.
 *
 * If @umh_c->synchronous is true and the usermode application was
 * successfully launched, call_usermodehelper() returns the non-negative
 * usermode application's exit code.
 *
 * If @umh_c->synchronous is true and the usermode application was not
 * successfully launched, call_usermodehelper() returns a negative error code.
 *
 * If @umh_c->synchronous is false, and call_usermodehelper() returns a
 * negative error code then the usermode application could not be launched.
 *
 * If @umh_c->synchronous is false and call_usermodehelper() returns zero
 * then the usermode application _may_ have been launched. It is not possible
 * for the caller to know the outcome until the completion routine (if
 * requested) is called.
 *
 * If @umh_c->completion is non-zero, @umh_c->synchronous is false and if
 * call_usermodehelper() returned zero, the completion routine will be called
 * at some time in the future, within the context of keventd or a child of
 * keventd. The completion routine is passed @umh_c->completion_arg and
 * `exit_code'. If `exit_code' is negative, the usermode application was never
 * launched. If `exit_code' is non-negative then the usermode application did
 * run and this is its exitcode. If `exit_code' is non-negative then the
 * completion routine may sleep.
 *
 * If @umh_c->synchronous is true then a completion function may not be used.
 *
 * If call_usermodehelper() was called from within a module then it raises
 * your module reference count while the completion is pending, provided you
 * correctly used the helper macros in kmod.h
 */
int call_usermodehelper2(char *path, char **argv, char **envp,
			 struct umh_control *umh_c)
{
    struct subprocess_info *sub_info;

    DECLARE_MUTEX_LOCKED(sem);

    int ret = -ENOMEM;
    int gfp_flags = in_interrupt() ? GFP_ATOMIC : GFP_KERNEL;

    if ((sub_info = kmallocz(sizeof(*sub_info), gfp_flags)) == 0) goto fail;

    if (umh_c)
    {
	sub_info->umh_c = *umh_c;
	if (umh_c->synchronous)
	{
	    sub_info->exit_code = &ret;
	    sub_info->sem = &sem;
	    sub_info->umh_c.completion = 0; /* No completion allowed */
	}
	if (sub_info->umh_c.owner &&
	    sub_info->umh_c.completion &&
	    try_inc_mod_count(sub_info->umh_c.owner) == 0) goto fail;
    }

    if ((sub_info->path = kstrdup(path, gfp_flags)) == 0) goto fail;
    if ((sub_info->argv = kstrdup_vec(argv, gfp_flags)) == 0) goto fail;
    if ((sub_info->envp = kstrdup_vec(envp, gfp_flags)) == 0) goto fail;

    sub_info->task_queue.routine = __call_usermodehelper;
    sub_info->task_queue.data = sub_info;

    if (schedule_task(&sub_info->task_queue) == 0) BUG();

    /*
     * We can't touch *sub_info from now on.
     */
    ret = 0;
    if (umh_c && umh_c->synchronous) down(&sem);
    smp_mb();
    return ret;

 fail:
    sub_info->umh_c.completion = 0;
    /*
     * It could have been set, above.
     */
    free_subprocess_info(sub_info, ret);
    return ret;
}
EXPORT_SYMBOL(call_usermodehelper2);
