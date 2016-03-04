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
