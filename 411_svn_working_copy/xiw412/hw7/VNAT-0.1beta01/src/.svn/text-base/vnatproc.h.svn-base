/*
 * vnatproc.h
 *
 * VNAT sysctl/proc interface module.
 *
 * Copyright (c) 2001-2002 Gong Su, All rights reserved.
 *
 * See the file "COPYRIGHT" for information on usage and redistribution
 * of this file.
 */

#ifndef __VNATPROC_H__
#define __VNATPROC_H__

/*#include <linux/sysctl.h>*/

#include "vnat.h" /* vnat_parm_t */

struct vnat_cmd
{
    struct vnat_cmd  *next;

    char comm[256];   /* command name              */
    vnat_parm_t parm; /* settings for this command */
};
typedef struct vnat_cmd vnat_cmd_t;

extern int  vnat_cmd_init(void);
extern void vnat_cmd_free(void);
extern int  vnat_cmd_verdict(const char *comm, int prot, vnat_parm_t *parm);
extern int  vnat_cmd_parm(vnat_parm_t *parm);

#endif
