#ifndef _PROCSYSCALLS_H_
#define _PROCSYSCALLS_H_

#include <types.h>
#include <vfs.h>
#include <syscall.h>


struct lock* pid_lock;

int sys_getpid(int *retval);

#endif
