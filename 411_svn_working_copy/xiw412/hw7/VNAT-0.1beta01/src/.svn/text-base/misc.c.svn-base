/*
 * misc.c
 *
 * Various support routines used by VNAT system.
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

#include <linux/kernel.h> /* NULL          */
#include <linux/slab.h>   /* kfree         */
#include <linux/sched.h>  /* for_each_task */
#include <linux/file.h>   /* fcheck_files  */

void
list_free(void **chain)
{
    struct templ { struct templ *next; } *ptr;

    /*
     * Advance "chain" instead of "ptr" save us an explicit "*chain = NULL"
     * at the end of the operation.
     */
    while (*chain)
    {
	ptr = *chain;
	*chain = ((struct templ *)(*chain))->next;
	kfree(ptr);
    }
}

void *
list_find(void **chain, void *item, int (*cmp)(void *p1, void *p2))
{
    struct templ { struct templ *next; } **ptr = (struct templ **)chain;

    for (; *ptr != NULL; ptr = &((*ptr)->next))
    {
	if (cmp(*ptr, item)) return *ptr;
    }

    return NULL;
}

void *
list_insert(void **chain, void *item, int (*cmp)(void *p1, void *p2))
{
    struct templ { struct templ *next; } **ptr = (struct templ **)chain;

    for (; *ptr != NULL; ptr = &((*ptr)->next))
    {
	if (cmp(*ptr, item)) return *ptr;
    }

    *ptr = item;

    return NULL;
}

void *
list_delete(void **chain, void *item, int (*cmp)(void *p1, void *p2))
{
    struct templ
    {
	struct templ *next;
    } **ptr = (struct templ **)chain, *ret = NULL;

    for (; *ptr != NULL; ptr = &((*ptr)->next))
    {
        if (cmp(*ptr, item))
        {
            ret = *ptr;

            *ptr = (*ptr)->next;
            break;
        }
    }

    return ret;
}

struct task_struct *
sock_owner(struct socket *sock)
{
    struct task_struct *p;
    struct file *file = sock->file;

    int i;

    read_lock(&tasklist_lock);
    for_each_task(p)
    {
	struct files_struct *files;

	task_lock(p);
	if ((files = p->files))
	{
	    read_lock(&files->file_lock);
	    for (i = 0; i < files->max_fds; i++)
	    {
		if (fcheck_files(files, i) == file)
		{
		    read_unlock(&files->file_lock);
		    task_unlock(p);
		    read_unlock(&tasklist_lock);

		    return p;
		}
	    }
	    read_unlock(&files->file_lock);
	}
	task_unlock(p);
    }
    read_unlock(&tasklist_lock);

    return NULL;
}
