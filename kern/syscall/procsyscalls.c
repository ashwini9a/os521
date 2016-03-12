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

int sys_execv(const char *program, char **args)
{
	//just a comment
//	(void)args;
	char *newprog = kmalloc(PATH_MAX+1);
	char **kargs; 
	//char *buff[];
	size_t len, total_len=0;
	int i=0, cnt, result;
	size_t MAX_ARG_SIZE=4000, actlen;
	size_t stcksize;
	//char *buff[MAX_PTRS];
//	kargs = kmalloc(MAX_PTRS*sizeof(char *));
	kargs = kmalloc(sizeof(char **));
	if (kargs == NULL) {
		kfree(newprog);
		return ENOMEM;
	}
	// Copy userdata to kernel buffer
//	copyin(args,*kargs,MAX_PTRS*sizeof(char *));
	result = copyin ((userptr_t)args, kargs, sizeof(char **));
	if (result) {
		return EINVAL;
	}
	while(args[i] != NULL)
	{
		kargs[i]=kmalloc(sizeof(char*) * MAX_ARG_SIZE);
		result = copyinstr((const_userptr_t)args[i], kargs[i],MAX_ARG_SIZE,&actlen);
		if (result) {
			kfree(kargs);
			kfree(newprog);
			return EFAULT;
		}
		//buff[i]=tempstr;
		total_len+=actlen;				
		i++;
	}
	kargs[i] = NULL;
	cnt=0;
	
	total_len=total_len+i;		//including /0
	stcksize = total_len + (sizeof(char *) * (i+1));
	(void) stcksize;
	if(total_len>ARG_MAX)
	{
		kfree(kargs);
		kfree(newprog);
		return E2BIG;
	}
	
	if (newprog == NULL) {
        //       kprintf("kmalloc failed to  assign space for newprog in file.c\n");
        	kfree(kargs);
                return ENOSPC;
        }
//	if (strcmp(newprog, "") ==0) {
//		return ENOEXEC;
//	}
        result =  copyinstr((const_userptr_t)program, newprog, PATH_MAX, &len);
	if (result) {
		kfree(newprog);
		kfree(kargs);
		return EFAULT;
	}
	struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;

	result = vfs_open(newprog, O_RDONLY, 0, &v);
	if (result) {
		kfree(newprog);
		kfree(kargs);
		return result;
	}

	 //Create a new address space. 
	as = as_create();
	if (as == NULL) {
		vfs_close(v);
		kfree(kargs);
		kfree(newprog);
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
	//stackptr = stackptr - stcksize;
	size_t tlen = 0;
	size_t len2;
	//tempstk=stackptr + sizeof(char *)*i;
	while(cnt<i)
	{
//		* stackptr = tempstk;
		//copyout((const void*) tempstk, (userptr_t)stackptr, sizeof(stcksize));
		//tlen=0;
		len2 = strlen(kargs[cnt]) + 1;
		tlen =len2;
		if((len2%4) != 0){
			len2 = len2 + (4 - (len2 % 4));
		}
		//char *temstr = kmalloc(sizeof(len2));
		char temstr[len2];
		strcpy(temstr,kargs[cnt]);
		//temstr = kstrdup(kargs[cnt]);
		for(size_t ind=0; ind<=len2; ind++)
		{
			if (ind>=tlen) {
			//		strcat(temstr,'\0');
				temstr[ind] = '\0';
			}
			//else {
			//	temstr[ind] = kargs[cnt][ind];
			//}
		}
		//* tempstk= temstr;
		stackptr-= len2;
		result = copyout((const void*)temstr, (userptr_t)stackptr, sizeof(len2));
		if (result) {
			kfree(kargs);
			kfree(newprog);
			return EFAULT;
		}
		//tempstk+=tlen;
		//stackptr = stackptr+sizeof(char *);
		//kfree(temstr);
		kargs[cnt] = (char *)stackptr;
		cnt++;	
	}
	//stackptr = stackptr - sizeof(char *)*i;
	if (kargs[cnt] == NULL) {
		stackptr -= sizeof(char*);
	}

	for (int i=(cnt-1); i>=0; i--) {
		stackptr = stackptr - sizeof(char*);
		result = copyout((const void *)(kargs +i), (userptr_t)stackptr, sizeof(char *));
		if (result) {
			return EFAULT;
		}
	}
//	as_destroy(curproc->p_addrspace);
	// Warp to user mode. 
	enter_new_process(cnt /*argc*/, (userptr_t)stackptr /*userspace addr of argv*/,
			  NULL /*userspace addr of environment*/,
			  stackptr, entrypoint);

	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
	return EINVAL;
}
