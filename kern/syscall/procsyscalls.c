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
//	child_proc->parent_pid = curproc->proc_pid;
	thread_fork("child_thread", child_proc, enter_forked_process, (void*)child_tf, (unsigned long)child_addrspace);

	*returnvalue = child_proc->proc_pid;
	lock_release(pid_lock);
	return 0;
}
	

int sys_waitpid (pid_t pid, userptr_t status, int options, int *returnvalue) {

	(void) options;
	int result;
	struct proc *proc;
	proc = pid_array[pid];
	if (!proc) {
		kprintf("can't wait on a process that does not exist");
		return ESRCH;
	}

	if (curproc->parent_pid == pid) {
		kprintf("I don't think I'm allowed to wait for my parent to die !!!");
		return -1;
	}

	if (curproc->proc_pid != proc->parent_pid) {
		kprintf("I am not allowed to wait for someone else's child...");
		return ECHILD;
	}

	if (proc->__exited == false) {
		P(proc->proc_sem);
	}

	result = copyout((const void*) &proc->exitstatus, status, sizeof(int));
	if (result) {
		return EFAULT;
	}
	
	proc_destroy(proc);	
	
	*returnvalue = pid;	/*Coz the man page says so.... */
	return 0;
}

void sys_exit(int exitcode) {
	pid_t pid = curproc->proc_pid;
	struct proc *proc = pid_array[pid];
	if (!proc) {
		kprintf("process id not found...");
	}
	if (proc->__exited == false) {
		proc->__exited = true;
		proc->exitstatus = _MKWAIT_EXIT(exitcode);
		V(proc->proc_sem);
	}
	thread_exit();
}
