#include <procsyscalls.h>
int sys_getpid(int *retval) {
	pid_t retpid = curproc->proc_pid;
	if (!retpid) {
//		kprintf("something is wrong...");
		return -1;
	}
	*retval = retpid;
	return 0;
}


int sys_fork(struct trapframe *tf, int *returnvalue) {
	int result;
	struct proc *child_proc;
	struct addrspace *child_addrspace;
//	lock_acquire(pid_lock);	
	struct trapframe *child_tf = (struct trapframe*)kmalloc(sizeof(struct trapframe));
	if (child_tf == NULL) {
//		kprintf("could not allocate memory for trapframe");
		return ENOMEM;
//		lock_release(pid_lock);
	}
	*child_tf = *tf;
	result = as_copy(proc_getas(), &child_addrspace);
	if (result) {
		return ENOMEM;
//		lock_release(pid_lock);
	}
	
	child_proc = proc_create_runprogram("childproc");
//	child_proc = proc_fork("childproc");
//	child_proc->p_addrspace = child_addrspace;
//	lock_release(pid_lock);
//	child_proc->parent_pid = curproc->proc_pid;
	thread_fork("child_thread", child_proc, enter_forked_process, (void*)child_tf, (unsigned long)child_addrspace);

	*returnvalue = child_proc->proc_pid;
	return 0;
}
	

int sys_waitpid (pid_t pid, userptr_t status, int options, int *returnvalue) {

	if (options != 0) {
//		kprintf("I dont accept options");
		return EINVAL;
	}
	int result;
	struct proc *proc;
	if (!status) {
		return EFAULT;
	}
	proc = pid_array[pid];
	if (!proc) {
//		kprintf("can't wait on a process that does not exist");
		return ESRCH;
	}

	if (curproc->parent_pid == pid) {
//		kprintf("I don't think I'm allowed to wait for my parent to die !!!");
		return -1;
	}

	if (curproc->proc_pid != proc->parent_pid) {
//		kprintf("I am not allowed to wait for someone else's child...");
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
//	pid_array[pid] = NULL;
	
	*returnvalue = pid;	/*Coz the man page says so.... */
	return 0;
}

void sys_exit(int exitcode) {
	pid_t pid = curproc->proc_pid;
	struct proc *proc = pid_array[pid];
//	pid_t part_pid =  proc->parent_pid; 	//	added in case parent has exited
//	lock_acquire(pid_lock); 
	if (!proc) {
//		kprintf("process id not found...");
	}
//	if(pid_array[part_pid]->__exited)		// added in case parent has exited
//	{
//		kprintf("this threads parent has exited, No need to store the exitcode");
		//proc_destroy(proc);
//		pid_array[pid] = NULL;
//	}
	if (proc->__exited == false) {
		proc->__exited = true;
		proc->exitstatus = _MKWAIT_EXIT(exitcode);
		V(proc->proc_sem);
	}
//	lock_release(pid_lock);
	thread_exit();
}

/*int sys_execv(const_userptr_t program, const_userptr_t **args)
{
	(void)args;
	char *newprog = kmalloc(PATH_MAX+1);
	char **kargs;
	size_t len;
	int i=0;
	while(i<ARG_MAX && args[i]!=NULL)
	{
		kargs[i]= kmalloc(sizeof(char) * strlen(*args[i])+1);
		copyinstr(args[i], kargs[i], strlen(*args[i])+1,&len );
		i++;
	}

        size_t length;
	if (newprog == NULL) {
                kprintf("kmalloc failed to  assign space for newprog in file.c\n");
                return ENOSPC;
        }
        int result =  copyinstr(program, newprog, PATH_MAX, &length);
	struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;

	result = vfs_open(newprog, O_RDONLY, 0, &v);
	if (result) {
		return result;
	}

	 //Create a new address space. 
	as = as_create();
	if (as == NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	// Switch to it and activate it. 
	proc_setas(as);
	as_activate();


	result = load_elf(v, &entrypoint);
	if (result) {
	
		vfs_close(v);
		return result;
	}

	// Done with the file now. 
	vfs_close(v);


	result = as_define_stack(as, &stackptr);
	if (result) {
	//	 p_addrspace will go away when curproc is destroyed 
		return result;
	}
	//Copy data from kernel buffer to stckptr
		
	
*/

	// Warp to user mode. 
//	enter_new_process(0 /*argc*/, NULL /*userspace addr of argv*/,
//			  NULL /*userspace addr of environment*/,
//			  stackptr, entrypoint);

//	/* enter_new_process does not return. */
//	panic("enter_new_process returned\n");
//	return EINVAL;

//}





