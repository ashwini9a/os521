#ifndef _PROCSYSCALLS_H_
#define _PROCSYSCALLS_H_

#include <types.h>
#include <kern/errno.h>
#include <kern/limits.h>
#include <kern/stat.h>
#include <kern/unistd.h>
#include <kern/fcntl.h>
#include <kern/seek.h>
#include <lib.h>
#include <synch.h>
#include <uio.h>
#include <thread.h>
#include <current.h>
#include <vfs.h>
#include <vnode.h>
#include <file.h>
#include <syscall.h>
#include <copyinout.h>


struct lock* pid_lock;

int sys_getpid(int *retval);

#endif
