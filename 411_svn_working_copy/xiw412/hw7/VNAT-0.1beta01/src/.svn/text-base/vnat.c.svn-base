/*
 * vnat.c
 *
 * VNAT entry module.
 *
 * Copyright (c) 2001-2002 Gong Su, All rights reserved.
 *
 * See the file "COPYRIGHT" for information on usage and redistribution
 * of this file.
 */

/*!
 * \defgroup vnat vnat
 *
 * \brief VNAT module
 *
 * \ingroup vnat
 *
 * VNAT kernel module implements the VNAT network virtualization and
 * translation functions to support migration of live end-to-end
 * transport layer network connections.
 *
 * VNAT module is loaded into the running kernel by the "modprobe" or
 * "insmod" command. "modprobe" is recommanded since it automatically
 * resolves module dependencies. To load the VNAT module (assuming it
 * has been compiled and installed):
 *
 *     modprobe kvnat [options]...
 *
 * To unload the VNAT module:
 *
 *     modprobe -r kvnat
 *
 * Note that the VNAT module cannot be unloaded if there are active
 * network connections being virtualized. This is reflected in the
 * module usage counter when you do "lsmod".
 *
 * Options are in the form of name=value. Valid option names and their
 * values are:
 *
 * \b debug=mask
 *
 * Debugging level.
 * \e Mask is a bit mask that controls the type of runtime debugging
 * information to be printed to console and syslog. Bits can be combined
 * to obtain more than one type of information. This optioni is only
 * available when VNAT has been compiled with the symbol DEBUG defined.
 *
 * \arg \e 0x00: don't print any runtime debugging information
 *
 * \arg \e 0x01: print socket calls that are intercepted and virtualized
 *
 * \arg \e 0x02: print socket calls that are intercepted but not virtualized
 *
 * \arg \e 0x04: print general information about what VNAT is doing
 *
 * \b timeo=n
 *
 * VNAT protocol timeout.
 * Set the timeout for VNAT protocol interaction (connection setup, message
 * read and write) to be n seconds.
 *
 * \b inver={0|1}
 *
 * Process list inversion.
 * Determine whether the list of processes whose network connections are
 * to be virtualized is inclusive or exclusive. If the list is inclusive,
 * then only the connections of the processes whose names are in the list
 * will be virtualized. If the list is exclusive, then only the connections
 * of the processes whose names are \e not in the list will be virtualized.
 * This will become clearer below with the description of the VNAT /proc
 * file system entries.
 *
 * \arg \e 0: the list is inclusive
 *
 * \arg \e 1: the list is exclusive
 *
 * \b pmask={0x1|0x2}
 *
 * Protocol mask.
 * Bit mask to determine which transport layer protocol to virtualize.
 *
 * \arg \e 0x1: TCP
 *
 * \arg \e 0x2: UPD
 *
 * \b tcpka=n
 *
 * TCP keepalive timer.
 * Set the TCP keepalive timer (in seconds) to be negotiated with the
 * remote peer when the local host is suspended. A value of 0 means to
 * disable the TCP keepalive timer on the remote peer for the migrating
 * connections, i.e., to keep the migrating connections alive indefinitely.
 *
 * \b fflag={0|1}
 *
 * Fixup flag.
 * By default, VNAT always reports to applications the physical IP address
 * of the current host. Some applications, such as FTP, explicitly
 * check the current IP address against a saved one when making new
 * connections. These applications will not work if moved. The \e fflag
 * option allows VNAT to report to applications the virtual IP address
 * rather than the physical IP address so the movement of the host will be
 * transparent to the applications.
 *
 * \arg \e 0: report physical IP address
 *
 * \arg \e 1: report virtual IP address
 *
 * \b trans={0|1}
 *
 * Forced translation.
 * Normally, VNAT installs translation rules only when necessary, i.e.,
 * when the virtual addresses are different from the physical addresses.
 * This flag forces VNAT to install (dummy) translation rules even when the
 * virtual addresses are the same as the physical addresses. This is only
 * intended for certain performance measurement purpose and is almost never
 * used under normal circumstances.
 *
 * \arg \e 0: no forced translation
 *
 * \arg \e 1: forced translation
 *
 *
 * When VNAT is loaded into the running kernel, it hooks into the socket
 * system calls and virtualizes connections according to a list of process
 * names. The list can be either inclusive, i.e., containing process names
 * whose connections are to be virtualized, or exclusive, i.e., containing
 * process names whose connections are \e not to be virtualized. This list,
 * along with other states of VNAT, is managed at runtime through the /proc
 * file system entries exported by VNAT. The following entries are exported
 * when VNAT is loaded:
 *
 * \b /proc/sys/net/vnat/command
 *
 * To add or remove a process name from the list of process names whose
 * connections are being virtualized:
 *
 * echo "{+|-}process_name tcpka pmask fflag trans" > /proc/sys/net/vnat/command
 *
 * The values for \e tcpka, \e pmask, \e fflag, and \e trans have the same
 * meaning as those one specifies as the options when loading VNAT module.
 * The difference is that here these values are per process, i.e., they only
 * apply to the connections created by the particular process. The values
 * one specifies as the options when loading VNAT module are global values
 * that apply to all processes whose connections are being virtualized.
 * When both types of values are present, the per process values always
 * take precedence over the global values.
 *
 * The command:
 *
 * echo "process_name tcpka pmask fflag trans" > /proc/sys/net/vnat/command
 *
 * will modify the values of \e tcpka, \e pmask, \e fflag, and \e trans for
 * the specified process if it is in the list; otherwise, this command has
 * no effect.
 *
 * The following command:
 *
 * cat /proc/sys/net/vnat/command
 *
 * will give a list of processes currently being virtualized along with
 * their respective \e tcpka, \e pmask, \e fflag, and \e trans values.
 *
 *
 * \b /proc/sys/net/vnat/debug
 *
 * Examine or modify the debugging level value at runtime.
 *
 * \b /proc/sys/net/vnat/timeo
 *
 * Examine or modity the VNAT protocol timeout value at runtime.
 *
 * \b /proc/sys/net/vnat/inver
 *
 * Examine or modify the process list inversion value at runtime.
 *
 * \b /proc/sys/net/vnat/pmask
 *
 * Examine or modify the global protocol mask value at runtime.
 *
 * \b /proc/sys/net/vnat/tcpka
 *
 * Examine or modify the global TCP keepalive timer value at runtime.
 *
 * \b /proc/sys/net/vnat/fflag
 *
 * Examine or modify the global fixup flag value at runtime.
 *
 * \b /proc/sys/net/vnat/trans
 *
 * Examine or modify the global forced translation value at runtime.
 *
 * \b /proc/sys/net/vnat/suspend and \b /proc/sys/net/vnat/resume
 *
 * These are read-only entries. The use of these entries is to notify VNAT
 * of system suspension and resumption events and to interface VNAT with
 * other system mechanisms, e.g., apmd, that handle suspension and resumption
 * events. Reading from \b /proc/sys/net/vnat/suspend signals VNAT that the
 * system is being suspended so that states of virtualized connections will
 * be saved to allow the connections to be suspended and migrated. Reading
 * from \b /proc/sys/net/vnat/resume signals VNAT that the system is being
 * resumed so that states of the virtualized connections can be restored to
 * allow the suspend connections to be resumed. Writing into these entries
 * has no effect.
 *
 *
 * \b Bug:
 *
 * Migrating FTP client in active mode (or server in passive mode)
 * is not yet working.
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

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kmod.h>

#include "vnatd.h"
#include "vnatmap.h"
#include "vnatproc.h"
#include "vnatsock.h"
#include "vnatvif.h"
#include "misc.h"

MODULE_DESCRIPTION("Virtualizing socket calls");
MODULE_AUTHOR("Gong Su, (C) 2001-2002, GPLv2 or later");
#if (LINUX_VERSION_CODE >= 0x02040e)
MODULE_LICENSE("GPL");
#endif

#ifdef DEBUG
MODULE_PARM(debug, "i");
int   debug = 0;
#endif
MODULE_PARM(timeo, "i");
MODULE_PARM(inver, "i");
/*
 * Mirror of per-command settings.
 */
MODULE_PARM(pmask, "i");
MODULE_PARM(tcpka, "i");
MODULE_PARM(fflag, "i");
MODULE_PARM(trans, "i");

int timeo = 3;
int inver = 0;
/*
 * Mirror of per-command settings.
 */
int pmask = 0;
int tcpka = 0;
int fflag = 0;
int trans = 0;

static struct kthr_data kvnatd_data;

/*
 * Here's our initialization routine. __init is a marker to show
 * that it can be thrown away after use, though it doesn't make
 * any difference for modules (yet)
 */
int __init init_kvnat(void)
{
    int err;

    if ((err = request_module("kvnatvif")) < 0) return err;
    /*
     * Make sure module vifdriver is loaded.
     */
    if (vnatvif_probe == NULL) return -ENODEV;

    /*
     * Initialize mapping table.
     */
    if ((err = vnat_map_init()) < 0) return err;

    /*
     * Register with CRAK.
     */
    /*vnat_crak_init();*/

    /*
     * Register proc entry table.
     */
    if ((err = vnat_cmd_init()) < 0)
    {
	PRINTK4(debug, ("[%s:%d|init_kvnat]: warning: unable to register \
/proc entries\n", current->comm, current->pid));
    }

    /*
     * Spawn off kernel VNAT virtual-physical mapping exchange/update
     * protocol daemon thread.
     */
    start_kvnatd(kvnatd, &kvnatd_data);

    /*
     * Hook into socket calls.
     */
    vnat_sock_init();

    /*
     * Everything's OK. We'd return -EINVAL or similar if it wasn't.
     */
    return 0;
}

/*
 * Here's our exit routine. __exit is a marker to show that it can be
 * thrown away if we don't compile as a module, as the code can then
 * never be unloaded :)
 */
void __exit exit_kvnat(void)
{
    /*
     * Unhook from socket calls.
     */
    vnat_sock_free();

    /*
     * Stop kvnatd thread.
     */
    stop_kvnatd(&kvnatd_data);

    /*
     * Unregister proc entry table.
     */
    vnat_cmd_free();

    /*
     * Deregister with CRAK.
     */
    /*vnat_crak_free();*/

    /*
     * Clean up mapping table.
     */
    vnat_map_free();

    /*
     * modprobe will take care of removing kvnatvif module.
     */
}

/*
 * Macros to tell module loader our init and exit routines.
 */
module_init(init_kvnat);
module_exit(exit_kvnat);
