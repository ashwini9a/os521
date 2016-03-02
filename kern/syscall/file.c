#include <file.h>

int sys_open(const_userptr_t file, int flags, mode_t mode, int *returnvalue) {
	int index;
	char *filename = (char*)kmalloc(PATH_MAX + 1);
	struct vnode *vnode;
	int offset =0;

	if (filename == NULL) {
		kprintf("Kmalloc failed to  give a filename");
		return ENOSPC;
	}
	copyin(file, &filename, sizeof(file));

	index = 3;
	while(curproc->filedescriptor[index]!=NULL) {
		index++;
	}

	if (index == OPEN_MAX) {
		kprintf("Can't open more files. Close some");
		return EMFILE;
	}

	curproc->filedescriptor[index] =(struct filehandle*)kmalloc(sizeof(struct filehandle));
	if (curproc->filedescriptor[index] == NULL) {
		kprintf("could not allocate memory to filehandle");
		return ENFILE;
	}
	int result = vfs_open(filename, flags, mode, &vnode);
	if (result) {
		kprintf("file open failed");
		return result;
	}
	curproc->filedescriptor[index]->filename = filename;
	curproc->filedescriptor[index]->flags = flags;
 	curproc->filedescriptor[index]->offset = offset;
	curproc->filedescriptor[index]->refcount = 1;
	curproc->filedescriptor[index]->filelock = lock_create(filename);
	curproc->filedescriptor[index]->vnode = vnode;

//	filedescriptor[index] = filehandle;
	kfree(filename);
	*returnvalue = index;
	return 0;
}

int filedescriptor_init(void) {
	
	char *filename;
	filename = kstrdup("con:");
	struct vnode *vn1;
	struct vnode *vn2;
	struct vnode *vn3;

	curproc->filedescriptor[0] =(struct filehandle*) kmalloc(sizeof(struct filehandle*));
	if (curproc->filedescriptor[0] == NULL) {
		kprintf("could not allocate memory for std in");
		return ENFILE;
	}
	
	int result = vfs_open(filename, O_RDONLY, 0664, &vn1);
	if (result) {
		kprintf("Console 0 open failed");
		return EINVAL;
	}

	curproc->filedescriptor[0]->filename = filename;
	curproc->filedescriptor[0]->flags = O_RDONLY;
	curproc->filedescriptor[0]->offset = 0;
	curproc->filedescriptor[0]->refcount = 1;
	curproc->filedescriptor[0]->filelock = lock_create("stdin_lock");
	curproc->filedescriptor[0]->vnode = vn1; 

	curproc->filedescriptor[1] = (struct filehandle*) kmalloc(sizeof(struct filehandle*));
	if (curproc->filedescriptor[1] == NULL) {
		kprintf("Could not allocate memory for std out");
		return ENFILE;
	}
	result = vfs_open(filename, O_WRONLY, 0664, &vn2);
	if (result) {
		kprintf("could not open std output");
		return EINVAL;
	}

	curproc->filedescriptor[1]->filename = filename;
	curproc->filedescriptor[1]->flags = O_WRONLY;
	curproc->filedescriptor[1]->offset = 0;
	curproc->filedescriptor[1]->refcount = 1;
	curproc->filedescriptor[1]->filelock = lock_create("stdout_lock");
	curproc->filedescriptor[1]->vnode = vn2;


	curproc->filedescriptor[2] = (struct filehandle*) kmalloc(sizeof(struct filehandle*));
	if (curproc->filedescriptor[2] == NULL) {
		kprintf("Could not allocate memory for std err");
		return ENFILE;
	}
	result = vfs_open(filename, O_WRONLY, 0664, &vn3);
	if (result) {
		kprintf("could not open std err");
		return EINVAL;
	}

	curproc->filedescriptor[2]->filename = filename;
	curproc->filedescriptor[2]->flags = O_WRONLY;
	curproc->filedescriptor[2]->offset = 0;
	curproc->filedescriptor[2]->refcount = 1;
	curproc->filedescriptor[2]->filelock = lock_create("stderr_lock");
	curproc->filedescriptor[2]->vnode = vn2;

	return 0;
}	
