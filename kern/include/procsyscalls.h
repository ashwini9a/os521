#ifndef _PROCSYSCALLS_H_
#define _PROCSYSCALLS_H_

#include <types.h>
#include <vfs.h>
#include <syscall.h>
#include <mips/trapframe.h>
#include <vm.h>
#include <addrspace.h>
#include <proc.h>
#include <kern/wait.h>
//#include <../proc/proc.c>
//struct lock* pid_lock;
//struct lock *fork_lock;
int sys_getpid(int *retval);
void child_forkentry(void *child_trapframe, unsigned long child_addrspace);
int sys_fork(struct trapframe *tf, int *returnvalue);
void sys_exit(int exitcode);
int sys_waitpid(pid_t pid, userptr_t status, int options, int *returnvalue);
int sys_execv(const_userptr_t program,const_userptr_t args);
#endif
