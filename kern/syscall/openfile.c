#include<openfile.h>
#include<kern/fcntl.h>
#include<types.h>
#include<copyinout.h>
#include<syscall.h>

int openfile (userptr_t usr_filename_ptr, userptr_t usr_mode_ptr,
		 0, userptr_t usr_vnode_ptr) {
	
	int result;
	char *filename;
	char *readmode;
	struct vnode *v;

	result = copyin(usr_filename_ptr, &filename, sizeof(usr_filename_ptr));
	if (result) {
		return result;
	}
	result = vfs_open(filename, O_RDONLY, 0, &v);
	return result;
	
}
