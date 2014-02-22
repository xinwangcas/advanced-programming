/*
 * vnat.h
 *
 * Export global variables used by VNAT system.
 *
 * Copyright (c) 2001-2002 Gong Su, All rights reserved.
 *
 * See the file "COPYRIGHT" for information on usage and redistribution
 * of this file.
 */

#ifndef __VNAT_H__
#define __VNAT_H__

/*
 * Our magic socket flag to indicate request for VNAT.
 */
#define O_VNAT  0x80000000L

/*
 * Per-command settings.
 */
struct vnat_parm
{
    int  tcpka; /* TCP keepalive when suspended */
    int  pmask; /* TCP/UDP mask                 */
    int  fflag; /* FTP-like application fix     */
    int  trans; /* Force address translation    */
};
typedef struct vnat_parm vnat_parm_t;

#ifdef DEBUG
extern int   debug;
#endif

extern int   inver;
/*
 * Global settings mirroring per-command settings.
 */
extern int   tcpka;
extern int   pmask;
extern int   fflag;
extern int   trans;

extern int   timeo;

#endif
