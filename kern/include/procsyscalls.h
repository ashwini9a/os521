#ifndef _PROCSYSCALLS_H_
#define _PROCSYSCALLS_H_

#include <types.h>
#include <vfs.h>
#include <syscall.h>
#include <mips/trapframe.h>
#include <vm.h>
#include <addrspace.h>
#include <proc.h>
//struct lock* pid_lock;
//struct lock *fork_lock;
int sys_getpid(int *retval);
void child_forkentry(void *child_trapframe, unsigned long child_addrspace);
int sys_fork(struct trapframe *tf, int *returnvalue);
#endif
