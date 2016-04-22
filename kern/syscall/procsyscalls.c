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
		kfree(child_addrspace);
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
//	if (!((int)status & 0x3)) {
//		return EFAULT;
//	}

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


int sys_execv(userptr_t progname, userptr_t args)
{
	char *newprog;
	char *buffer_end;
	vaddr_t entrypoint, stackptr;
	size_t buffer_max = ARG_MAX;
	size_t buffer_rem = ARG_MAX;
	int argc;
	userptr_t address;
	userptr_t addr_start;
	int result;
	char *buffer;
	size_t arg_len;
	size_t *offset;
	int arg_tot;
	struct addrspace *as;
	struct vnode *v;
	userptr_t stack_start;
	newprog = kmalloc(PATH_MAX);
	if (newprog == NULL) {
		return ENOMEM;
	}
	userptr_t arg_start;
	offset = kmalloc(64 * sizeof(char*));
	if (offset == NULL) {
		//kfree(buffer);
		kfree(newprog);
		return ENOMEM;
	}
	buffer = kmalloc(ARG_MAX);
	if (buffer == NULL) {
		kfree(newprog);
		kfree(offset);
		return ENOMEM;
	}

	buffer_end = buffer;

	result = copyinstr(progname, newprog, PATH_MAX, NULL);
	if (result) {
		kfree(newprog);
		kfree(buffer);
		kfree(offset);
		return result;
	}

	for (arg_tot = 0; arg_tot<64;arg_tot++) {
		result = copyin(args, &arg_start, sizeof(char*));
		if (result) {
			kfree(progname);
			kfree(buffer);
			kfree(offset);
			return result;
		}
		if (arg_start == NULL) {
			break;
		}
		if (arg_tot > 64) {
			kfree(progname);
			kfree(buffer);
			kfree(offset);
			return E2BIG;
		}
		result = copyinstr(arg_start, buffer_end, buffer_rem, &arg_len);
		if (result) {
			kfree(progname);
			kfree(buffer);
			kfree(offset);
			return result;
		}
		offset[arg_tot] = buffer_max - buffer_rem;
		buffer_end = buffer_end + arg_len;
		buffer_rem = buffer_rem - arg_len;
		args += sizeof(char*);
	}

	result = vfs_open(newprog, O_RDONLY, 0, &v);
	if (result) {
		kfree(newprog);
		kfree(buffer);
		kfree(offset);
		return result;
	}

	//Create a new address space. 
	as = as_create();
	if (as == NULL) {
		vfs_close(v);
		kfree(buffer);
		kfree(newprog);
		kfree(offset);
		return ENOMEM;
	}

	// Switch to it and activate it. 
	proc_setas(as);
	as_activate();

	result = load_elf(v, &entrypoint);
	if (result) {
		vfs_close(v);
		kfree(buffer);
		kfree(newprog);
		kfree(offset);
		return result;
	}

	// Done with the file now. 
	vfs_close(v);

	result = as_define_stack(as, &stackptr);
	if (result) {
	//	 p_addrspace will go away when curproc is destroyed 
		kfree(buffer);
		kfree(newprog);
		kfree(offset);
		return result;
	}

	kfree(newprog);
	
	size_t stack_size = buffer_end - buffer;
	stackptr -= stack_size;
	stackptr -= (stackptr & (sizeof(void *) - 1));
	stack_start = (userptr_t)stackptr;
	result = copyout(buffer, stack_start, stack_size);

	if (result) {
		kfree(buffer);
		kfree(offset);
	}
	stackptr -= (arg_tot+1) * sizeof(char*);
	addr_start = (userptr_t) stackptr;

	for (int i=0; i<arg_tot; i++) {
		address = stack_start + offset[i];
		result = copyout(&address, addr_start, sizeof(char*));
		if (result) {
			kfree(buffer);
			kfree(offset);
		}
		addr_start = addr_start + sizeof(char*);
	}

	address = NULL;
	copyout(&address, addr_start, sizeof(char*));
	argc = arg_tot;

	kfree(buffer);
	kfree(offset);
	/*enter user mode.... No comingback from here */
	enter_new_process(argc, (userptr_t)stackptr, NULL,stackptr, entrypoint);
	// No program should ever reach here
	return EINVAL;
}
