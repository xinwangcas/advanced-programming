/*
 * vnatproc.c
 *
 * VNAT sysctl/proc interface module.
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

#include <linux/mm.h>
#include <linux/slab.h>

#include <linux/sysctl.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include <net/sock.h>

#include "vnat.h"
#include "vnatmap.h"
#include "vnatproc.h"
#include "misc.h"

static int proc_dovnatcmd(ctl_table *table, int write, struct file *filp,
			  void *buffer, size_t *lenp);
static int sysctl_vnatcmd(ctl_table *table, int *name, int nlen,
			  void *oldval, size_t *oldlenp,
			  void *newval, size_t newlen, void **context);
static int proc_dovnatmap(ctl_table *table, int write, struct file *filp,
			  void *buffer, size_t *lenp);
static int sysctl_vnatmap(ctl_table *table, int *name, int nlen,
			  void *oldval, size_t *oldlenp,
			  void *newval, size_t newlen, void **context);
static int proc_dovnatsuspend(ctl_table *table, int write, struct file *filp,
			      void *buffer, size_t *lenp);
static int sysctl_vnatsuspend(ctl_table *table, int *name, int nlen,
			      void *oldval, size_t *oldlenp,
			      void *newval, size_t newlen, void **context);
static int proc_dovnatresume(ctl_table *table, int write, struct file *filp,
			     void *buffer, size_t *lenp);
static int sysctl_vnatresume(ctl_table *table, int *name, int nlen,
			     void *oldval, size_t *oldlenp,
			     void *newval, size_t newlen, void **context);

static int find_by_cmd(vnat_cmd_t *p1, vnat_cmd_t *p2);

#define NET_VNAT 2031

enum
{
    VNAT_INVER = 1,
    VNAT_TCPKA,
    VNAT_PROT,
    VNAT_FFLAG,
    VNAT_TRANS,
    VNAT_TIMEO,
    VNAT_MAP,
    VNAT_COMMAND,
    VNAT_SUSPEND,
    VNAT_RESUME,
#ifdef DEBUG
    VNAT_DEBUG
#endif
};
enum { VNATPROTO_TCP = 0x01, VNATPROTO_UDP = 0x02 };

static vnat_cmd_t *vnat_cmd_base = NULL;
static spinlock_t  vnat_cmd_lock = SPIN_LOCK_UNLOCKED;

static ctl_table vnat_table[] =
{
    {
	/*
	 * Command list is inclusive or exclusive.
	 */
	VNAT_INVER, "inver", &inver, sizeof(int), 0644, NULL,
	&proc_dointvec, &sysctl_intvec, 0, 0, 0
    },
    {
	/*
	 * Global TCP keepalive timer. Overridden by per-command timer.
	 */
	VNAT_TCPKA, "tcpka", &tcpka, sizeof(int), 0644, NULL,
	&proc_dointvec, &sysctl_intvec, 0, 0, 0
    },
    {
	/*
	 * Global protocol mask. Overridden by per-command mask.
	 */
	VNAT_PROT, "pmask", &pmask, sizeof(int), 0644, NULL,
	&proc_dointvec, &sysctl_intvec, 0, 0, 0
    },
    {
	/*
	 * Global fix flag. Overridden by per-command flag.
	 */
	VNAT_FFLAG, "fflag", &fflag, sizeof(int), 0644, NULL,
	&proc_dointvec, &sysctl_intvec, 0, 0, 0
    },
    {
	/*
	 * Global forced address translation flag. Overridden by per-command
	 * flag.
	 */
	VNAT_TRANS, "trans", &trans, sizeof(int), 0644, NULL,
	&proc_dointvec, &sysctl_intvec, 0, 0, 0
    },
    {
	/*
	 * Timeout for remote VNAT daemon probing.
	 */
	VNAT_TIMEO, "timeo", &timeo, sizeof(int), 0644, NULL,
	&proc_dointvec, &sysctl_intvec, 0, 0, 0
    },
    {
	/*
	 * Dump VNAT mapping entries.
	 */
	VNAT_MAP, "map", NULL, 0, 0644, NULL,
	&proc_dovnatmap, &sysctl_vnatmap, 0, 0, 0
    },
    {
	/*
	 * Command list.
	 */
	VNAT_COMMAND, "command", NULL, 0, 0644, NULL,
	&proc_dovnatcmd, &sysctl_vnatcmd, 0, 0, 0
    },
    {
	/*
	 * Trigger for suspending.
	 */
	VNAT_SUSPEND, "suspend", NULL, 0, 0644, NULL,
	&proc_dovnatsuspend, &sysctl_vnatsuspend, 0, 0, 0
    },
    {
	/*
	 * Trigger for resuming.
	 */
	VNAT_RESUME, "resume", NULL, 0, 0644, NULL,
	&proc_dovnatresume, &sysctl_vnatresume, 0, 0, 0
    },
#ifdef DEBUG
    {
	/*
	 * Debug level.
	 */
	VNAT_DEBUG, "debug", &debug, sizeof(int), 0644, NULL,
	&proc_dointvec, &sysctl_intvec, 0, 0, 0
    },
#endif
    { 0 }
};

static ctl_table vnat_dir_table[] =
{
    {
	NET_VNAT, "vnat", NULL, 0, 0555, vnat_table,
	0, 0, 0, 0, 0
    },
    { 0 }
};

static ctl_table vnat_root_table[] =
{
    {
	CTL_NET, "net", NULL, 0, 0555, vnat_dir_table,
	0, 0, 0, 0, 0
    },
    { 0 }
};

static struct ctl_table_header *vnat_table_header;

static int
find_by_cmd(vnat_cmd_t *p1, vnat_cmd_t *p2)
{
    return !strcmp(p1->comm, p2->comm);
}

int
vnat_cmd_init(void)
{
    int err = 0;

    PRINTK4(debug, ("[%s:%d|vnat_cmd_init]: initializing proc entries { ", \
		    current->comm, current->pid));

    if ((vnat_table_header = register_sysctl_table(vnat_root_table, 0))
	== NULL)
    {
	PRINTK4(debug, ("xxx"));
	err = -ENOMEM;
    }
    else
    {
	PRINTK4(debug, ("..."));
    }

    PRINTK4(debug, (" }\n"));

    return err;
}

void
vnat_cmd_free(void)
{
    /*
     * Before unregistering proc entry table, clean up the list of
     * command entries.
     */
    PRINTK4(debug, ("[%s:%d|vnat_cmd_free]: freeing command entry list { ", \
		    current->comm, current->pid));

    if (vnat_cmd_base)
    {
	PRINTK4(debug, ("+"));
	list_free((void **)&vnat_cmd_base);
    }
    else
    {
	PRINTK4(debug, ("."));
    }

    PRINTK4(debug, (" }\n"));

    /*
     * Unregister proc entry table.
     */
    PRINTK4(debug, ("[%s:%d|vnat_cmd_free]: freeing proc entries { ... }\n", \
		    current->comm, current->pid));
    unregister_sysctl_table(vnat_table_header);
}

static int
proc_dovnatcmd(ctl_table *table, int write, struct file *filp,
	       void *buffer, size_t *lenp)
{
    size_t len;
    char  *p, c;
    int    err = 0;

    /*
     * Default: virtualize both TCP and UDP; no fix.
     */
    vnat_cmd_t *cmd, tmp;

    if (!*lenp || (filp->f_pos && !write))
    {
	*lenp = 0;
	return 0;
    }

    if (write)
    {
	len = 0;
	p = buffer;
	while (len < *lenp)
	{
	    if(get_user(c, p++)) return -EFAULT;

	    if (c == '\0' || c == '\n') break;

	    len++;
	}

	/*
	 * Allocate temporary space and copy the string over from userspace.
	 */
	if ((p = kmalloc(len+1, GFP_KERNEL)) == NULL) return -ENOMEM;
	if(copy_from_user(p, buffer, len))
	{
	    kfree(p);
	    return -EFAULT;
	}
	p[len] = '\0';

	/*
	 * Look at the first character of the command and decide
	 * what to do:
	 *
	 * +command: insert the command
	 * -command: delete the command
	 *  command: modify the command
	 */
	switch(p[0])
	{
	case '+':
	    if ((cmd = kmalloc(sizeof(vnat_cmd_t), GFP_KERNEL)) == NULL)
	    {
		err =  -ENOMEM;
		break;
	    }
	    memset(cmd, 0, sizeof(vnat_cmd_t));

	    sscanf(&p[1], "%255s %d %d %d %d", cmd->comm, &cmd->parm.tcpka,
		   &cmd->parm.pmask, &cmd->parm.fflag, &cmd->parm.trans);
	    if (cmd->comm[0] == '\0')
	    {
		kfree(cmd);
		err = -EINVAL;
		break;
	    }

	    spin_lock(&vnat_cmd_lock);
	    if (list_insert((void **)&vnat_cmd_base, (void *)cmd,
			    (int (*)(void *, void *))find_by_cmd))
	    {
		kfree(cmd);
		err = -EEXIST;
	    }
	    spin_unlock(&vnat_cmd_lock);
	    break;
	case '-':
	    memset(&tmp, 0, sizeof(vnat_cmd_t));

	    sscanf(&p[1], "%255s %d %d %d %d",
		   tmp.comm, &tmp.parm.tcpka, &tmp.parm.pmask,
		   &tmp.parm.fflag, &tmp.parm.trans);
	    if (tmp.comm[0] == '\0')
	    {
		err = -EINVAL;
		break;
	    }

	    spin_lock(&vnat_cmd_lock);
	    if ((cmd = list_delete((void **)&vnat_cmd_base, (void *)&tmp,
				   (int (*)(void *, void *))find_by_cmd))
		== NULL)
	    {
		err = -ENOENT;
	    }
	    else
	    {
		kfree(cmd);
	    }
	    spin_unlock(&vnat_cmd_lock);
	    break;
	default:
	    memset(&tmp, 0, sizeof(vnat_cmd_t));

	    sscanf(&p[0], "%255s %d %d %d %d",
		   tmp.comm, &tmp.parm.tcpka, &tmp.parm.pmask,
		   &tmp.parm.fflag, &tmp.parm.trans);
	    if (tmp.comm[0] == '\0')
	    {
		err = -EINVAL;
		break;
	    }

	    spin_lock(&vnat_cmd_lock);
	    if ((cmd = list_find((void **)&vnat_cmd_base, (void *)&tmp,
				 (int (*)(void *, void *))find_by_cmd))
		== NULL)
	    {
		err = -ENOENT;
	    }
	    else
	    {
		sprintf(cmd->comm, "%s", tmp.comm);
		cmd->parm = tmp.parm;
	    }
	    spin_unlock(&vnat_cmd_lock);
	    break;
	}

	if (!err) filp->f_pos += *lenp;
    }
    else
    {
#define VNAT_COMMAND_FT "%-32s %-5d %02x %02x %02x\n"
#define VNAT_COMMAND_ST "command                          ka    pm fx tr"

	if (*lenp < sizeof(VNAT_COMMAND_ST+2))
	{
	    *lenp = 0;
	    return 0;
	}

	/*
	 * Format into a temporary buffer.
	 */
	if ((p = kmalloc(*lenp, GFP_KERNEL)) == NULL) return -ENOMEM;

	len = 0;
	len += sprintf(p+len, "%s\n", VNAT_COMMAND_ST);

	cmd = vnat_cmd_base;
	spin_lock(&vnat_cmd_lock);
	while (cmd && (*lenp-len) >= (sizeof(VNAT_COMMAND_ST)+2))
	{
	    len += sprintf(p+len, VNAT_COMMAND_FT, cmd->comm, cmd->parm.tcpka,
			   cmd->parm.pmask, cmd->parm.fflag, cmd->parm.trans);
	    cmd = cmd->next;
	}
	spin_unlock(&vnat_cmd_lock);

	/*
	 * Copy into userspace.
	 */
	if(copy_to_user(buffer, p, len))
	{
	    err = -EFAULT;
	}
	else
	{
	    filp->f_pos += (*lenp = len);
	}
    }

    kfree(p);

    return err;
}

static int
sysctl_vnatcmd(ctl_table *table, int *name, int nlen,
	       void *oldval, size_t *oldlenp,
	       void *newval, size_t newlen, void **context)
{
    /*
     * Not yet supported.
     */
    return -ENOSYS;
}

/*
 * Dump VNAT mapping table.
 */
static int
proc_dovnatmap(ctl_table *table, int write, struct file *filp,
	       void *buffer, size_t *lenp)
{
    char  *p;
    int    err = 0;

    if (!*lenp || (filp->f_pos && !write))
    {
	*lenp = 0;
	return 0;
    }

    /*
     * No write yet.
     */
    if (write) return -ENOSYS;

    /*
     * Format into a temporary buffer.
     */
    if ((p = kmalloc(*lenp, GFP_KERNEL)) == NULL) return -ENOMEM;

    *lenp = vnat_map_dump(p, *lenp);

    /*
     * Copy into userspace.
     */
    if(copy_to_user(buffer, p, *lenp))
    {
	*lenp = 0;
	err = -EFAULT;
    }
    else
    {
	filp->f_pos += *lenp;
    }

    kfree(p);

    return 0;
}

static int
sysctl_vnatmap(ctl_table *table, int *name, int nlen,
	       void *oldval, size_t *oldlenp,
	       void *newval, size_t newlen, void **context)
{
    /*
     * Not yet supported.
     */
    return -ENOSYS;
}

static int
proc_dovnatsuspend(ctl_table *table, int write, struct file *filp,
		   void *buffer, size_t *lenp)
{
    vnat_map_suspend(NULL);

    if (write) filp->f_pos += *lenp;
    else *lenp = 0;

    return 0;
}

static int
sysctl_vnatsuspend(ctl_table *table, int *name, int nlen,
		   void *oldval, size_t *oldlenp,
		   void *newval, size_t newlen, void **context)
{
    /*
     * Not yet supported.
     */
    return -ENOSYS;
}

static int
proc_dovnatresume(ctl_table *table, int write, struct file *filp,
		  void *buffer, size_t *lenp)
{
    vnat_map_resume(NULL);

    if (write) filp->f_pos += *lenp;
    else *lenp = 0;

    return 0;
}

static int
sysctl_vnatresume(ctl_table *table, int *name, int nlen,
		  void *oldval, size_t *oldlenp,
		  void *newval, size_t newlen, void **context)
{
    /*
     * Not yet supported.
     */
    return -ENOSYS;
}

/*
 * Return the verdict of whether or not to virtualize the socket
 * given the command and protocol.
 *
 * Also pass back via a value-result variable whether or not to
 * fix up the "odd" applications like FTP.
 */
int
vnat_cmd_verdict(const char *comm, int prot, vnat_parm_t *parm)
{
    vnat_cmd_t *cmd, tmp;

    int verdict = 0;

    /*
     * First fill up the template and search the list.
     */
    memset(&tmp, 0, sizeof(vnat_cmd_t));
    strncpy(tmp.comm, comm, sizeof(tmp.comm)-1);
    tmp.comm[sizeof(tmp.comm)-1] = '\0';

    spin_lock(&vnat_cmd_lock);
    /*
     * We found the command and the list is "inclusive". Or
     * we didn't find the command and the list is "exclusive".
     */
    if (((cmd = list_find((void **)&vnat_cmd_base, (void *)&tmp,
			  (int (*)(void *, void *))find_by_cmd)) && !inver) ||
	(!cmd && inver))
    {
	/*
	 * Check protocol.
	 */
	switch(prot)
	{
	case IPPROTO_TCP:
	    /*
	     * Use per-command setting if the command is in the list.
	     */
	    if (cmd)
	    {
		/*
		 * Masked out.
		 */
		if ((cmd->parm.pmask & VNATPROTO_TCP)) break;

		*parm = cmd->parm;
	    }
	    /*
	     * Otherwise use global setting.
	     */
	    else
	    {
		if ((pmask & VNATPROTO_TCP)) break;

		parm->tcpka = tcpka;
		parm->fflag = fflag;
		parm->trans = trans;
	    }

	    verdict = 1;
	    break;

	case IPPROTO_UDP:
	    /*
	     * Use per-command setting if the command is in the list.
	     */
	    if (cmd)
	    {
		/*
		 * Masked out.
		 */
		if ((cmd->parm.pmask & VNATPROTO_UDP)) break;

		*parm = cmd->parm;
	    }
	    /*
	     * Otherwise use global setting.
	     */
	    else
	    {
		if ((pmask & VNATPROTO_UDP)) break;

		parm->tcpka = tcpka;
		parm->fflag = fflag;
		parm->trans = trans;
	    }

	    verdict = 1;
	    break;

	default:
	    break;
	}
    }
    spin_unlock(&vnat_cmd_lock);

    if (verdict)
    {
	PRINTK1(debug, ("[%s:%d|vnat_cmd_verdict]: \
+comm=\"%s\", prot=%d, {tcpka=%d,pmask=%d,fflag=%d,trans=%d}\n", \
			current->comm, current->pid, comm, prot, \
			parm->tcpka, parm->pmask, parm->fflag, parm->trans));
    }
    else
    {
	PRINTK2(debug, ("[%s:%d|vnat_cmd_verdict]: \
-comm=\"%s\", prot=%d, {tcpka=%d,pmask=%d,fflag=%d,trans=%d}\n", \
			current->comm, current->pid, comm, prot, \
			parm->tcpka, parm->pmask, parm->fflag, parm->trans));
    }

    return verdict;
}

/*
 * Return the global configuration settings.
 */
int
vnat_cmd_parm(vnat_parm_t *parm)
{
    spin_lock(&vnat_cmd_lock);
    parm->tcpka = tcpka;
    parm->pmask = pmask;
    parm->fflag = fflag;
    parm->trans = trans;
    spin_unlock(&vnat_cmd_lock);

    return 0;
}
