#include <file.h>

int sys_open(const_userptr_t file, int flags, mode_t mode, int *returnvalue) {
	int index;
	char *filename = kmalloc(PATH_MAX+1);
	struct vnode *vnode;
	off_t offset = 0;
	size_t length;

	if (filename == NULL) {
		kprintf("kmalloc failed to  assign space for filename in file.c\n");
		return ENOSPC;
	}
	copyinstr(file, filename, PATH_MAX, &length);
	filename = (char*)file;
	kprintf("filename according to kernel is %s:\n",filename);
	index = 3;
	while(curproc->filedescriptor[index]!=NULL) {
		index++;
	}

	if (index == OPEN_MAX) {
		kprintf("Can't open more files. Close some");
		filename = NULL;
		kfree(filename);
		return EMFILE;
	}

	curproc->filedescriptor[index] =(struct filehandle*)kmalloc(sizeof(struct filehandle));
	if (curproc->filedescriptor[index] == NULL) {
		kprintf("could not allocate memory to filehandle");
		filename = NULL;
		kfree(filename);
		return ENFILE;
	}
	int result = vfs_open(filename, flags, mode, &vnode);
	if (result) {
		kprintf("file open failed");
		filename = NULL;
		kfree(filename);
		kfree(curproc->filedescriptor[index]);
		return result;
	}
	curproc->filedescriptor[index]->filename = filename;
	curproc->filedescriptor[index]->flags = flags;
 	curproc->filedescriptor[index]->offset = offset;
	curproc->filedescriptor[index]->refcount = 1;
	curproc->filedescriptor[index]->filelock = lock_create(filename);
	curproc->filedescriptor[index]->vnode = vnode;

//	filedescriptor[index] = filehandle;
	filename = NULL;
	kfree(filename);
	*returnvalue = index;
	return 0;
}

int filedescriptor_init(void) {
	
	char *filename;
	filename = kstrdup("con:");
	char *out = kstrdup("con:");
	char *err = kstrdup("con:");
	struct vnode *vn1;
	struct vnode *vn2;
	struct vnode *vn3;

	if (curproc->filedescriptor[0] == NULL) {
		curproc->filedescriptor[0] =(struct filehandle*) kmalloc(sizeof(struct filehandle));
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
	}

	if (curproc->filedescriptor[1] == NULL) {
		curproc->filedescriptor[1] = (struct filehandle*) kmalloc(sizeof(struct filehandle));
		if (curproc->filedescriptor[1] == NULL) {
			kprintf("Could not allocate memory for std out");
			return ENFILE;
		}
		int result = vfs_open(out, O_WRONLY, 0664, &vn2);
		if (result) {
			kprintf("could not open std output");
			return EINVAL;
		}

		curproc->filedescriptor[1]->filename = out;
		curproc->filedescriptor[1]->flags = O_WRONLY;
		curproc->filedescriptor[1]->offset = 0;
		curproc->filedescriptor[1]->refcount = 1;
		curproc->filedescriptor[1]->filelock = lock_create("stdout_lock");
		curproc->filedescriptor[1]->vnode = vn2;
	}

	if (curproc->filedescriptor[2] == NULL) {
		curproc->filedescriptor[2] = (struct filehandle*) kmalloc(sizeof(struct filehandle));
		if (curproc->filedescriptor[2] == NULL) {
			kprintf("Could not allocate memory for std err");
			return ENFILE;
		}
		int result = vfs_open(err, O_WRONLY, 0664, &vn3);
		if (result) {
			kprintf("could not open std err");
			return EINVAL;
		}

		curproc->filedescriptor[2]->filename = err;
		curproc->filedescriptor[2]->flags = O_WRONLY;
		curproc->filedescriptor[2]->offset = 0;
		curproc->filedescriptor[2]->refcount = 1;
		curproc->filedescriptor[2]->filelock = lock_create("stderr_lock");
		curproc->filedescriptor[2]->vnode = vn2;
	}
	return 0;
}


int sys_lseek(int fd, off_t pos, int whence, int *returnvalue) {
	off_t present_offset = 0;
	off_t new_pos = 0;
	struct stat temp;
	int result;

	if (fd == 0 || fd == 1 || fd == 2) {
		kprintf("The user is mad.... Trying to seek in con :(\n");
		return ESPIPE;
	}
	if (fd > OPEN_MAX || fd < 0) {
		kprintf("Stupid malicious user...\n");
		return EBADF;
	}
	if (curproc->filedescriptor[fd] == NULL) {
		kprintf("Stupid malicious user...\n");
		return EBADF;
	}

	if (VOP_ISSEEKABLE(curproc->filedescriptor[fd]->vnode)) {
		present_offset = curproc->filedescriptor[fd]->offset;	
		switch(whence) {
	
			case SEEK_SET :
				new_pos = pos;
				break;
		
			case SEEK_CUR :	
				new_pos = present_offset + pos;
				break;

			case SEEK_END :
				result = VOP_STAT(curproc->filedescriptor[fd]->vnode, &temp);
				if (result) {
					kprintf("the kernel screwed up... couldn't vop_stat");
					return result;
				}
				present_offset = temp.st_size;
				new_pos = present_offset + pos;
				break;

			default :
				kprintf("whence is invalid");
				return EINVAL;
		}
	
		curproc->filedescriptor[fd]->offset = new_pos;
		*returnvalue =(int) new_pos;
		return 0;
	}
	return ESPIPE;
}	


bool isFdReadValid(int fd)
{
	if(curproc->filedescriptor[fd]==NULL)
		return false;
	if(curproc->filedescriptor[fd]->flags != O_RDONLY ||  curproc->filedescriptor[fd]->flags != O_RDWR)
		return false;
	return true;
}

bool isFdWriteValid(int fd)
{
        if(curproc->filedescriptor[fd]==NULL)
                return false;
        if(curproc->filedescriptor[fd]->flags != O_WRONLY ||  curproc->filedescriptor[fd]->flags != O_RDWR)
                return false;
        return true;

}

filehandle* getfileHandle(int fd)
{
	return curproc->filedescriptor[fd];
}

ssize_t sys_read(int fd, const void *buf, size_t buflen)
{
        if(!(isFdReadValid(fd)))
                return EBADF;
        if(buf==NULL)
                return EFAULT;
        struct filehandle *fh;
        struct uio *uiotemp;
	int result;
        uiotemp = (struct uio*)kmalloc(sizeof(struct uio*));

        fh=(struct filehandle*)kmalloc(sizeof(struct filehandle*));
        fh = getfileHandle(int fd);
	uiotemp->uio_iov = ;
	uiotemp->uio_iovcnt = ;
	uiotemp->uio_offset = ;
	uiotemp->uio_resid = ;
	uiotemp->uio_segflg = UIO_USERSPACE;
        uiotemp->uio_rw = UIO_READ ;
	uiotemp->uio_space = ;

        result = VOP_READ(fh->vnode,uiotemp);
	if(result)
		return EIO;
        return;
}


ssize_t sys_write(int fd, const void *buf, size_t nbytes)
{
        if(!(isFdWriteValid(fd)))
                return EBADF;
        if(buf==NULL)
                return EFAULT;
        struct filehandle *fh;
        struct uio *uiotemp;
	int result;
        uiotemp = (struct uio*)kmalloc(sizeof(struct uio*));

        fh=(struct filehandle*)kmalloc(sizeof(struct filehandle*));
        fh = getfileHandle(int fd);
        uiotemp->uio_iov = ;
        uiotemp->uio_iovcnt = ;
        uiotemp->uio_offset = ;
        uiotemp->uio_resid = ;
        uiotemp->uio_segflg = UIO_USERSPACE;
        uiotemp->uio_rw = UIO_READ ;
        uiotemp->uio_space = ;
        result = VOP_WRITE(fh->vnode,uiotemp);
	if(result)
		return EIO;
        return;
}
