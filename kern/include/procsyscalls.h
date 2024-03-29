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
int sys_execv(userptr_t progname, userptr_t args);
int sys_sbrk(intptr_t amount,int *returnvalue);
void deallocate_pages(vaddr_t end,intptr_t amount);
#endif
