#include <procsyscalls.h>

int sys_getpid(int *retval) {
	pid_t retpid = curproc->proc_pid;
	if (!retpid) {
		kprintf("something is wrong...");
		return -1;
	}
	*retval = retpid;
	return 0;
}

//void child_forkentry(void* child_trapframe, unsigned long child_addrspace) {
//
//	struct trapframe *tf = (struct trapframe *)child_trapframe;
//	struct addrspace *adspc = (struct addrspace *)child_addrspace;
//
//	tf->tf_v0 = 0;
//	tf->tf_a3 = 0;
//	tf->tf_epc = tf->tf_epc+4;
//
//	proc_setas(adspc);
//	as_activate();
//	struct trapframe temp = *tf;
//	mips_usermode(&temp);
//
//}

int sys_fork(struct trapframe *tf, int *returnvalue) {
	int result;
	struct proc *child_proc;
	struct addrspace *child_addrspace;
	lock_acquire(pid_lock);	
	struct trapframe *child_tf = (struct trapframe*)kmalloc(sizeof(struct trapframe));
	if (child_tf == NULL) {
		kprintf("could not allocate memory for trapframe");
		return ENOMEM;
		lock_release(pid_lock);
	}
	child_tf = tf;
	result = as_copy(proc_getas(), &child_addrspace);
	if (result) {
		return ENOMEM;
		lock_release(pid_lock);
	}
	
	child_proc = proc_create_runprogram("childproc");
	child_proc->parent_pid = curproc->proc_pid;
	thread_fork("child_thread", child_proc, enter_forked_process, (void*)child_tf, (unsigned long)child_addrspace);

	*returnvalue = child_proc->proc_pid;
	lock_release(pid_lock);
	return 0;
}
