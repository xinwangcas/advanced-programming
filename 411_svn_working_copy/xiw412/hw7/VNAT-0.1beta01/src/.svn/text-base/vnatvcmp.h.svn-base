/*
 * vcmp.h
 *
 * Data structures used by VNAT Connection Migration Protocol (VCMP).
 *
 * Copyright (c) 2001-2002 Gong Su, All rights reserved.
 *
 * See the file "COPYRIGHT" for information on usage and redistribution
 * of this file.
 */

#ifndef __VCMP_H__
#define __VCMP_H__

#include <asm/types.h>    /* s16, u32                */
#include <linux/net.h>    /* struct socket, sockaddr */
#include <linux/socket.h> /* struct msghdr           */
#include <linux/uio.h>    /* struct iovec            */

/*
 * Protocol number (used by "socket" call) for VNAT.
 */
#define IPPROTO_VNAT 832

/*
 * Note the order of entries.
 */
struct vcmp_pdu
{
    char version;
    char command;

    s16  r_korv_port; /* remote kernel/user port mapping requested */
    u32  r_korv_addr; /* remote VNAT-ed address we are using/seeing;
		       * this should have been l_vnat_addr if we use
		       * route lookup; but for now we will tell each
		       * other about our VNAT-ed address
		       */

    u32  l_korv_addr; /* local kernel or VNAT address  */
    u32  l_vnat_addr; /* local updated VNAT-ed address */
    s16  l_korv_port; /* local kernel or VNAT port     */
    s16  l_vnat_port; /* local updated VNAT-ed port    */
};
#define VCMP_VER 1
enum
{
    VCMP_ALL,
    VCMP_XCH,
    VCMP_XCH_R,
    VCMP_UPD,
    VCMP_UPD_R,
    VCMP_UPD_R_ACK,
    VCMP_SUSP,
};

struct vcmp_msg
{
    struct msghdr    hdr;   /* control header          */
    struct iovec     iov;   /* scattered buffer        */
    struct vcmp_pdu  pdu;   /* actual PDU              */

    struct socket   *sock;  /* for accessing transport */
    /*struct sockaddr *addr;*/  /* needed by sock_sendmsg  */
    struct vnat_map *map;   /* filled in by sock_recvmsg
			     * and used by sock_sendmsg
			     */
};
typedef struct vcmp_msg vcmp_msg_t;

extern int vcmp_socket(unsigned int addr, unsigned short port,
		       struct socket **sock, long timeout);
extern int vcmp_sendmsg(struct socket *sock, vcmp_msg_t *vcmp, char command,
			int flags, long timeout);
extern int vcmp_recvmsg(struct socket *sock, vcmp_msg_t *vcmp, char command,
			int flags, long timeout);

#endif
