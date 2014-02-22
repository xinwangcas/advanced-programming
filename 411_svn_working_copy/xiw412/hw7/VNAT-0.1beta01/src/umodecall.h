/*
 * umodecall.h
 *
 * See the file "COPYRIGHT" for information on usage and redistribution
 * of this file.
 */

#ifndef __UMODECALL_H__
#define __UMODECALL_H__

struct umh_control
{
    int synchronous;                         /* Flag: make
						call_usermodehelper() block */
    void (*completion)(void *completion_arg,
		       int exit_code);       /* Called when subprocess exits
						if non-NULL */
    void *completion_arg;                    /* Private to caller */
    struct module *owner;
};

extern struct umh_control umhc;

extern int exec_usermodehelper(char *program_path, char *argv[], char *envp[]);

extern int call_usermodehelper2(char *path, char *argv[], char *envp[],
				struct umh_control *umh_c);

#endif
