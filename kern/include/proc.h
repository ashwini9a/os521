/*
 * Copyright (c) 2013
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _PROC_H_
#define _PROC_H_

/*
 * Definition of a process.
 *
 * Note: curproc is defined by <current.h>.
 */

#include <spinlock.h>
#include <file.h>
//#include <procsyscalls.h>
//#include <sys/types.h>

struct addrspace;
struct thread;
struct vnode;

extern struct proc *pid_array[PID_MAX];
extern int total_pids;
extern struct lock* pid_lock;
//extern bool pid_initialized;

//static struct proc *pid_array[PID_MAX];
//for (int i=0;i<PID_MAX;i++) {
//	pid_array[i] = NULL;
//}
//static int total_pids = 0;
//static struct lock* pid_lock = lock_create("pid_lock");

//static struct proc *pid_array[PID_MAX];
//static int total_pids;
//static struct lock *pid_lock;
//static bool pid_initialized = false;

pid_t request_pid(struct proc *proc);
int pid_destroy(pid_t pid);
void pid_initialize(void);

//void pid_initialize() {
//	for(int i=0;i<PID_MAX;i++) {
//		pid_array[i] =NULL;
//	}
//	pid_lock = lock_create("pid_lock");
//}


//pid_t request_pid(struct proc *proc) {
	
//	if (pid_initialized == false) {
//		pid_initialize();
//		pid_initialized = true;
//	}	

//	int i=2;
//	while(i<PID_MAX) {
//		if (pid_array[i] == NULL) {
//			break;
//		}
//		i++;
//	}
//	total_pids++;
//	pid_array[i] = proc;	
//	return (pid_t)i;
//	
//}

//int pid_destroy(pid_t pid) {
//	int i = (int)pid;
//	pid_array[i] = NULL;
//	total_pids--;
//	return 0;
//}


/*
 * Process structure.
 *
 * Note that we only count the number of threads in each process.
 * (And, unless you implement multithreaded user processes, this
 * number will not exceed 1 except in kproc.) If you want to know
 * exactly which threads are in the process, e.g. for debugging, add
 * an array and a sleeplock to protect it. (You can't use a spinlock
 * to protect an array because arrays need to be able to call
 * kmalloc.)
 *
 * You will most likely be adding stuff to this structure, so you may
 * find you need a sleeplock in here for other reasons as well.
 * However, note that p_addrspace must be protected by a spinlock:
 * thread_switch needs to be able to fetch the current address space
 * without sleeping.
 */
struct proc {
	char *p_name;			/* Name of this process */
	struct spinlock p_lock;		/* Lock for this structure */
	unsigned p_numthreads;		/* Number of threads in this process */

	/* VM */
	struct addrspace *p_addrspace;	/* virtual address space */

	/* VFS */
	struct vnode *p_cwd;		/* current working directory */

	/* add more material here as needed */
	struct filehandle *filedescriptor[OPEN_MAX];
	pid_t proc_pid;
	pid_t parent_pid;
	struct semaphore *proc_sem;
	bool __exited;
	int exitstatus;
};

/* This is the process structure for the kernel and for kernel-only threads. */
extern struct proc *kproc;

/* Call once during system startup to allocate data structures. */
void proc_bootstrap(void);

/* Create a fresh process for use by runprogram(). */
struct proc *proc_create_runprogram(const char *name);

/* Destroy a process. */
void proc_destroy(struct proc *proc);

/* Attach a thread to a process. Must not already have a process. */
int proc_addthread(struct proc *proc, struct thread *t);

/* Detach a thread from its process. */
void proc_remthread(struct thread *t);

/* Fetch the address space of the current process. */
struct addrspace *proc_getas(void);

/* Change the address space of the current process, and return the old one. */
struct addrspace *proc_setas(struct addrspace *);

struct proc *proc_fork(const char *temp);
//pid_t request_pid(struct proc *proc);
#endif /* _PROC_H_ */
